#include <catch2/catch_test_macros.hpp>

#include "sendy/encode.hpp"

#define __SENDY_REVERSE_ENDIANNESS

TEST_CASE("Test network encoding of integral values (big endian)", "[encoding]") {
    if constexpr(std::endian::native == std::endian::little) {
        REQUIRE(sendy::sendynetorder<int>(0x20000000) == 32);
        auto buf = sendy::bytes(32);
        int val;
        sendy::encoder<int>::read(val, buf);

        REQUIRE(val == sendy::revbytes(32));
    }
}
