#pragma once


#include "synchronized_ordered_buffer.hpp"

template<typename T>
class MaxRingBuffer : public SynchronizedOrderedBuffer<T> {
private:
    const size_t _max_clause_count;
    Mutex _clauses_mutex;
    std::multiset<T> _elements;

public:
    explicit MaxRingBuffer(const size_t maxClauseCount) : _max_clause_count(maxClauseCount) {}

    // new elements that dont fit get discarded
    bool insert(T element) override {
        auto lock = _clauses_mutex.getLock();
        bool enough_space = true;
        if (_elements.size() >= _max_clause_count) {
            enough_space = false;
            auto last_element_iterator = std::prev(_elements.end());
            if (element < *last_element_iterator) {
                return enough_space;
            } else {
                _elements.erase(last_element_iterator);
            }
        };
        _elements.insert(element);
        return enough_space;
    }

    T extract() override {
        assert(!empty());
        auto lock = _clauses_mutex.getLock();
        auto clause = *std::prev(_elements.end());
        _elements.erase(std::prev(_elements.end()));
        return clause;
    }

    bool empty() override {
        auto lock = _clauses_mutex.getLock();
        return _elements.empty();
    }

    int size() override {
        auto lock = _clauses_mutex.getLock();
        return _elements.size();
    }
};