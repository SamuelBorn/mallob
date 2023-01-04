#include "external_clause_checker.hpp"


std::list<Clause> ExternalClauseChecker::getAdmittedExternalClauses() {
    auto lock = _admitted_clauses_mutex.getLock();
    auto ret = std::move(_admitted_clauses);
    _admitted_clauses.clear();
    return ret;
}

void ExternalClauseChecker::insertExternalClausesToCheck(int *clauses, size_t size) {
    // wie splitte ich Klauseln in einzelne Klauseln?

    auto lock = _clauses_to_check_mutex.getLock();

}

ExternalClauseChecker::ExternalClauseChecker() {}