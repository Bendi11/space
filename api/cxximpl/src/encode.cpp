#include "sendy/encode.hpp"

namespace sendy {

using len_t = std::uint32_t;

pair<std::string, std::size_t> encoder<std::string>::read(std::span<byte> span) noexcept {
    std::size_t size = 0;
    auto [len, read] = decode<len_t>(span);
    span = span.subspan(read);
    size += read + len;

    return pair(std::string(reinterpret_cast<const char*>(span.data()), len), size);
}

void encoder<std::string>::write(std::string const& val, std::vector<byte>& buf) noexcept {
    encode<len_t>(val.size(), buf);
    auto data = reinterpret_cast<const byte*>(val.data());
    std::copy(data, data + val.size(), std::back_inserter(buf));
}

}
