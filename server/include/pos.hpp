#pragma once


#include <concepts>
#include <cstdint>
#include <limits>
#include <compare>

#define WRAP(ty) \
    inline constexpr ty operator+(ty const& other) const noexcept { return ty(_v + other._v); } \
    inline constexpr ty operator-(ty const& other) const noexcept { return ty(_v - other._v); } \
    inline constexpr ty operator*(ty const& other) const noexcept { return ty(_v * other._v); } \
    inline constexpr ty operator/(ty const& other) const noexcept { return ty(_v / other._v); } \
    inline constexpr ty operator%(ty const& other) const noexcept { return ty(_v / other._v); } \
    inline constexpr auto operator<=>(ty const& other) const noexcept { return _v <=> other._v; };          \
    inline constexpr ty& operator+=(ty const& other) noexcept { return *this = *this + other; }     \
    inline constexpr ty& operator-=(ty const& other) noexcept { return *this = *this - other; }     \
    inline constexpr ty& operator*=(ty const& other) noexcept { return *this = *this * other; }     \
    inline constexpr ty& operator/=(ty const& other) noexcept { return *this = *this / other; }     \
    inline constexpr ty& operator%=(ty const& other) noexcept { return *this = *this % other; }


struct millimeter {
public:
    WRAP(millimeter)

    constexpr inline millimeter(std::uint64_t v) noexcept : _v(v) {}

    constexpr inline bool chkadd(millimeter& result, millimeter other) const noexcept {
        result = *this + other;
        return result < other;
    }

private:
    std::uint64_t _v;
};

struct gridsquare {
public:
    WRAP(gridsquare)
    constexpr inline gridsquare(std::uint64_t v) noexcept : _v(v) {}
private:
    std::uint64_t _v;
};

#undef WRAP


/**
 * @brief Millimeter-accurate position in the universe, used only to track interstellar entities
 */
struct universe_pos {
    using distance = std::uint64_t;
    using limits = std::numeric_limits<distance>;

    inline constexpr universe_pos(gridsquare grid, millimeter mm) : _grid(grid), _mm(mm) {}
    
    [[nodiscard]]
    static inline constexpr universe_pos middle() noexcept { return universe_pos(limits::max() / 2, limits::max() / 2); }

    inline constexpr universe_pos operator+(universe_pos const& other) const noexcept {
        universe_pos tmp{_grid + other._grid, 0};
        if(_mm.chkadd(tmp._mm, other._mm)) {
            tmp._grid += 1;
        }
        return tmp;
    }

private:
    /// 1 ~= 18.4467 petamters
    gridsquare _grid;
    /// 1 = 1 millimeter
    millimeter _mm;
};
