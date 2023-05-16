#pragma once
#include <cstdint>

#include "sendy/encode.hpp"

namespace sendy {

enum class packet_kind : std::uint8_t  {
    Connect = 1,
};

typedef std::uint64_t packetid;

struct packet_header {
    packetid id;
    packet_kind kind;

    inline constexpr auto encode(auto packer) { return packer(id, kind); }
    inline constexpr bool operator==(packet_header const&) const = default;
};

static_assert(encodable<packet_header>);

}
