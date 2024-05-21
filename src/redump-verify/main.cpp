
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>
#include <pugixml.hpp>
#include <range/v3/algorithm/sort.hpp>

using namespace std::literals;

namespace redump {
struct content {
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
    std::vector<content> roms;
};

template <typename T = std::string>
auto print(const redump::content &content, T prefix = {}) {
    fmt::print("{}filename : {}\n", prefix, content.filename);
    fmt::print("{}    size : {} bytes\n", prefix, content.size);
    fmt::print("{}     crc : {}\n", prefix, content.crc);
    fmt::print("{}     md5 : {}\n", prefix, content.md5);
    fmt::print("{}    sha1 : {}\n", prefix, content.sha1);
}

template <typename T = std::string>
auto print(const std::vector<redump::content> &content, T prefix = {}) {
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

namespace xml {
auto attr(const pugi::xml_node &node, const char *name) {
    return node.attribute(name).value();
}
} // namespace xml

auto main(int argc, char **argv) -> int {
    pugi::xml_document input;
    auto ret = input.load_file("psx.xml");
    if (!ret) return 1;

    std::vector<redump::game> games;

    for (auto &&node : input.document_element()) {
        if (node.name() != "game"sv) continue;

        redump::game game{
            .name = xml::attr(node, "name"),
            .category = node.child_value("category"),
            .description = node.child_value("description"),
        };

        for (auto &&s : node) {
            if (s.name() != "rom"sv) continue;

            game.roms.emplace_back( //
                redump::content{
                    .filename = xml::attr(s, "name"),
                    .size = std::stoull(xml::attr(s, "size")),
                    .crc = xml::attr(s, "crc"),
                    .md5 = xml::attr(s, "md5"),
                    .sha1 = xml::attr(s, "sha1"),
                });
        }

        games.emplace_back(std::move(game));
    }

    ranges::sort(games, [](const redump::game &lhs, const redump::game &rhs) -> bool {
        return lhs.name < rhs.name;
    });

    print(games);
    return 0;
}
