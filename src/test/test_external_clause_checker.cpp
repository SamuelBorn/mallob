
#include <iostream>
#include "util/assert.hpp"
#include <vector>
#include <string>
#include <map>
#include <chrono>

#include "util/random.hpp"
#include "util/logger.hpp"
#include "util/sys/timer.hpp"
#include "comm/mympi.hpp"
#include "util/params.hpp"

#include "app/sat/solvers/cadical.hpp"
#include "app/sat/execution/solver_thread.hpp"
#include "app/sat/data/owned_clause.h"

#define private private  // ATTENTION THIS IS EVIL AND WILL BREAK ALL ENCAPSULATION

#include "cross_problem_clause_sharing/external_clause_checker.hpp"

#undef private

Parameters params;

void fillTestCdb(AdaptiveClauseDatabase::Setup &setup, AdaptiveClauseDatabase &cdb, std::list<OwnedClause> &testClauses) {
    const int nbMaxClausesPerSlot = 10;
    int nbClausesThisSlot = 0;
    BufferIterator it(setup.maxClauseLength, setup.slotsForSumOfLengthAndLbd);

    bool success = true;
    while (success) {
        std::vector<int> lits(it.clauseLength);
        for (size_t i = 0; i < lits.size(); i++) {
            lits[i] = 100 * nbClausesThisSlot + i + 1;
        }
        Clause c{lits.data(), (int) lits.size(), it.lbd};
        success = cdb.addClause(c);
        if (success) testClauses.emplace_back(c.copy());
        nbClausesThisSlot++;
        if (nbClausesThisSlot == nbMaxClausesPerSlot) {
            it.nextLengthLbdGroup();
            nbClausesThisSlot = 0;
        }
    }
}

bool areClausesEqual(int *a, int a_len, int *b, int b_len) {
    if (a_len != b_len) return false;
    for (int i = 0; i < a_len; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

void testClauseDestruction() {
    AdaptiveClauseDatabase::Setup setup;
    AdaptiveClauseDatabase cdb(setup);
    std::list<OwnedClause> testClauses;
    fillTestCdb(setup, cdb, testClauses);
    int num_exported_clauses;
    auto buf = cdb.exportBuffer(100000, num_exported_clauses);

    std::list<OwnedClause> clauses_to_check = {};
    std::atomic_int _num_clauses_to_check = 0;

    auto reader = BufferReader(buf.data(), buf.size(), setup.maxClauseLength, setup.slotsForSumOfLengthAndLbd, false);
    Clause c = reader.getNextIncomingClause();
    while (c.begin != nullptr) {
        clauses_to_check.emplace_back(c.copy());
        _num_clauses_to_check++;
        c = reader.getNextIncomingClause();
    }

    assert(_num_clauses_to_check == num_exported_clauses);
    assert(clauses_to_check.size() == num_exported_clauses);
    assert(testClauses.size() == num_exported_clauses);
    auto a = testClauses.begin();
    auto b = clauses_to_check.begin();
    for (; a != testClauses.end() && b != clauses_to_check.end(); a++, b++) {
        assert(areClausesEqual(a->stored_clause.begin, a->stored_clause.size, b->stored_clause.begin, b->stored_clause.size));
    }

    std::cout << "✔ Clause Destruction" << std::endl;
}

void testClauseCreation() {
    AdaptiveClauseDatabase::Setup setup;
    AdaptiveClauseDatabase cdb(setup);
    std::list<OwnedClause> testClauses;
    fillTestCdb(setup, cdb, testClauses);
    int num_exported_clauses;
    auto buf = cdb.exportBuffer(100000, num_exported_clauses);

    std::vector<int> new_buffer;
    auto builder = BufferBuilder(-1, params.strictClauseLengthLimit(), params.groupClausesByLengthLbdSum(), &new_buffer);
    for (const auto &clause: testClauses) {
        builder.append(clause.stored_clause);
    }

    assert(buf == new_buffer);
    std::cout << "✔ Clause Creation" << std::endl;
}

void testECCCreation() {
    SatProcessConfig satProcessConfig;
    SolverSetup solverSetup;
    ExternalClauseChecker ecc(params, satProcessConfig, solverSetup, 0, nullptr, 0, nullptr, 0);
}

int main(int argc, char *argv[]) {
    params.init(argc, argv);

    testClauseDestruction();
    testClauseCreation();
    testECCCreation();
}
