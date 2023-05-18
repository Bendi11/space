#pragma once


#include <concepts>
#include <cstdint>
#include <limits>
#include <compare>

namespace _impl {

template<std::integral T>
struct wrapper {
    using self = wrapper<T>;

    explicit inline constexpr wrapper(T v) noexcept : _v{v} {}
    explicit inline constexpr operator T() noexcept { return _v; }
    
    inline constexpr self operator+(self const& other) const noexcept { return self(_v + other._v); };
    inline constexpr self operator-(self const& other) const noexcept { return self(_v - other._v); }
    inline constexpr self operator*(self const& other) const noexcept { return self(_v * other._v); }
    inline constexpr self operator/(self const& other) const noexcept { return self(_v / other._v); }
    inline constexpr self operator%(self const& other) const noexcept { return self(_v / other._v); }
    inline constexpr auto operator<=>(self const& other) const noexcept = default;

protected:
    T _v;
};


}

struct millimeter : _impl::wrapper<std::uint64_t> {
    constexpr inline millimeter(std::uint64_t v) noexcept : _impl::wrapper<std::uint64_t>(v) {}
};


/**
 * @brief Millimeter-accurate position in the universe, used only to track interstellar entities
 */
struct universe_pos {
    using distance = std::uint64_t;
    using limits = std::numeric_limits<distance>;

    inline constexpr universe_pos(distance grid, distance mm) : _grid(grid), _mm(mm) {}
    
    [[nodiscard]]
    static inline constexpr universe_pos middle() noexcept { return universe_pos(limits::max() / 2, limits::max() / 2); }

    inline constexpr universe_pos operator+(universe_pos const& other) const noexcept {

    }

private:
    /// 1 ~= 18.4467 petamters
    distance _grid;
    /// 1 = 1 millimeter
    millimeter _mm;
};
