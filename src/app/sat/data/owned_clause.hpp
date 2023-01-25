#pragma once

#include "clause.hpp"

// Wrapper for clause class that automatically frees memory of clauses when object goes out of scope
// Only use for Clauses that were created with clause.copy() as there the begin field is malloc'ed

struct OwnedClause {
public:
    Mallob::Clause stored_clause;

    explicit OwnedClause(const Mallob::Clause clause) : stored_clause(clause) {}

    virtual ~OwnedClause() {
        free(stored_clause.begin);
    }

    bool operator<(const OwnedClause& other) const {
        return this->stored_clause < other.stored_clause;
    }
};