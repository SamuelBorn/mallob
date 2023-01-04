#pragma once

#include "comm/msg_queue/message_subscription.hpp"
#include "ring_message.hpp"
#include <iostream>

class InterJobCommunicator {

private:
    MessageSubscription _join_ring_request_subscription = {MSG_JOIN_RING_REQUEST, [&](auto &h) { handleJoinRingRequest(h); }};
    MessageSubscription _join_ring_request_accept_subscription = {MSG_JOIN_RING_REQUEST_ACCEPT, [&](auto &h) { handleJoinRingRequestAccept(h); }};
    MessageSubscription _pass_ring_message_subscription = {MSG_RING_MESSAGE, [&](auto &h) { forwardRingMessage(h); }};
    std::function<void (RingMessage&)> _ring_action;

    int _next_ring_member_rank = -1;
    int _group_id = -2;  // group_id of -1 indicates that no group id is set
    int _reduction_call_counter = -1;
    int _rank = MyMpi::rank(MPI_COMM_WORLD);
    std::list<int> _open_join_request_ranks;

public:
    InterJobCommunicator();

    void gatherIntoRing(std::map<int, std::pair<int, bool>> &reps, int reduction_call_counter);

    void handleOpenJoinRingRequests();

    bool partOfRing();

    void setGroupId(int group_id);

    int getNextRingMemberRank();

    void setNextRingMemberRank(int nextRingMemberRank);

    void emitMessageIntoRing(Serializable &s);

    void emitMessageIntoRing(std::vector<uint8_t> &payload);

    void setRingAction(std::function<void(RingMessage &)> callback);

private:
    bool createNewRing(std::map<int, std::pair<int, bool>> &reps);

    void acceptIntoRing(int rankToJoin);

    void handleJoinRingRequest(MessageHandle &h);

    void handleJoinRingRequestAccept(MessageHandle &h);

    void forwardRingMessage(MessageHandle &h);

};


