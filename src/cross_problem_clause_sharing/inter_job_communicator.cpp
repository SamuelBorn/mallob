#include "data/job_transfer.hpp"
#include "comm/msg_queue/message_subscription.hpp"
#include "inter_job_communicator.hpp"

#include <iostream>


void InterJobCommunicator::gatherIntoRing(std::map<int, std::pair<int, bool>> &reps, int reduction_call_counter) {
    assert(_group_id != -2);
    if (isRingMember() || createNewRing(reps)) {
        _reduction_call_counter = reduction_call_counter;
        return;
    }
    _reduction_call_counter = reduction_call_counter;

    LOG(V4_VVER, "[CPCS] Sending join req to %i\n", reps[_group_id].first);
    MyMpi::isend(reps[_group_id].first, MSG_JOIN_RING_REQUEST, IntVec({_group_id, _rank, _reduction_call_counter}));
}

// If the job is a representative that is not part of a ring  →  create a ring by himself
bool InterJobCommunicator::createNewRing(std::map<int, std::pair<int, bool>> &reps) {
    //assert(reps.count(_group_id));
    if (reps[_group_id].first == _rank && !reps[_group_id].second) {
        LOG(V4_VVER, "[CPCS] Create new ring by myself\n");
        _next_ring_member_rank = _rank;
        return true;
    }
    return false;
}

void InterJobCommunicator::handleOpenJoinRingRequests() {
    for (const auto &rankToJoin: _open_join_request_ranks) {
        LOG(V5_DEBG, "[CPCS] handleOpenJoinRingRequests\n");
        acceptIntoRing(rankToJoin);
    }
    _open_join_request_ranks.clear();
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

int InterJobCommunicator::getGroupId() {
    return _group_id;
};

bool InterJobCommunicator::isRingMember() {
    return _next_ring_member_rank != -1;
}

int InterJobCommunicator::getNextRingMemberRank() {
    return _next_ring_member_rank;
}

void InterJobCommunicator::emitMessageIntoRing(std::vector<uint8_t> &payload) {
    assert(isRingMember());
    if (_next_ring_member_rank == _rank) return;
    auto r = RingMessage(_group_id, _rank, payload);
    LOG(V5_DEBG, "[CPCS] IJC emit into ring, sending to %i\n", _next_ring_member_rank);
    MyMpi::isend(_next_ring_member_rank, MSG_RING_MESSAGE, r);
}

void InterJobCommunicator::emitMessageIntoRing(Serializable &s) {
    auto packed = s.serialize();
    emitMessageIntoRing(packed);
}

void InterJobCommunicator::setNextRingMemberRank(int nextRingMemberRank) {
    _next_ring_member_rank = nextRingMemberRank;
}

void InterJobCommunicator::resetRingStatus() {
    setNextRingMemberRank(-1);
}

void InterJobCommunicator::forwardRingMessage(MessageHandle &h) {
    if (not isRingMember()) return;
    if (_next_ring_member_rank == _rank) return;
    auto ring_message = Serializable::get<RingMessage>(h.getRecvData());
    if (ring_message.group_id != _group_id) {
        LOG(V4_VVER, "[CPCS] WARNING THIS MESSAGE WAS SENT TO THE WRONG GROUP ID → this can happen when a job finishes and a new job with a different group_id joins\n");
        return;
    }
    if (_ring_action) _ring_action(ring_message);
    if (ring_message.msg_start_rank == _next_ring_member_rank) return;
    LOG(V5_DEBG, "[CPCS] IJC forward to %i, start=%i\n", _next_ring_member_rank, ring_message.msg_start_rank);
    MyMpi::isend(_next_ring_member_rank, MSG_RING_MESSAGE, ring_message);
}

void InterJobCommunicator::setRingAction(std::function<void(RingMessage &)> callback) {
    _ring_action = callback;
}

InterJobCommunicator::InterJobCommunicator() {
    LOG(V5_DEBG, "[CPCS] IJC created\n");
}
