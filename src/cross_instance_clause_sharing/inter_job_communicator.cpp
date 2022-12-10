#include "data/job_transfer.hpp"
#include "comm/msg_queue/message_subscription.hpp"
#include "inter_job_communicator.hpp"

#include <iostream>


void InterJobCommunicator::gatherIntoRing(std::map<int, std::pair<int, bool>> &reps, int reduction_call_counter) {
    assert(_group_id != -2);
    if (partOfRing() || createNewRing(reps)) {
        _reduction_call_counter = reduction_call_counter;
        return;
    }
    _reduction_call_counter = reduction_call_counter;

    MyMpi::isend(reps[_group_id].first, MSG_JOIN_RING_REQUEST, IntVec({_group_id, _rank, _reduction_call_counter}));
}

// If the job is a representative that is not part of a ring  →  create a ring by himself
bool InterJobCommunicator::createNewRing(std::map<int, std::pair<int, bool>> &reps) {
    assert(reps.count(_group_id));
    if (reps[_group_id].first == _rank && !reps[_group_id].second) {
        _next_ring_member_rank = _rank;
        return true;
    }
    return false;
}

void InterJobCommunicator::handleOpenJoinRingRequests() {
    for (const auto &rankToJoin: _open_join_request_ranks) {
        acceptIntoRing(rankToJoin);
    }
}

void InterJobCommunicator::handleJoinRingRequest(MessageHandle &h) {
    auto x = Serializable::get<IntVec>(h.getRecvData()).data;
    assert(x.size() == 3);
    int newGroupId = x.at(0);
    int rankToJoin = x.at(1);
    int requestReductionInstanceCounter = x.at(2);

    if (newGroupId != _group_id) return;

    if (requestReductionInstanceCounter > _reduction_call_counter) {
        _open_join_request_ranks.emplace_back(rankToJoin);
        return;
    }

    acceptIntoRing(rankToJoin);
}

void InterJobCommunicator::acceptIntoRing(int rankToJoin) {
    auto temp = _next_ring_member_rank;
    _next_ring_member_rank = rankToJoin;
    MyMpi::isend(rankToJoin, MSG_JOIN_RING_REQUEST_ACCEPT, IntVec({_group_id, temp}));
}

void InterJobCommunicator::handleJoinRingRequestAccept(MessageHandle &h) {
    auto x = Serializable::get<IntVec>(h.getRecvData()).data;
    assert(x.size() == 2);
    int newGroupId = x.at(0);
    int nextRingNode = x.at(1);

    if (newGroupId != _group_id) return;
    _next_ring_member_rank = nextRingNode;
}

void InterJobCommunicator::setGroupId(int group_id) {
    _group_id = group_id;
}

bool InterJobCommunicator::partOfRing() {
    return _next_ring_member_rank != -1;
}

int InterJobCommunicator::getNextRingMemberRank() {
    return _next_ring_member_rank;
}

InterJobCommunicator::InterJobCommunicator() = default;
