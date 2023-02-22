
#include "util/sys/timer.hpp"
#include "util/random.hpp"
#include "util/logger.hpp"
#include "util/sys/process.hpp"
#include "util/sys/thread_pool.hpp"

#include "util/morton/morton_filter.hpp"
#include "util/morton/hierarchical_morton_filter.hpp"

void testMortonFilter() {
    const int numItems = 10'000'000;
    const float overloadFactor = 1;

    MortonFilter filter(numItems);

    filter.registerItem(3);
    assert(!filter.likely_contains(4));
    std::cout << filter.registerItem(3) << std::endl;
    std::cout << filter.registerItem(4) << std::endl;

    int numFalsePositives = 0;
    for (size_t i = 0; i < (int) (overloadFactor * numItems); i++) {
        if (!filter.registerItem(i)) numFalsePositives++;
    }

    LOG(V2_INFO, "n=%i of=%.3f #fp=%i bitsperelem=%.3f\n", numItems, overloadFactor, numFalsePositives,
        (8 * filter.getSizeInBytes()) / (overloadFactor * numItems));

    for (size_t i = 0; i < (int) (overloadFactor * numItems); i++) {
        assert(filter.likely_contains(i));
        //assert(!filter.registerItem(i));
    }
}

void testHierarchicalMorton() {
    const int numItems = 1'000'000;
    const float overloadFactor = 1;

    HierarchicalMortonFilter filter;

    int numFalsePositives = 0;
    for (size_t i = 0; i < (int) (overloadFactor * numItems); i++) {
        if (!filter.registerItem(i)) numFalsePositives++;
    }

    LOG(V2_INFO, "n=%i of=%.3f #fp=%i #layers=%i\n", numItems, overloadFactor, numFalsePositives, filter.filters.size());

    for (size_t i = 0; i < (int) (overloadFactor * numItems); i++) {
        assert(filter.likely_contains(i));
    }
}

int main() {
    Timer::init();
    Random::init(rand(), rand());
    Logger::init(0, V5_DEBG);
    Process::init(0);
    ProcessWideThreadPool::init(1);

    testHierarchicalMorton();
}
