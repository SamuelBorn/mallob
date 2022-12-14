#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <cstring>
#include "data/reduceable.hpp"

struct GroupSharingMap : public Reduceable {
    // map:    group_id → (rank, is_part_of_ring)
    std::map<int, std::pair<int, bool>> data;

    GroupSharingMap() = default;
    explicit GroupSharingMap(const std::map<int, std::pair<int, bool>> &data) : data(data) {}

    virtual std::vector<uint8_t> serialize() const override {
        size_t i = 0;
        std::vector<uint8_t> result(data.size() * (2 * sizeof(int) + sizeof(bool)));
        for (auto &[key, val]: data) {
            memcpy(result.data() + i, &key, sizeof(int));
            i += sizeof(int);
            memcpy(result.data() + i, &val.first, sizeof(int));
            i += sizeof(int);
            memcpy(result.data() + i, &val.second, sizeof(bool));
            i += sizeof(bool);
        }
        return result;
    }

    virtual GroupSharingMap &deserialize(const std::vector<uint8_t> &packed) override {
        data.clear();
        size_t i = 0;
        while (i < packed.size()) {
            int group_id, rank;
            bool ring_member;
            memcpy(&group_id, packed.data() + i, sizeof(int));
            i += sizeof(int);
            memcpy(&rank, packed.data() + i, sizeof(int));
            i += sizeof(int);
            memcpy(&ring_member, packed.data() + i, sizeof(bool));
            i += sizeof(bool);
            data[group_id] = {rank, ring_member};
        }
        return *this;
    }

    virtual void aggregate(const Reduceable &other) {
        GroupSharingMap *other_map = (GroupSharingMap *) &other;
        for (auto &[key, val]: other_map->data) {
            auto is_ring_member = val.second;
            auto ring_member_exists = data[key].second;  // data[key].second is false if entry does not exist
            if (is_ring_member || !ring_member_exists) {
                data[key] = val;
            }
        }
    }

    virtual bool isEmpty() const {
        return data.empty();
    }
};
