#pragma once
#include "comm/message_subscription.hpp"

class InterJobCommunicator {

private:
    MessageSubscription _join_ring_request_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST, [&](auto &h) { handleJoinRingRequest(h); });
    MessageSubscription _join_ring_request_accept_subscription = MessageSubscription(MSG_JOIN_RING_REQUEST_ACCEPT, [&](auto &h) { handleJoinRingRequestAccept(h); });
    int _next_ring_member_rank = -1;
    int _group_id = -2;  // group_id of -1 indicates that no group id is set;
    int _reduction_instance_counter = -1;
    int _rank = MyMpi::rank(MPI_COMM_WORLD);
    std::list<int> _open_join_request_ranks;

public:
    InterJobCommunicator();
    void gatherIntoRing(std::map<int, std::pair<int, bool>> &reps, int reduction_instance_counter);
    void handleOpenJoinRingRequests();
    bool partOfRing();
    void setGroupId(int group_id);

private:
    void handleJoinRingRequest(MessageHandle &h);
    void handleJoinRingRequestAccept(MessageHandle &h);
    bool createNewRing(std::map<int, std::pair<int, bool>> &reps);
    void acceptIntoRing(int rankToJoin);
};


