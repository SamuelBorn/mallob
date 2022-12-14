#include <iostream>
#include "util/assert.hpp"
#include <vector>
#include <string>
#include <map>
#include <chrono>

#include "util/random.hpp"
#include "util/logger.hpp"
#include "util/sys/timer.hpp"
#include "comm/mympi.hpp"
#include "util/params.hpp"
#include "comm/async_collective.hpp"
#include "util/sys/threading.hpp"
#include "cross_problem_clause_sharing/group_sharing_map.hpp"
#include "cross_problem_clause_sharing/inter_job_communicator.hpp"
#include "comm/msg_queue/message_subscription.hpp"
#include "data/job_transfer.hpp"

int reductionCallCounter = 0;
int reductionInstanceCounter = 0;

void testGroupSharingMap() {
    GroupSharingMap m1;
    GroupSharingMap m2;

    m1.data[1] = {1, true};
    m1.data[2] = {2, false};
    m1.data[3] = {3, false};

    m2.data[1] = {4, false};
    m2.data[2] = {5, true};
    m2.data[4] = {6, false};

    m1.aggregate(m2);
    assert(m1.data.size() == 4);

    assert(m1.data[1].first == 1);
    assert(m1.data[2].first == 5);
    assert(m1.data[3].first == 3);
    assert(m1.data[4].first == 6);
}

void testGroupSharingMap2() {
    MPI_Comm comm = MPI_COMM_WORLD;
    int rank = MyMpi::rank(comm);
    auto &q = MyMpi::getMessageQueue();
    Terminator::reset();

    AsyncCollective<GroupSharingMap> allRed(comm, q, reductionInstanceCounter++);
    MPI_Barrier(MPI_COMM_WORLD);

    InterJobCommunicator ijc;
    int group_id = rank % 2;
    ijc.setGroupId(group_id);
    GroupSharingMap contribution({{group_id, {rank, false}}});

    allRed.allReduce(reductionCallCounter++, contribution, [&](std::list<GroupSharingMap> &results) {
        std::map<int, std::pair<int, bool>> reps = results.front().data;
        assert(reps.size() == 2);
        assert(reps.at(group_id).first % 2 == rank % 2);
        assert(reps.at(group_id).second == false);
        Terminator::setTerminating();
    });

    while (!Terminator::isTerminating() || q.hasOpenSends()) q.advance();
}

void testIJC() {
    MPI_Comm comm = MPI_COMM_WORLD;
    int rank = MyMpi::rank(comm);
    auto &q = MyMpi::getMessageQueue();
    Terminator::reset();

    AsyncCollective<GroupSharingMap> allRed(comm, q, reductionInstanceCounter++);
    MPI_Barrier(MPI_COMM_WORLD);

    InterJobCommunicator ijc;
    int group_id = rank % 2;
    ijc.setGroupId(group_id);
    GroupSharingMap contribution({{group_id, {rank, false}}});

    std::map<int, std::pair<int, bool>> reps;
    allRed.allReduce(reductionCallCounter++, contribution, [&](std::list<GroupSharingMap> &results) {
        reps = results.front().data;
        ijc.gatherIntoRing(reps, reductionCallCounter);
        ijc.handleOpenJoinRingRequests();
        //for (int i = 0; i < 5000; ++i) MyMpi::getMessageQueue().advance();
        auto begin = std::chrono::steady_clock::now();
        while (!ijc.partOfRing() || ijc.getNextRingMemberRank() == rank) {
            MyMpi::getMessageQueue().advance();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() > 5000) assert("took too long to gather into ring" && false);
        }
        assert(ijc.partOfRing());
        assert(MyMpi::size(MPI_COMM_WORLD) < 4 | ijc.getNextRingMemberRank() != rank);
        Terminator::setTerminating();
    });

    while (!Terminator::isTerminating() || q.hasOpenSends()) q.advance();
}

void testMessagePassing() {
    auto comm = MPI_COMM_WORLD;
    auto size = MyMpi::size(comm);
    auto rank = MyMpi::rank(comm);
    assert(size >= 4);

    auto group_id = rank % 2;
    InterJobCommunicator ipc;
    ipc.setGroupId(group_id);
    ipc.setNextRingMemberRank((rank + 2) % size);
    MPI_Barrier(comm);

    auto x = IntVec({1});
    ipc.emitMessageIntoRing(x);
    while (MyMpi::getMessageQueue().hasOpenSends()) MyMpi::getMessageQueue().advance();
    auto begin = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() < 1000){
        end = std::chrono::steady_clock::now();
        MyMpi::getMessageQueue().advance();
    }
}


int main(int argc, char *argv[]) {

    MyMpi::init();
    Timer::init();
    int rank = MyMpi::rank(MPI_COMM_WORLD);

    Process::init(rank);

    Random::init(rand(), rand());
    Logger::init(rank, V5_DEBG);

    Parameters params;
    params.init(argc, argv);
    MyMpi::setOptions(params);

    //testIntegerSum();
    //testGroupSharingMap2();
    //testIJC();
    testMessagePassing();;

    // Exit properly
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    LOG(V2_INFO, "Exiting happily\n");
    Process::doExit(0);
}
