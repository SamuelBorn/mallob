#pragma once

#include "comm/message_subscription.hpp"
#include "app/job.hpp"

class Job;

class InterJobCommunicator {
private:
    InterJobCommunicator(Job &job);

    MessageSubscription join_ring_request_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST, [&](auto &h) {
        handleJoinRingRequest(h);
    });
    MessageSubscription join_ring_request_accept_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST_ACCEPT, [&](auto &h) {
        handleJoinRingRequestAccept(h);
    });
    Job &job;
    int group_id;
    std::list<std::pair<int, int>> open_join_requests;

public:
    void gatherIntoRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm);

private:
    bool handleJoinRingRequest(MessageHandle &h);
    bool handleJoinRingRequestAccept(MessageHandle &h);
    bool createNewRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm);
};


