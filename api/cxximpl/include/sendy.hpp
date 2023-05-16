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

struct PacketHeader {
    packetid id;
    PacketKind kind;

    inline constexpr auto encode(auto packer) { return packer(id, kind); }
};

static_assert(encodable<PacketHeader>);

}
