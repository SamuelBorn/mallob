#pragma once

#include <limits>

#include "util/logger.hpp"
#include "util/sys/threading.hpp"
#include "util/sys/timer.hpp"

#include "app/sat/solvers/cadical_interface.hpp"
#include "app/sat/solvers/cadical_terminator.hpp"

struct CadicalTimeoutTerminator : public HordeTerminator {

public:
    explicit CadicalTimeoutTerminator(Logger &logger) : HordeTerminator(logger) {};

    void setSolverStarted() {
        _last_start_seconds = Timer::elapsedSeconds();
        _solver_started = true;
    }

    void setSolverFinished() {
        _solver_started = false;
    }

    void enableTimeout(float timeout) {
        _timeout_seconds = timeout;
        _timeout_enabled = true;
    }

    void disableTimeout() {
        _timeout_enabled = false;
    }

    bool terminate() override {
        if (HordeTerminator::terminate()) return true;

        if (!_timeout_enabled) return false;
        if (!_solver_started) return false;
        float elapsedTimeSinceSolverStarted = Timer::elapsedSeconds() - _last_start_seconds;
        if (elapsedTimeSinceSolverStarted > _timeout_seconds) LOG(V5_DEBG, "[CPCS] TIMEOUT OF SOLVER after %.2f\n", elapsedTimeSinceSolverStarted);
        return elapsedTimeSinceSolverStarted > _timeout_seconds;
    }

private:
    float _last_start_seconds;
    bool _solver_started = false;
    float _timeout_seconds;
    bool _timeout_enabled = false;
};