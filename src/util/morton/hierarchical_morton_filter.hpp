#pragma once

#include "morton_filter.hpp"

class HierarchicalMortonFilter {
public:
    const size_t initial_buffer_size = 8192;
    size_t current_buffer_size = initial_buffer_size;
    size_t elements_in_current_buffer = 0;
    size_t filter_grow_factor = 2;

    std::list<MortonFilter> filters;

public:
    HierarchicalMortonFilter() {
        filters.emplace_back(initial_buffer_size);
    }

    bool registerItem(uint64_t item) {
        if (likely_contains(item)) return false;

        filters.back().registerItem(item);
        elements_in_current_buffer++;

        if (elements_in_current_buffer >= current_buffer_size) {
            std::cout << elements_in_current_buffer << std::endl;
            LOG(V4_VVER, "[CPCS] Creating new Morton Buffer\n");
            elements_in_current_buffer = 0;
            current_buffer_size = filter_grow_factor * current_buffer_size;
            filters.emplace_back(current_buffer_size);
        }

        return true;
    }

    bool likely_contains(uint64_t item) {
        for (const auto &filter: filters) {
            if (filter.likely_contains(item)) return true;
        }
        return false;
    }
};