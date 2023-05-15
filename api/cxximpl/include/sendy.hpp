#pragma once
#include <cstdint>
#include <concepts>
#include <vector>

template<typename T>
consteval T val() {
    uint8_t v[sizeof(T)];
    return reinterpret_cast<T>(v);
}

template<typename T>
concept Sendable = requires(T const& v) {
    { v.write(val<std::vector<std::uint8_t>&>()) };
    { T::read } -> std::invocable<std::uint8_t*, std::size_t>;
    { T::read(val<std::uint8_t*>(), val<std::size_t>()) } -> std::convertible_to<T>;
};

#pragma pack(push, 1)

enum class PacketKind : std::uint8_t  {
    Connect = 1,
};

typedef std::uint64_t packetid;

template<typename Buf, typename T> void write(Buf&& buf, T&& v);
template<typename T>
void write_buf(std::vector<uint8_t>& buf, T&& v) {
    for(auto b : reinterpret_cast<const uint8_t (&&)[sizeof(T)]>(std::forward<T>(v))) {
        buf.push_back(b);
    }
}

struct PacketHeader {
    packetid id;
    PacketKind kind;

    void write(std::vector<std::uint8_t>& buf) const {
        write_buf(buf, id);
        write_buf(buf, kind);
    }
    static PacketHeader read(std::uint8_t *buf, std::size_t sz); 
};

static_assert(Sendable<PacketHeader>);

#pragma pack(pop)
