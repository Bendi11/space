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
#include <memory>
#include <span>
#include <ranges>
#include <type_traits>
#include <vector>

namespace sendy {

using std::pair;
using std::byte;
using std::array;

/** @brief If the host's processor uses little endian byte order */
constexpr bool is_host_le = (std::endian::native == std::endian::little);

/// Conver the given value of native byte ordering to sendy's network byte ordering,
/// or convert a value of sendy's byte ordering to native
template<typename T>
[[nodiscard]]
inline constexpr T sendynetorder(T host) noexcept {
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
    std::same_as<std::ranges::range_value_t<Raw>, byte> &&
    std::has_unique_object_representations_v<T>
[[nodiscard]]
inline constexpr T sendynetorder(Raw host) noexcept {
    if constexpr(!is_host_le) {
        std::ranges::reverse(host);
    }

    if constexpr(std::convertible_to<Raw, array<byte, sizeof(T)>>) {
        return std::bit_cast<T>(host);
    } else {
        auto iter = host | std::views::take(sizeof(T));
        array<byte, sizeof(T)> array;
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
[[nodiscard]]
inline constexpr array<byte, sizeof(T)> bytes(T val) noexcept {
    return std::bit_cast<array<byte, sizeof(T)>>(val);
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
            std::declval<std::span<byte>>()
        )
    } noexcept -> std::convertible_to<pair<T, std::size_t>>;
    {
        encoder<T>::write(
            v,
            std::declval<std::vector<byte>&>()
        )
    } noexcept;
};

/// Wrapper over `encoder<T>::read`
template<encodable T>
[[nodiscard]]
constexpr inline std::pair<T, std::size_t> decode(std::span<byte> buf) {
    return encoder<T>::read(buf);
}

/// Wrapper over `encoder<T>::write`
template<encodable T>
constexpr inline void encode(T const& val, std::vector<byte>& buf) noexcept {
    encoder<T>::write(val, buf);
}

/** @brief Wrapper over `encoder<T>::write` */
template<encodable T>
[[nodiscard("Encode with no buf argument creates a vector of bytes")]]
constexpr inline std::vector<byte> encode(T const& val) noexcept {
    std::vector<std::byte> vec{};
    vec.reserve(encoder<T>::encoded_sz(val));
    encoder<T>::write(val, vec);
    return vec;
}

/** @brief Wrapper over `encoder<T>::encoded_sz` */
template<encodable T>
[[nodiscard]]
constexpr inline std::size_t encoded_size(T const& val) noexcept {
    return encoder<T>::encoded_sz(val);
}

/**
 * @brief Encoder for all integral types `T` that converts them to and from sendy's byte ordering
 */
template<std::integral T>
struct encoder<T> {
    static inline std::size_t encoded_sz(int const&) noexcept { return sizeof(T); }
    static constexpr pair<T, size_t> read(std::span<std::byte> buf) noexcept {
        return pair(sendynetorder<T>(buf), sizeof(T));
    }

    static void write(T const& val, std::vector<std::byte>& buf) noexcept {
        auto array = sendynetorder(bytes(val));
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
        std::size_t size = encoded_size<len_t>(vec.size());
        for(auto const& val : vec) {
            size += encoded_size<T>(val);
        }
        return size;
    }

    static constexpr pair<std::vector<T>, std::size_t> read(std::span<byte> buf) noexcept {
        std::size_t read = 0;
        std::size_t size = 0;

        len_t len;
        std::tie(len, read) = decode<len_t>(buf);
        buf = buf.subspan(read);
        size += read;

        std::vector<T> vec{};
        vec.reserve(len);
        
        for(len_t i = 0; i < len; ++i) {
            T val;
            std::tie(val, read) = decode<T>(buf);
            buf = buf.subspan(read);
            size += read;

            vec.push_back(val);
        }

        return pair(vec, size);
    }

    static constexpr void write(std::vector<T> const& val, std::vector<byte>& buf) noexcept {
        len_t len = val.size();
        encode(len, buf);
        for(auto const& element : val) {
            encode(element, buf);
        }
    }
};

/**
 * Encoder specialization that encodes an std::string as a length and
 * a sequence of ASCII characters
 */
template<>
struct encoder<std::string> {
    using len_t = std::uint32_t;

    static inline constexpr std::size_t encoded_sz(std::string const& val) noexcept {
        return encoded_size<len_t>(val.size()) + val.size();
    }
    static pair<std::string, std::size_t> read(std::span<byte> span) noexcept;
    static void write(std::string const& val, std::vector<byte>& buf) noexcept;
};

/**
 * Encoder specialization that can convert scoped enumerations to their underlying representation before encoding
 */
template<typename T>
requires std::is_enum_v<T>
struct encoder<T> {
    using underlying = typename std::underlying_type<T>::type;

    static inline constexpr std::size_t encoded_sz(T const& val) noexcept { return sizeof(underlying); }
    static pair<T, std::size_t> read(std::span<byte> span) noexcept {
        auto [val, read] = decode<underlying>(span);
        return pair(T{val}, read);
    }
    static void write(T const& val, std::vector<byte>& buf) noexcept {
        encode<underlying>(static_cast<underlying>(val), buf);
    }
};


/** @brief Concept for types that can be easily encoded using a single `encode` template */
template<typename T>
concept easyencodable = requires(T& ref, T const& cref) {
    {
        ref.encode([](auto&...){})
    };
};

/**
 * @brief Encoder implementation for types that satisfy the `easyencodable` concept
 */
template<easyencodable T>
struct encoder<T> {
    static inline constexpr std::size_t encoded_sz(T const& val) noexcept {
        const auto sizes = [](auto const&... args) { return (... + encoded_size<std::decay_t<decltype(args)>>(args)); };
        return const_cast<T&>(val).encode(sizes);
    }

    static inline pair<T, std::size_t> read(std::span<byte> span) noexcept {
        std::size_t size = 0;

        static const auto parser = [&](auto&... args) {(
            (
                [&] {
                    auto [val, read] = decode<std::decay_t<decltype(args)>>(span);
                    size += read;
                    span = span.subspan(read);
                    (*(std::addressof(args))) = val;
                }
            )(), ...
        );};

        T val = std::bit_cast<T>(std::array<byte, sizeof(T)>());
        val.encode(parser);
        return pair(val, size);
    }

    static inline void write(T const& val, std::vector<byte>& buf) noexcept {
        static const auto writer = [&](auto&... args) {
            (
                (encode(args, buf)), ...
            );
        };

        const_cast<T&>(val).encode(writer);
    }
};

}
