#include <catch2/catch_test_macros.hpp>

#include "sendy/encode.hpp"


TEST_CASE("Test network encoding of integral values (simulated non-native byte ordering)", "[encoding]") {
    auto long_buf = sendy::bytes(sendy::sendynetorder(145ULL));
    auto span = std::span(long_buf);
    REQUIRE(sendy::decode<unsigned long long>(span) == 145ULL);
}
