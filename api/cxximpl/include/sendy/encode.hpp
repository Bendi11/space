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
#include <ranges>
#include <type_traits>
#include <vector>

namespace sendy {

template<std::integral T>
requires std::has_unique_object_representations_v<T>
inline constexpr T revbytes(T val) noexcept {
    auto buf = std::bit_cast<std::array<std::byte, sizeof(T)>>(val);
    std::ranges::reverse(buf);
    return std::bit_cast<T>(buf);
}

#if !defined(__SENDY_REVERSE_ENDIANNESS)
/** @brief If the host's processor uses little endian byte order */
constexpr bool is_host_le = (std::endian::native == std::endian::little);
#else
constexpr bool is_host_le = (std::endian::native != std::endian::little);
#endif

template<typename T, typename Raw>
T sendynetorder(Raw host);

template<std::integral T, typename Raw = T>
requires std::same_as<T, Raw>
inline constexpr T sendynetorder(Raw host) noexcept {
    if constexpr(is_host_le) {
        return host;
    } else {
        return revbytes(host);
    }
}

/**
 * @brief Convert a network byte ordered value to the host's byte ordering, or
 * convert a host byte ordered value to sendy's byte ordering
 */
template<std::integral T, std::ranges::bidirectional_range Raw>
requires
    std::same_as<std::ranges::range_value_t<Raw>, std::byte> &&
    std::has_unique_object_representations_v<T>
inline constexpr T sendynetorder(Raw host) noexcept {
    if constexpr(!is_host_le) {
        std::ranges::reverse(host);
    }

    if constexpr(std::convertible_to<Raw, std::array<std::byte, sizeof(T)>>) {
        return std::bit_cast<T>(host);
    } else {
        auto iter = host | std::views::take(sizeof(T));
        std::array<std::byte, sizeof(T)> array;
        std::copy(std::begin(iter), std::end(iter), array.begin());
        return std::bit_cast<T>(array);
    }
}

template<std::integral T>
requires std::has_unique_object_representations_v<T>
inline constexpr std::span<std::byte> consume(std::span<std::byte> bytes, T& out) {
    out = sendynetorder<T>(bytes);
    return bytes.subspan(sizeof(T));
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

    { encoder<T>::read(example_val<T&>, example_val<std::span<std::byte>>) } noexcept -> std::convertible_to<std::span<std::byte>>;
    { encoder<T>::write(v, example_val<std::vector<std::byte>&>) } noexcept;
};

template<std::integral T>
struct encoder<T> {
    static inline std::size_t encoded_sz() noexcept { return sizeof(T); }
    static constexpr std::span<std::byte> read(T& out, std::span<std::byte> buf) noexcept {
        return consume(buf, out);
    }

    static void write(T const& val, std::vector<std::byte>& buf) noexcept {
        auto array = sendynetorder<std::array<std::byte, sizeof(T)>>(bytes(val));
        buf.insert(buf.end(), std::begin(array), std::end(array));
    }
};

static_assert(encodable<int>);

}
