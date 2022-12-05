#include "app/job.hpp"
#include "comm/message_subscription.hpp"
#include "inter_job_communicator.hpp"


void InterJobCommunicator::gatherIntoRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm,
                                          int newReductionInstanceCounter) {
    if (job.isPartOfRing()) {
        reductionInstanceCounter = newReductionInstanceCounter;
        return;
    }

    if (createNewRing(reps, _comm)) {
        reductionInstanceCounter = newReductionInstanceCounter;
        return;
    }
    reductionInstanceCounter = newReductionInstanceCounter;

    MyMpi::isend(reps[group_id].first, MSG_JOIN_RING_REQUEST,
                 IntVec({group_id, MyMpi::rank(_comm), reductionInstanceCounter}));
}

// If the job is a representative that is not part of a ring  →  create a ring by himself
bool InterJobCommunicator::createNewRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm) {
    if (reps[group_id].first == MyMpi::rank(_comm) && !reps[group_id].second) {
        job.setNextRingMemberRank(MyMpi::rank(_comm));
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
    int newGroupId = x.at(0);
    int rankToJoin = x.at(1);
    int requestReductionInstanceCounter = x.at(2);

    if (newGroupId != group_id) return;

    if (requestReductionInstanceCounter > reductionInstanceCounter) {
        open_join_request_ranks.emplace_back(rankToJoin);
        return;
    }

    acceptIntoRing(rankToJoin);
}

void InterJobCommunicator::acceptIntoRing(int rankToJoin) {
    auto nextRingNode = job.getNextRingMemberRank();
    job.setNextRingMemberRank(rankToJoin);
    MyMpi::isend(rankToJoin, MSG_JOIN_RING_REQUEST_ACCEPT, IntVec({group_id, nextRingNode}));
}

void InterJobCommunicator::handleJoinRingRequestAccept(MessageHandle &h) {
    auto x = Serializable::get<IntVec>(h.getRecvData()).data;
    int newGroupId = x.at(0);
    int nextRingNode = x.at(1);

    if (newGroupId != group_id) return;

    job.setNextRingMemberRank(nextRingNode);
}

InterJobCommunicator::InterJobCommunicator(Job &job) : job(job), group_id(job.getDescription().getGroupId()) {}