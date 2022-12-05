#include "app/job.hpp"
#include "comm/message_subscription.hpp"

void InterJobCommunicator::gatherIntoRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm) {
    if (job.isPartOfRing()) return;

    if (createNewRing(reps, _comm)) return;
    MyMpi::isend(reps[group_id].first, MSG_JOIN_RING_REQUEST, IntPair(group_id, MyMpi::rank(_comm)));
}

// If the job is a representative that is not part of a ring → create a ring by itself
bool InterJobCommunicator::createNewRing(std::map<int, std::pair<int, bool>> reps, MPI_Comm _comm) {
    if (reps[group_id].first == MyMpi::rank(_comm) && !reps[group_id].second) {
        job.setNextRingMemberRank(MyMpi::rank(_comm));
        return true;
    }
    return false;
}

InterJobCommunicator::InterJobCommunicator(Job &job) : job(job) {
    group_id = job.getDescription().getGroupId();
}
