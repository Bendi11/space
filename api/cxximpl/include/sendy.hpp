#pragma once
#include <cstdint>
#include <concepts>
#include <vector>

#include "sendy/encode.hpp"

namespace sendy {

enum class PacketKind : std::uint8_t  {
    Connect = 1,
};

typedef std::uint64_t packetid;

template<typename... Fields>
struct fields {
    using tuple = std::tuple<Fields...>;

    constexpr void operator()(Fields&&... fields) {
        
    }
};

template<typename... Fields>
struct any {
    static constexpr bool value = ( ... || std::is_same_v<Fields, int>);
};

struct PacketHeader {
    packetid id;
    PacketKind kind;

    template<auto packer> constexpr auto encode() { packer(id, kind); }
};

static_assert(encodable<PacketHeader>);

}
