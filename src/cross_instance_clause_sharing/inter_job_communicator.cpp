#include "app/job.hpp"
#include "comm/message_subscription.hpp"
#include "inter_job_communicator.hpp"


void InterJobCommunicator::gatherIntoRing(std::map<int, std::pair<int, bool>> reps, int my_rank,
                                          int newReductionInstanceCounter) {
    assert(job.getJobTree().isRoot());
    if (job.isPartOfRing()) {
        reductionInstanceCounter = newReductionInstanceCounter;
        return;
    }

    if (createNewRing(reps, my_rank)) {
        reductionInstanceCounter = newReductionInstanceCounter;
        return;
    }
    reductionInstanceCounter = newReductionInstanceCounter;

    MyMpi::isend(reps[job.getDescription().getGroupId()].first, MSG_JOIN_RING_REQUEST,
                 IntVec({job.getDescription().getGroupId(), my_rank, reductionInstanceCounter}));
}

// If the job is a representative that is not part of a ring  →  create a ring by himself
bool InterJobCommunicator::createNewRing(std::map<int, std::pair<int, bool>> reps, int my_rank) {
    if (reps[job.getDescription().getGroupId()].first == my_rank && !reps[job.getDescription().getGroupId()].second) {
        job.setNextRingMemberRank(my_rank);
        return true;
    }
    return false;
}

void InterJobCommunicator::handleOpenJoinRingRequests() {
    for (const auto &rankToJoin: open_join_request_ranks) {
        acceptIntoRing(rankToJoin);
    }
}

void InterJobCommunicator::handleJoinRingRequest(MessageHandle &h) {
    auto x = Serializable::get<IntVec>(h.getRecvData()).data;
    assert(x.size() == 3);
    int newGroupId = x.at(0);
    int rankToJoin = x.at(1);
    int requestReductionInstanceCounter = x.at(2);

    if (newGroupId != job.getDescription().getGroupId()) return;

    if (requestReductionInstanceCounter > reductionInstanceCounter) {
        open_join_request_ranks.emplace_back(rankToJoin);
        return;
    }

    acceptIntoRing(rankToJoin);
}

void InterJobCommunicator::acceptIntoRing(int rankToJoin) {
    auto nextRingNode = job.getNextRingMemberRank();
    job.setNextRingMemberRank(rankToJoin);
    MyMpi::isend(rankToJoin, MSG_JOIN_RING_REQUEST_ACCEPT, IntVec({job.getDescription().getGroupId(), nextRingNode}));
}

void InterJobCommunicator::handleJoinRingRequestAccept(MessageHandle &h) {
    auto x = Serializable::get<IntVec>(h.getRecvData()).data;
    assert(x.size() == 2);
    int newGroupId = x.at(0);
    int nextRingNode = x.at(1);

    if (newGroupId != job.getDescription().getGroupId()) return;

    job.setNextRingMemberRank(nextRingNode);
}

InterJobCommunicator::InterJobCommunicator(Job &job) : job(job) {}