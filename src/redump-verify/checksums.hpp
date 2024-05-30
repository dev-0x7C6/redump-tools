#pragma once

#include <cryptopp/sha.h>
#include <filesystem>
#include <fmt/format.h>

#include "raii.hpp"

namespace checksum {

auto sha1(const std::filesystem::path &path) -> std::string {
    raii::open fd(path.c_str(), O_RDONLY);
    if (!fd) return {};

    CryptoPP::SHA1 processor;
    std::array<CryptoPP::byte, 4096> buffer{};

    ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    ::lseek64(fd, 0, SEEK_SET);

    for (;;) {
        const auto size = ::read(fd, buffer.data(), buffer.size());
        if (size == 0) break;
        if (size <= 0) return {};

        processor.Update(buffer.data(), size);
    }

    std::array<std::uint8_t, CryptoPP::SHA1::DIGESTSIZE> digest;
    processor.Final(digest.data());

    return fmt::format("{:02x}", fmt::join(digest, {}));
}

} // namespace checksum
