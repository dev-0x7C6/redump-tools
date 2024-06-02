#pragma once

#include <cryptopp/sha.h>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>

#include "raii.hpp"

namespace checksum {

template <typename HashAlgorithm>
auto compute(const std::filesystem::path &path, std::atomic<double> &progress) -> std::string {
    progress = 0.0;
    raii::open fd(path.c_str(), O_RDONLY);
    if (!fd) return {};

    HashAlgorithm processor;
    std::array<CryptoPP::byte, 65536> buffer{};

    ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    std::size_t total = ::lseek64(fd, 0, SEEK_END);
    std::size_t readed{};
    ::lseek64(fd, 0, SEEK_SET);

    for (;;) {
        const auto size = ::read(fd, buffer.data(), buffer.size());
        if (size == 0) break;
        if (size <= 0) return {};

        processor.Update(buffer.data(), size);
        readed += size;
        progress = static_cast<double>(readed) / static_cast<double>(total);
    }

    std::array<std::uint8_t, HashAlgorithm::DIGESTSIZE> digest;
    processor.Final(digest.data());
    progress = 1.0;

    return fmt::format("{:02x}", fmt::join(digest, {}));
}

} // namespace checksum
