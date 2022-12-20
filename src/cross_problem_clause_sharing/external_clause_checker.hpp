#include "app/sat/execution/solver_thread.hpp"
#include "app/sat/execution/solver_thread.hpp"

class ExternalClauseChecker : public SolverThread {

private:
    std::list<Clause> _clauses_to_check;
    std::list<Clause> _admitted_clauses;

    Mutex _clauses_to_check_mutex;
    Mutex _admitted_clauses_mutex;

public:
    ExternalClauseChecker(const Parameters &params, const SatProcessConfig &config, const std::shared_ptr<PortfolioSolverInterface> &solver, size_t fSize, const int *fLits, size_t aSize, const int *aLits, int localId);

public:
    std::list<Clause> getAdmittedExternalClauses();

    void insertExternalClausesToCheck(int* clauses, size_t size);
};
