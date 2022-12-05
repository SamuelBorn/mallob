#pragma once

#include <array>
#include "comm/message_subscription.hpp"
#include "app/job.hpp"

class Job;

class InterJobCommunicator {
public:
    InterJobCommunicator(Job &job);

private:
    MessageSubscription join_ring_request_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST, [&](auto &h) {
        handleJoinRingRequest(h);
    });
    MessageSubscription join_ring_request_accept_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST_ACCEPT, [&](auto &h) {
        handleJoinRingRequestAccept(h);
    });
    Job &job;
    std::list<int> open_join_request_ranks;
    int reductionInstanceCounter = -1;

public:
    void gatherIntoRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm, int reductionInstanceCounter);
    void handleOpenJoinRingRequests();

private:
    void handleJoinRingRequest(MessageHandle &h);
    void handleJoinRingRequestAccept(MessageHandle &h);
    bool createNewRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm);

    void acceptIntoRing(int rankToJoin);
};


