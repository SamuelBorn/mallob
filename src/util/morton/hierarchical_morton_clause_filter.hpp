#pragma once

#include "hierarchical_morton_filter.hpp"

class HierarchicalMortonClauseFilter {
private:
    HierarchicalMortonFilter filter;

    static uint64_t xor_hash(Clause &c) {
        auto x = ClauseHasher::hash(c, 1);
        x = x ^ ClauseHasher::hash(c, 2);
        x = x ^ ClauseHasher::hash(c, 3);
        x = x ^ ClauseHasher::hash(c, 4);
        return x;
    }

public:
    bool register_clause(Clause &c) {
        return filter.registerItem(xor_hash(c));
    }

    bool likely_contains(Clause &c) {
        return filter.likely_contains(xor_hash(c));
    }
};