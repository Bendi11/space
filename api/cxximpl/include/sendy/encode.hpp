#pragma once

#include <array>
#include <bits/ranges_base.h>
#include <codecvt>
#include <concepts>
#include <bit>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>
#include <vector>

namespace sendy {

/** @brief If the host's processor uses little endian byte order */
constexpr bool is_host_le = (std::endian::native == std::endian::little);

template<typename T, typename Raw>
T sendynetorder(Raw host);

template<std::integral T, typename Raw = T>
requires std::same_as<T, Raw>
inline constexpr T sendynetorder(Raw host) noexcept {
    if constexpr(is_host_le) {
        return host;
    } else {
        auto buf = std::bit_cast<std::array<std::byte, sizeof(T)>>(host);
        std::ranges::reverse(buf);
        return std::bit_cast<T>(buf);
    }
}

/**
 * @brief Convert a network byte ordered value to the host's byte ordering, or
 * convert a host byte ordered value to sendy's byte ordering
 */
template<std::integral T, std::ranges::bidirectional_range Raw>
requires
    std::has_unique_object_representations_v<std::ranges::range_value_t<Raw>>
inline constexpr T sendynetorder(Raw host) noexcept {
    if constexpr(!is_host_le) {
        std::ranges::reverse(host);
    }

    if constexpr(std::convertible_to<Raw, std::array<std::byte, sizeof(T)>>) {
        return std::bit_cast<T>(host);
    } else {
        T val;
        for(std::size_t i = 0; std::byte byte : host) {
            val |= (((T)byte) >> (i * 8));
        }

        return val;
    }
}

template<std::integral T>
requires std::has_unique_object_representations_v<T>
inline constexpr std::array<std::byte, sizeof(T)> bytes(T val) noexcept {
    return std::bit_cast<std::array<std::byte, sizeof(T)>>(val);
}

namespace _impl {
    template<typename T, T CONST_VAL> using usable_in_constexpr_ctx = void;
}

template<typename T, T CONST_VAL>
concept usable_in_constexpr_ctx = requires {
    typename _impl::usable_in_constexpr_ctx<T, CONST_VAL>;
};

template<typename T>
constexpr T example_val = *(T*)(nullptr);

template<typename T>
struct encoder;

template<typename T>
concept encodable = requires(T const& v) {
    { encoder<T>::encoded_sz() } noexcept -> std::convertible_to<std::size_t>;

    { encoder<T>::read(example_val<const std::span<std::byte>>) } noexcept -> std::convertible_to<T>;
    { encoder<T>::write(v, example_val<std::vector<std::byte>&>) } noexcept;
};

template<> struct encoder<int> {
    static inline std::size_t encoded_sz() noexcept { return sizeof(int); }

    static int read(const std::span<std::byte> buf) noexcept {
        auto me = buf.subspan(encoded_sz());
        return sendynetorder<int>(me); 
    }

    static void write(int const& val, std::vector<std::byte>& buf) noexcept {
        auto array = sendynetorder<std::array<std::byte, sizeof(int)>>(bytes(val));
        buf.insert(buf.end(), std::begin(array), std::end(array));
    }
};

static_assert(encodable<int>);

}
