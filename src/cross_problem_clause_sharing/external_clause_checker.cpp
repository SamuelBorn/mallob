
#include <sys/resource.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <list>

#include "util/assert.hpp"

#include "app/sat/execution/solver_thread.hpp"
#include "app/sat/execution/engine.hpp"
#include "util/sys/proc.hpp"
#include "util/hashing.hpp"
#include "external_clause_checker.hpp"
#include "app/sat/solvers/cadical_timeout_terminator.hpp"
#include "jersolow_wang.hpp"


template<typename T>
std::string array_to_str(const T *arr, size_t size) {
    std::string ret;
    for (int i = 0; i < size; ++i) {
        ret += std::to_string(arr[i]) + " ";
    }
    return ret;
}

template<typename T>
std::string vec_to_str(std::vector<T> &vec) {
    return array_to_str(vec.data(), vec.size());
}

using namespace SolvingStates;

// A GLORIFIED COPY OF SOLVER THREAD

ExternalClauseChecker::ExternalClauseChecker(const Parameters &params, const SatProcessConfig &config, const SolverSetup &solverSetup,
                                             size_t fSize, const int *fLits, size_t aSize, const int *aLits, int localId) :
        _params(params), _solver_ptr(createLocalSolverInterface(solverSetup)), _solver(*_solver_ptr),
        _logger(_solver.getLogger()), _local_id(localId), _clause_bloom_filter(params.strictClauseLengthLimit()),
        _has_pseudoincremental_solvers(_solver.getSolverSetup().hasPseudoincrementalSolvers),
        literal_ranks(get_lit_rankings(fSize, fLits)) {

    _portfolio_rank = config.apprank;
    _portfolio_size = config.mpisize;
    _local_solvers_count = config.threads;

    appendRevision(0, fSize, fLits, aSize, aLits);
    _result.result = UNKNOWN;
}

void ExternalClauseChecker::start() {
    if (!_thread.joinable())
        _thread = std::thread([this]() {
            init();
            run();
        });
}

void ExternalClauseChecker::init() {
    _tid = Proc::getTid();
    LOGGER(_logger, V5_DEBG, "tid %ld\n", _tid);
    std::string threadName = "SATSolver#" + std::to_string(_local_id);
    Proc::nameThisThread(threadName.c_str());

    _active_revision = 0;
    _imported_lits_curr_revision = 0;

    _initialized = true;
}

void *ExternalClauseChecker::run() {

    diversifyInitially();

    while (!_terminated) {

        // Sleep and wait if the solver should not do solving right now
        waitWhileSolved();
        waitWhileSuspended();
        if (_terminated) break;

        bool readingDone = readFormula();

        // Skip solving attempt if reading was incomplete
        if (!readingDone) continue;

        diversifyAfterReading();

        runOnce();
    }

    LOGGER(_logger, V4_VVER, "exiting\n");
    return NULL;
}

bool ExternalClauseChecker::readFormula() {
    constexpr int batchSize = 100000;

    SerializedFormulaParser *fParser;
    size_t aSize = 0;
    const int *aLits;

    while (true) {

        // Shuffle next input
        if (_imported_lits_curr_revision == 0) {
            // ... not for the first solver
            bool shuffle = _solver.getDiversificationIndex() >= 1;
            float random = 0.0001f * (rand() % 10000); // random number in [0,1)
            assert(random >= 0);
            assert(random <= 1);
            // ... only if random throw hits user-defined probability
            shuffle = shuffle && random < _params.inputShuffleProbability();

            if (shuffle) {
                LOGGER(_logger, V4_VVER, "Shuffling input rev. %i\n", (int) _active_revision);
                {
                    auto lock = _state_mutex.getLock();
                    assert(_active_revision < (int) _pending_formulae.size());
                    fParser = &_pending_formulae[_active_revision];
                }
                fParser->shuffle(_solver.getGlobalId());
            }
        }

        // Fetch the next formula to read
        {
            auto lock = _state_mutex.getLock();
            assert(_active_revision < (int) _pending_formulae.size());
            fParser = &_pending_formulae[_active_revision];
            aSize = _pending_assumptions[_active_revision].first;
            aLits = _pending_assumptions[_active_revision].second;
        }

        LOGGER(_logger, V4_VVER, "Reading rev. %i, start %i\n", (int) _active_revision, (int) _imported_lits_curr_revision);

        // Repeatedly read a batch of literals, checking in between whether to stop/terminate
        while (_imported_lits_curr_revision < fParser->getPayloadSize()) {

            // Read next batch
            auto numImportedBefore = _imported_lits_curr_revision;
            auto end = std::min(numImportedBefore + batchSize, fParser->getPayloadSize());
            int lit;
            while (_imported_lits_curr_revision < end && fParser->getNextLiteral(lit)) {

                if (std::abs(lit) > 134217723) {
                    LOGGER(_logger, V0_CRIT, "[ERROR] Invalid literal %i at rev. %i pos. %ld/%ld.\n",
                           lit, (int) _active_revision, _imported_lits_curr_revision, fParser->getPayloadSize());
                    abort();
                }
                if (lit == 0 && _last_read_lit_zero) {
                    LOGGER(_logger, V0_CRIT, "[ERROR] Empty clause at rev. %i pos. %ld/%ld.\n",
                           (int) _active_revision, _imported_lits_curr_revision, fParser->getPayloadSize());
                    abort();
                }
                _solver.addLiteral(_vt.getTldLit(lit));
                _max_var = std::max(_max_var, std::abs(lit));
                _last_read_lit_zero = lit == 0;
                //_dbg_lits += std::to_string(lit]) + " ";
                //if (lit == 0) _dbg_lits += "\n";

                ++_imported_lits_curr_revision;
            }

            // Suspend and/or terminate if needed
            waitWhileSuspended();
            if (_terminated) return false;
        }
        // Adjust _max_var according to assumptions as well
        for (size_t i = 0; i < aSize; i++) _max_var = std::max(_max_var, std::abs(aLits[i]));

        {
            auto lock = _state_mutex.getLock();
            assert(_imported_lits_curr_revision == fParser->getPayloadSize());

            // If necessary, introduce extra variable to the problem
            // to encode equivalence to the set of assumptions
            if (_has_pseudoincremental_solvers && _active_revision >= _vt.getExtraVariables().size()) {
                _vt.addExtraVariable(_max_var);
                int aEquivVar = _vt.getExtraVariables().back();
                LOGGER(_logger, V4_VVER, "Encoding equivalence for %i assumptions of rev. %i/%i @ var. %i\n",
                       aSize, (int) _active_revision, (int) _latest_revision, (int) aEquivVar);

                // Make clause exporter append this condition
                // to each clause this solver exports
                if (_solver.exportsConditionalClauses() || !_solver.getSolverSetup().doIncrementalSolving)
                    _solver.setCurrentCondVarOrZero(aSize > 0 ? aEquivVar : 0);

                if (aSize > 0) {
                    // If all assumptions hold, then aEquivVar holds
                    for (size_t i = 0; i < aSize; i++) {
                        _solver.addLiteral(-_vt.getTldLit(aLits[i]));
                    }
                    _solver.addLiteral(aEquivVar);
                    _solver.addLiteral(0);

                    // If aEquivVar holds, then each assumption holds
                    for (size_t i = 0; i < aSize; i++) {
                        _solver.addLiteral(-aEquivVar);
                        _solver.addLiteral(_vt.getTldLit(aLits[i]));
                        _solver.addLiteral(0);
                    }
                }
            }

            _solver.setCurrentRevision(_active_revision);

            // No formula left to read?
            if (_active_revision == _latest_revision) {
                LOGGER(_logger, V4_VVER, "Reading done @ rev. %i\n", (int) _active_revision);
                return true;
            }
            _active_revision++;
            _imported_lits_curr_revision = 0;
        }
    }
}

void ExternalClauseChecker::appendRevision(int revision, size_t fSize, const int *fLits, size_t aSize, const int *aLits) {
    {
        auto lock = _state_mutex.getLock();
        _pending_formulae.emplace_back(_logger, fSize, fLits);
        LOGGER(_logger, V4_VVER, "Received %i literals\n", fSize);
        _pending_assumptions.emplace_back(aSize, aLits);
        LOGGER(_logger, V4_VVER, "Received %i assumptions\n", aSize);
        _latest_revision = revision;
        _found_result = false;
        assert(_latest_revision + 1 == (int) _pending_formulae.size()
               || LOG_RETURN_FALSE("%i != %i", _latest_revision + 1, _pending_formulae.size()));
        if (revision > 0) {
            _solver.interrupt();
            _interrupted = true;
        }
    }
    _state_cond.notify();
}

void ExternalClauseChecker::diversifyInitially() {

    // Random seed
    size_t seed = 42;
    hash_combine(seed, (unsigned int) _tid);
    hash_combine(seed, (unsigned int) _portfolio_size);
    hash_combine(seed, (unsigned int) _portfolio_rank);
    srand(seed);
    _solver.diversify(seed);
}

void ExternalClauseChecker::diversifyAfterReading() {
    if (_solver.getGlobalId() >= _solver.getNumOriginalDiversifications()) {
        int solversCount = _local_solvers_count;
        int totalSolvers = solversCount * _portfolio_size;
        int vars = _solver.getVariablesCount();

        for (int var = 1; var <= vars; var++) {
            if (rand() % totalSolvers == 0) {
                _solver.setPhase(var, rand() % 2 == 1);
            }
        }
    }
}

void ExternalClauseChecker::runOnce() {
    // Wait until main() sends data
    std::unique_lock lk(m);
    cv.wait(lk, [this] { return _ready_for_external_checking; });
    lk.unlock();
    cv.notify_one();

    //std::string s;
    while (!_clauses_to_check.empty()) {
        auto clause = OwnedClause(_clauses_to_check.extract());
        LOG(V4_VVER, "[CPCS] Befor: %s\n", array_to_str(clause.stored_clause.begin, clause.stored_clause.size).c_str());
        std::sort(clause.stored_clause.begin, clause.stored_clause.begin + clause.stored_clause.size,
                  [&](const int lhs, const int rhs) { return literal_ranks.at(std::abs(lhs)) > literal_ranks.at(std::abs(rhs)); });

        size_t total_num_assumptions = clause.stored_clause.size;
        LOG(V4_VVER, "[CPCS] Checking: %s\n", array_to_str(clause.stored_clause.begin, clause.stored_clause.size).c_str());
        for (int i = 0; i < total_num_assumptions; ++i)
            if (clause.stored_clause.begin[i] > _max_var) continue;

        auto assumptions = std::vector<int>();
        assumptions.reserve(total_num_assumptions);
        for (int i = 0; i < clause.stored_clause.size; ++i) {
            {
                auto lock = _state_mutex.getLock();
                if (_suspended) return;
                _solver.resume();
            }

            assumptions.push_back(-1 * clause.stored_clause.begin[i]);

            // If necessary, translate assumption literals
            std::vector<int> tldAssumptions;
            if (!_vt.getExtraVariables().empty()) {
                for (size_t j = 0; j < total_num_assumptions; j++) tldAssumptions.push_back(_vt.getTldLit(assumptions.at(j)));
                assumptions = tldAssumptions;
            }

            LOG(V4_VVER, "[CPCS] Subchecking %s\n", vec_to_str<int>(assumptions).c_str());
            auto t1 = Timer::elapsedSeconds();
            SatResult res = _solver.solve(assumptions.size(), assumptions.data());
            auto t2 = Timer::elapsedSeconds();

            auto elapsed_time_seconds = t2 - t1;
            assert(elapsed_time_seconds <= 2 * _solver_timeout_seconds);

            {  // Un interrupt solver (if it was interrupted)
                auto lock = _state_mutex.getLock();
                _solver.uninterrupt();
                _interrupted = false;
            }

            if (res == UNSAT) {
                auto lock = _admitted_clauses_mutex.getLock();
                _admitted_clauses.emplace(clause.stored_clause.copy());
                admitted++;
                if (total_num_assumptions > assumptions.size())
                    LOG(V4_VVER, "[CPCS] ------------ VALITADED NEW CLAUSE!! %s\n", vec_to_str<int>(assumptions).c_str());
                break;
            }
        }
    }

    {
        std::lock_guard lk2(m);
        _ready_for_external_checking = false;
    }
    LOG(V4_VVER, "[CPCS] ECC finished all Clauses → waiting for next\n");
}

void ExternalClauseChecker::waitWhileSolved() {
    waitUntil([&] { return _terminated || !_found_result; });
}

void ExternalClauseChecker::waitWhileSuspended() {
    waitUntil([&] { return _terminated || !_suspended; });
}

void ExternalClauseChecker::waitUntil(std::function<bool()> predicate) {
    if (predicate()) return;
    _state_cond.wait(_state_mutex, predicate);
}

void ExternalClauseChecker::reportResult(int res, int revision) {

    if (res == 0 || _found_result) return;
    const char *resultString = res == SAT ? "SAT" : "UNSAT";

    auto lock = _state_mutex.getLock();

    if (revision != _latest_revision) {
        LOGGER(_logger, V4_VVER, "discard obsolete result %s for rev. %i\n", resultString, revision);
        return;
    }

    LOGGER(_logger, V3_VERB, "found result %s for rev. %i\n", resultString, revision);
    _result.result = SatResult(res);
    _result.revision = revision;
    if (res == SAT) {
        auto solution = _solver.getSolution();
        _result.setSolutionToSerialize(solution.data(), solution.size());
    } else {
        auto failed = _solver.getFailedAssumptions();
        auto failedVec = std::vector<int>(failed.begin(), failed.end());
        _result.setSolutionToSerialize(failedVec.data(), failedVec.size());
    }

    // If necessary, convert solution back to original variable domain
    if (!_vt.getExtraVariables().empty()) {
        std::vector<int> origSolution;
        for (size_t i = 0; i < _result.getSolutionSize(); i++) {
            if (res == UNSAT) {
                // Failed assumption
                origSolution.push_back(_vt.getOrigLitOrZero(_result.getSolution(i)));
            } else if (i > 0) {
                // Assignment: v or -v at position v
                assert(_result.getSolution(i) == i || _result.getSolution(i) == -i);
                int origLit = _vt.getOrigLitOrZero(_result.getSolution(i));
                if (origLit != 0) origSolution.push_back(origLit);
                assert(origSolution[origSolution.size() - 1] == origSolution.size() - 1
                       || origSolution[origSolution.size() - 1] == 1 - origSolution.size());
            } else origSolution.push_back(0); // position zero
        }
        _result.setSolutionToSerialize(origSolution.data(), origSolution.size());
    }

    _found_result = true;
}

Cadical *ExternalClauseChecker::createLocalSolverInterface(const SolverSetup &solverSetup) {
    auto cadical = new Cadical(solverSetup);
    cadical->setAllowedConflicts(0);
    cadical->enableTimeout(_solver_timeout_seconds);
    return cadical;
}

ExternalClauseChecker::~ExternalClauseChecker() {
    if (_thread.joinable()) _thread.join();
}

const char *ExternalClauseChecker::toStr() {
    _name = "S" + std::to_string(_solver.getGlobalId());
    return _name.c_str();
}

void ExternalClauseChecker::submitClausesForTesting(int *externalClausesBuffer, int externalClausesBufferSize) {
    auto reader = BufferReader(externalClausesBuffer, externalClausesBufferSize, _params.strictClauseLengthLimit(), _params.groupClausesByLengthLbdSum(), false);
    Clause c = reader.getNextIncomingClause();
    while (c.begin != nullptr) {

        if (_clause_bloom_filter.registerClause(c.begin, c.size)) {
            if (!_clauses_to_check.insert(OwnedClause(c.copy()))) buffer_full++;
            //_num_clauses_to_check++;
        } else {
            LOG(V6_DEBGV, "[CPCS] Clause discarded -> already existed in bloom filter\n");
        }

        c = reader.getNextIncomingClause();
    }
    {
        std::lock_guard lk(m);
        _ready_for_external_checking = true;
    }
    cv.notify_one();

    LOG(V5_DEBG, "[CPCS] RECV clauses. To check: %i\n", (int) _clauses_to_check.size());
}

std::vector<int> ExternalClauseChecker::fetchAdmittedClauses() {
    auto lock = _admitted_clauses_mutex.getLock();
    LOG(V4_VVER, "[CPCS] full: %i,  admitted: %i,  rejected: %i,  timeout: %i, fill-level: %i\n", (int) buffer_full, (int) admitted, (int) rejected, (int) timeouted, _clauses_to_check.size());
    LOG(V4_VVER, "[CPCS] FETCH admitted clauses: %i\n", _admitted_clauses.size());

    std::vector<int> buffer;

    auto builder = BufferBuilder(-1, _params.strictClauseLengthLimit(), _params.groupClausesByLengthLbdSum(), &buffer);

    for (const auto &clause: _admitted_clauses) {
//        // sanity checks for a clause before adding it
//        assert(clause.stored_clause.size <= 255);
//        assert(clause.stored_clause.size >= last_clause_size);
//        for (int i = 0; i < clause.stored_clause.size; ++i) {
//            assert(std::abs(clause.stored_clause.begin[i]) < 10000);
//            assert(clause.stored_clause.begin[i] != 0);
//            assert(clause.stored_clause.lbd > 0);
//        }
        builder.append(clause.stored_clause);
    }
    _admitted_clauses.clear();

    return buffer;
}