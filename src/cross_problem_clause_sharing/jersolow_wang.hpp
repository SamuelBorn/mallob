#pragma once


std::vector<std::vector<int>> split_flits(const size_t fSize, const int *fLits) {
    auto ret = std::vector<std::vector<int>>();

    auto clause = std::vector<int>();
    for (int i = 0; i < fSize; ++i) {
        if (fLits[i] == 0) {
            auto new_clause = clause;
            ret.push_back(new_clause);
            clause.clear();
        } else {
            clause.push_back(fLits[i]);
        }
    }

    return  ret;
}

int get_max_lit(const size_t fSize, const int *fLits) {
    int max = -1;
    for (int i = 0; i < fSize; ++i) {
        if (std::abs(fLits[i]) > max) max = std::abs(fLits[i]);
    }
    assert(max != -1);
    return max;
}

std::vector<int> get_lit_rankings(const size_t fSize, const int *fLits) {
    auto split = split_flits(fSize, fLits);
    auto max_lit = get_max_lit(fSize, fLits);
    auto rankings = std::vector<int>(max_lit + 1);

    for (const auto &clause: split) {
        for (const auto &literal: clause) {
            rankings.at(std::abs(literal)) += (int) std::pow(2, clause.size());
        }
    }

    return rankings;
}