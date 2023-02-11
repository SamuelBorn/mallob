#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <utility>
#include <thread>
#include <atomic>
#include <list>

#include "app/sat/solvers/cadical.hpp"
#include "app/sat/execution/solver_thread.hpp"
#include "app/sat/data/owned_clause.hpp"
#include "app/sat/sharing/filter/clause_filter.hpp"

#include "util/ordered_buffers/fifo_ring_buffer.hpp"
#include "util/ordered_buffers/lifo_ring_buffer.hpp"
#include "util/ordered_buffers/min_ring_buffer.hpp"
#include "util/ordered_buffers/max_ring_buffer.hpp"

class SatEngine;

class ExternalClauseChecker {

private:
    //std::list<OwnedClause> _clauses_to_check;
    const int _max_clause_count = 2000;
    LIFORingBuffer<OwnedClause> _clauses_to_check_implementation = LIFORingBuffer<OwnedClause>(_max_clause_count);
    SynchronizedOrderedBuffer<OwnedClause> &_clauses_to_check = _clauses_to_check_implementation;
    std::set<OwnedClause> _admitted_clauses;

    std::atomic_int buffer_full = 0;
    std::atomic_int admitted = 0;
    std::atomic_int rejected = 0;
    std::atomic_int timeouted = 0;

    //Mutex _clauses_to_check_mutex;
    Mutex _admitted_clauses_mutex;

    //std::atomic_int _num_clauses_to_check = 0;

    ClauseFilter _clause_bloom_filter;

    const Parameters& _params;
    std::unique_ptr<Cadical> _solver_ptr;
    Cadical& _solver;
    Logger& _logger;
    std::thread _thread;

    std::vector<SerializedFormulaParser> _pending_formulae;
    std::vector<std::pair<size_t, const int*>> _pending_assumptions;

    int _local_id;
    std::string _name;
    int _portfolio_rank;
    int _portfolio_size;
    int _local_solvers_count;
    long _tid = -1;

    Mutex _state_mutex;
    ConditionVariable _state_cond;

    std::atomic_int _latest_revision = 0;
    std::atomic_int _active_revision = 0;
    unsigned long _imported_lits_curr_revision = 0;
    bool _last_read_lit_zero = true;
    int _max_var = 0;
    VariableTranslator _vt;
    bool _has_pseudoincremental_solvers;

    std::atomic_bool _initialized = false;
    std::atomic_bool _interrupted = false;
    std::atomic_bool _suspended = false;
    std::atomic_bool _terminated = false;

    bool _found_result = false;
    JobResult _result;

public:
    ExternalClauseChecker(const Parameters& params, const SatProcessConfig& config,  const SolverSetup &solverSetup,
                 size_t fSize, const int* fLits, size_t aSize, const int* aLits, int localId);
    ~ExternalClauseChecker();

    void start();
    void appendRevision(int revision, size_t fSize, const int* fLits, size_t aSize, const int* aLits);
    void setSuspend(bool suspend) {
        {
            auto lock = _state_mutex.getLock();
            _suspended = suspend;
            if (_suspended) _solver.suspend();
            else _solver.resume();
        }
        _state_cond.notify();
    }
    void setTerminate() {
        _solver.setTerminate();
        _terminated = true;
        _state_cond.notify();
    }
    void tryJoin() {if (_thread.joinable()) _thread.join();}

    bool isInitialized() const {
        return _initialized;
    }
    int getTid() const {
        return _tid;
    }
    bool hasFoundResult(int revision) {
        auto lock = _state_mutex.getLock();
        return _initialized && _active_revision == revision && _found_result;
    }
    JobResult& getSatResult() {
        return _result;
    }
    int getActiveRevision() const {return _active_revision;}

    void submitClausesForTesting(int *externalClausesBuffer, int externalClausesBufferSize);
    std::vector<int> fetchAdmittedClauses();
    bool hasAdmittedClauses();

private:
    void init();
    void* run();

    void pin();
    bool readFormula();

    void diversifyInitially();
    void diversifyAfterReading();

    void runOnce();

    void waitWhileSolved();
    void waitWhileSuspended();
    void waitUntil(std::function<bool()> predicate);

    void reportResult(int res, int revision);

    Cadical* createLocalSolverInterface(const SolverSetup &solverSetup);

    const char* toStr();
};