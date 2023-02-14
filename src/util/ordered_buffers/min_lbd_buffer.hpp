#pragma once

#include "app/sat/data/owned_clause.hpp"
#include "synchronized_ordered_buffer.hpp"


class MinLBDBuffer : public SynchronizedOrderedBuffer<OwnedClause> {

private:
    const size_t _max_clause_count;
    Mutex _clauses_mutex;

    struct cmp {
        bool operator()(const OwnedClause &lhs, const OwnedClause &rhs) const {
            return lhs.stored_clause.lbd < rhs.stored_clause.lbd;
        }
    };

    std::multiset<OwnedClause, cmp> _elements;

public:
    explicit MinLBDBuffer(const size_t maxClauseCount) : _max_clause_count(maxClauseCount) {}

    // new elements that dont fit get discarded
    bool insert(OwnedClause element) override {
        auto cmp = [](int a, int b) { return a < b; };
        std::multiset<int, decltype(cmp)> s(cmp);


        auto lock = _clauses_mutex.getLock();
        bool enough_space = true;
        if (_elements.size() >= _max_clause_count) {
            enough_space = false;
            auto last_element_iterator = std::prev(_elements.end());
            if (element.stored_clause.lbd < last_element_iterator->stored_clause.lbd) {
                _elements.erase(last_element_iterator);
            } else {
                return enough_space;
            }
        };
        _elements.insert(element);
        return enough_space;
    }

    OwnedClause extract() override {
        assert(!empty());
        auto lock = _clauses_mutex.getLock();
        auto clause = *_elements.begin();
        _elements.erase(_elements.begin());
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