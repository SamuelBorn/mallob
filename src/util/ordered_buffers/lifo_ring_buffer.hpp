#pragma once

#include "synchronized_ordered_buffer.hpp"

template<typename T>
class LIFORingBuffer : public SynchronizedOrderedBuffer<T> {
private:
    const size_t _max_clause_count;
    Mutex _clauses_mutex;
    std::list<T> _elements;

public:
    explicit LIFORingBuffer(const size_t maxClauseCount) : _max_clause_count(maxClauseCount) {}

    // new elements that dont fit get discarded
    bool insert(T element) override {
        auto lock = _clauses_mutex.getLock();
        if (_elements.size() >= _max_clause_count) {
            _elements.pop_back();
        };
        _elements.push_front(element);
        return true;
    }

    T extract() override {
        assert(!empty());
        auto lock = _clauses_mutex.getLock();
        auto clause = _elements.front();
        _elements.pop_front();
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