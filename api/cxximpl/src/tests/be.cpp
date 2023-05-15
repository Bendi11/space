#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "sendy/encode.hpp"


TEST_CASE("encoding and decoding", "[encoding]") {
    SECTION("encoding integral values") {
        auto long_buf = sendy::encode(145ULL);
        auto span = std::span(long_buf);
        REQUIRE(std::get<0>(sendy::decode<unsigned long long>(span)) == 145ULL);
    }
    
    SECTION("encoding variable-size structures") {
        std::vector<int> o_ints = { 3, 2, 1, 2 };
        auto buf = sendy::encode(o_ints);
        std::vector<long> o_longs = { 20000, 2131 };
        sendy::encode(o_longs, buf);
        REQUIRE(buf.size() == 20 + 20);

        auto span = std::span(buf);
        auto [decoded_ints, read] = sendy::decode<std::vector<int>>(span);
        span = span.subspan(read);

        std::vector<long> decoded_longs;
        std::tie(decoded_longs, read) = sendy::decode<std::vector<long>>(span);
        REQUIRE(decoded_ints == o_ints);
        REQUIRE(decoded_longs == o_longs);
    }
}
