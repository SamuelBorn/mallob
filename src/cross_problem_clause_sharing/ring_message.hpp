#pragma once

struct RingMessage : Serializable {
    int group_id;
    int msg_start_rank;
    std::vector<uint8_t> payload;

    RingMessage() = default;

    RingMessage(int groupId, int msgStartRank, const std::vector<uint8_t> &payload) : group_id(groupId),
                                                                                      msg_start_rank(msgStartRank),
                                                                                      payload(payload) {}

    virtual std::vector<uint8_t> serialize() const override {
        std::vector<uint8_t> result(2 * sizeof(int) + payload.size());
        memcpy(result.data(), &group_id, sizeof(int));
        memcpy(result.data() + sizeof(int), &msg_start_rank, sizeof(int));
        memcpy(result.data() + 2 * sizeof(int), payload.data(), payload.size());
        return result;
    }

    virtual RingMessage &deserialize(const std::vector<uint8_t> &packed) override {
        payload = std::vector<uint8_t>(packed.size() - 2 * sizeof(int));
        memcpy(&group_id, packed.data(), sizeof(int));
        memcpy(&msg_start_rank, packed.data() + sizeof(int), sizeof(int));
        memcpy(payload.data(), packed.data() + 2 * sizeof(int), packed.size() - 2 * sizeof(int));
        return *this;
    }
};