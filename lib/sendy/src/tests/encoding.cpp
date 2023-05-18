#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <iostream>
#include <limits>

#include "sendy.hpp"
#include "sendy/encode.hpp"

using namespace sendy;

#define _STR(x) #x
#define STR(x) _STR(x)

#define TEST_ENCODE(type, desc, ...) do {                           \
    SECTION("[" #type "] - " desc, "[encoding]") {                  \
        using _ = type;                                             \
        auto val = GENERATE(__VA_ARGS__);                           \
        auto buf = encode<type>(val);                               \
        CHECK(                                                      \
            std::get<0>(decode<type>(std::span(buf))) == val        \
        );                                                          \
    }                                                               \
} while(0)

#define AUTO_LIMITS_INTEGRAL_TEST(type) TEST_ENCODE(                \
    type, "limits",                                                 \
    as<type>{},                                                     \
    std::numeric_limits<type>::min(),                               \
    std::numeric_limits<type>::max() / (type)2,                     \
    std::numeric_limits<type>::max()                                \
)

#define AUTO_RAND_INTEGRAL_TEST(type)                               \
    TEST_ENCODE(                                                    \
        type,                                                       \
        "random",                                                   \
        take(                                                       \
            25,                                                     \
            random(                                                 \
                std::numeric_limits<type>::min(),                   \
                std::numeric_limits<type>::max()                    \
            )                                                       \
        )                                                           \
    )

#define AUTO_INTEGRAL_TEST(type) AUTO_LIMITS_INTEGRAL_TEST(type); AUTO_RAND_INTEGRAL_TEST(type)

TEST_CASE("encoding and decoding", "[encoding]") {
    AUTO_LIMITS_INTEGRAL_TEST(std::uint8_t);
    AUTO_INTEGRAL_TEST(std::uint16_t);
    AUTO_INTEGRAL_TEST(std::uint32_t);
    AUTO_INTEGRAL_TEST(std::uint64_t);

    AUTO_LIMITS_INTEGRAL_TEST(std::int8_t);
    AUTO_INTEGRAL_TEST(std::int16_t);
    AUTO_INTEGRAL_TEST(std::int32_t);
    AUTO_INTEGRAL_TEST(std::int64_t);

    TEST_ENCODE(
        std::vector<std::uint32_t>,
        "vector",
        _{1, 2, 3},
        _{4, 5, 6, 7},
        _{}
    );

    TEST_ENCODE(
        std::string,
        "string",
        as<std::string>{},
        _{},
        "ABC\nDEF"
    );
}
