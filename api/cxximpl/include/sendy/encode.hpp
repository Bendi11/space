#pragma once

#include <array>
#include <bits/ranges_base.h>
#include <codecvt>
#include <concepts>
#include <bit>
#include <algorithm>
#include <cstddef>
#include <cstdint>
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

/** @brief If the host's processor uses little endian byte order */
constexpr bool is_host_le = (std::endian::native == std::endian::little);

/// Conver the given value of native byte ordering to sendy's network byte ordering,
/// or convert a value of sendy's byte ordering to native
template<std::integral T>
inline constexpr T sendynetorder(T host) noexcept {
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

/**
 * @brief Wrapper around `std::bit_cast` providing easier access to an array of bytes representing the given
 * integral value
 */
template<std::integral T>
requires std::has_unique_object_representations_v<T>
inline constexpr std::array<std::byte, sizeof(T)> bytes(T val) noexcept {
    return std::bit_cast<std::array<std::byte, sizeof(T)>>(val);
}

namespace _impl {
    template<typename T>
    constexpr T example_val = *(T*)(nullptr);
}

/** @brief Type to be specialized for a given type `T`, providing methods to encode and decode values */
template<typename T>
struct encoder;

/**
 * @brief Requires a valid specialization of the `encoder` type to exist for the given type
 * `T`
 */
template<typename T>
concept encodable = requires(T const& v) {
    { encoder<T>::encoded_sz(v) } noexcept -> std::convertible_to<std::size_t>;

    {
        encoder<T>::read(
            _impl::example_val<std::span<std::byte>>
        )
    } noexcept -> std::convertible_to<T>;
    {
        encoder<T>::write(
            v,
            _impl::example_val<std::vector<std::byte>&>
        )
    } noexcept;
};

/// Wrapper over `encoder<T>::read`
template<encodable T>
constexpr inline T decode(std::span<std::byte> buf) {
    return encoder<T>::read(buf);
}

/// Wrapper over `encoder<T>::write`
template<typename T>
constexpr inline void encode(T const& val, std::vector<std::byte>& buf) {
    encoder<T>::write(val, buf);
}

/**
 * @brief Encoder for all integral types `T` that converts them to and from sendy's byte ordering
 */
template<std::integral T>
struct encoder<T> {
    static inline std::size_t encoded_sz(int const&) noexcept { return sizeof(T); }
    static constexpr T read(std::span<std::byte> buf) noexcept {
        return sendynetorder<T>(buf);
    }

    static void write(T const& val, std::vector<std::byte>& buf) noexcept {
        auto array = sendynetorder<std::array<std::byte, sizeof(T)>>(bytes(val));
        buf.insert(buf.end(), std::begin(array), std::end(array));
    }
};

/**
 * @brief Encoder for a dynamic list of values
 */
template<encodable T>
struct encoder<std::vector<T>> {
    using len_t = std::uint32_t;

    static inline constexpr std::size_t encoded_sz(std::vector<T> const& vec) noexcept {
        return sizeof(len_t) + encoder<T>::encoded_sz() * vec.size();
    }

    static constexpr T read(std::span<std::byte> buf) noexcept {
        len_t len = decode<len_t>(buf);
        buf = buf.subspan(encoder<len_t>::encoded_sz(len));
        std::vector<T> vec{};
        vec.reserve(len);
        
        for(len_t i = 0; i < len; ++i) {
            
        }
    }
};

}
