#pragma once

#include "common.hpp"

#include <fmt/printf.h>
#include <string>

namespace redump {
template <typename T = std::string>
auto print(const redump::rom &rom, T prefix = {}) {
    fmt::print("{}filename : {}\n", prefix, rom.filename);
    fmt::print("{}    size : {} bytes\n", prefix, rom.size);
    fmt::print("{}     crc : {}\n", prefix, rom.crc);
    fmt::print("{}     md5 : {}\n", prefix, rom.md5);
    fmt::print("{}    sha1 : {}\n", prefix, rom.sha1);
}

template <typename T = std::string>
auto print(const std::vector<redump::rom> &content, T prefix = {}) {
    for (auto &&c : content) {
        print(c, prefix);
        fmt::print("\n");
    }
}

template <typename T = std::string>
auto print(const redump::game &game, T prefix = {}) {
    fmt::print("{}{}\n", prefix, game.name);
    fmt::print("{}    category : {}\n", prefix, game.category);
    fmt::print("{} description : {}\n\n", prefix, game.description);
    print(game.roms, "    ");
}

template <typename T = std::string>
auto print(const std::vector<redump::game> &games, T prefix = {}) {
    for (auto &&game : games) {
        print(game, prefix);
        fmt::print("\n");
    }
}
} // namespace redump
