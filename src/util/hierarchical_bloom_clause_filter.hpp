#pragma once

#include "app/sat/sharing/filter/clause_filter.hpp"

class HierarchicalBloomClauseFilter {
private:
    const size_t _initial_buffer_size = 4096;
    const int _bits_per_element = 10;  // close enough to 9.6f which is for 1% false positive rate and allows for int multiplication
    size_t _current_buffer_size = _initial_buffer_size;
    size_t _elements_in_current_buffer = 0;
    const size_t _filter_grow_factor = 2;

    std::list<ClauseFilter> _filters;
    const int _max_clause_length;

public:
    HierarchicalBloomClauseFilter(int maxClauseLength) : _max_clause_length(maxClauseLength) {
        _filters.emplace_back(_initial_buffer_size * _bits_per_element, _max_clause_length);
    }

    /**
     * Return false if the given clause has already been registered
     * otherwise add it to the filter and return true.
     */
    bool registerClause(Clause c) {
        for (auto &clauseFilter: _filters) {
            if (clauseFilter.likely_contains(c.begin, c.size)) return false;
        }

        _filters.back().registerClause(c.begin, c.size);
        _elements_in_current_buffer++;

        if (_elements_in_current_buffer >= _current_buffer_size) {
            LOG(V4_VVER, "[CPCS] Creating new AMQ Clause Buffer Layer\n");
            _elements_in_current_buffer = 0;
            _current_buffer_size = _filter_grow_factor * _current_buffer_size;
            _filters.emplace_back(_initial_buffer_size * _bits_per_element, _max_clause_length);
        }

        return true;
    }
};