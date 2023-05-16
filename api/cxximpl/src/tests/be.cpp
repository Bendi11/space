#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "sendy.hpp"
#include "sendy/encode.hpp"

using namespace sendy;

TEST_CASE("encoding and decoding", "[encoding]") {
    SECTION("encoding integral values") {
        auto long_buf = encode(145ULL);
        auto span = std::span(long_buf);
        REQUIRE(std::get<0>(decode<unsigned long long>(span)) == 145ULL);
    }
    
    SECTION("encoding variable-size structures") {
        std::vector<int> o_ints = { 3, 2, 1, 2 };
        auto buf = encode(o_ints);
        std::vector<long> o_longs = { 20000, 2131 };
        encode(o_longs, buf);
        REQUIRE(buf.size() == 20 + 20);

        auto span = std::span(buf);
        auto [decoded_ints, read] = decode<std::vector<int>>(span);
        span = span.subspan(read);

        std::vector<long> decoded_longs;
        std::tie(decoded_longs, read) = decode<std::vector<long>>(span);
        REQUIRE(decoded_ints == o_ints);
        REQUIRE(decoded_longs == o_longs);

        std::string str = "hello, world!";
        buf = encode(str);
        REQUIRE(decode<std::string>(buf).first == str);
    }

    SECTION("encoding easyencodable structs") {
        packet_header header;
        header.id = 3;
        header.kind = packet_kind::Connect;

        auto buf = encode(header);
        auto decoded = decode<packet_header>(buf);
    }
}
