#include "app/sat/execution/solver_thread.hpp"

class ExternalClauseChecker {

private:
    std::list<Clause> _clauses_to_check;
    std::list<Clause> _admitted_clauses;

    Mutex _clauses_to_check_mutex;
    Mutex _admitted_clauses_mutex;

public:
    ExternalClauseChecker();

public:
    std::list<Clause> getAdmittedExternalClauses();

    void insertExternalClausesToCheck(int* clauses, size_t size);
};
