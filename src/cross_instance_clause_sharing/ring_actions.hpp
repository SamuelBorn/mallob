#pragma once

#include <vector>
#include <cstdint>

class RingAction {
public:
    virtual ~RingAction() = default;

    virtual void execute(std::vector<uint8_t> &input) = 0;
};

class NOPRingAction : public RingAction {
public:
    void execute(std::vector<uint8_t> &input) override {
        std::cout << MyMpi::rank(MPI_COMM_WORLD) << " executed NOP" << std::endl;
    };
};

class ApplyClausesRingAction : public RingAction {
public:
    void execute(std::vector<uint8_t> &clauses) override {
        // TODO: Apply the given clauses to the current job
    }
};

