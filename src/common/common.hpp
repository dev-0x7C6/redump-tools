#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace redump {
struct rom {
    std::string filename;
    std::uint64_t size{};
    std::string crc;
    std::string md5;
    std::string sha1;
};

struct game {
    std::string name;
    std::string category;
    std::string description;
    std::vector<rom> roms;
};

using games = std::vector<game>;

auto load(const std::filesystem::path &path) -> std::optional<games>;

} // namespace redump
