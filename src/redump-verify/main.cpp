
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <CLI/CLI.hpp>
#include <cryptopp/sha.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <pugixml.hpp>
#include <range/v3/algorithm/sort.hpp>

using namespace std::literals;

namespace xml {
auto attr(const pugi::xml_node &node, const char *name) {
    return node.attribute(name).value();
}
} // namespace xml

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

auto load(const std::filesystem::path &path) -> std::optional<std::vector<redump::game>> {
    pugi::xml_document xml;
    if (!xml.load_file(path.c_str())) return {};

    std::vector<redump::game> ret;

    for (auto &&node : xml.document_element()) {
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

        ret.emplace_back(std::move(game));
    }

    return ret;
}

} // namespace redump

#include <fcntl.h>

namespace raii {
struct open {
    template <typename... Ts>
    open(Ts &&...args) noexcept
            : descriptor(::open(std::forward<Ts>(args)...)) {}

    ~open() noexcept {
        if (descriptor != -1)
            ::close(descriptor);
    }

    operator auto() const noexcept {
        return descriptor;
    }

private:
    const int descriptor{};
};
} // namespace raii

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

auto main(int argc, char **argv) -> int {
    CLI::App app("redump-verify");

    std::vector<std::string> paths;
    std::vector<std::string> files;

    app.add_option("-i,--input", paths, "xml redump database") //
        ->required()
        ->allow_extra_args()
        ->check(CLI::ExistingFile);

    app.add_option("-v,--verify", files, "files to verify") //
        ->required()
        ->allow_extra_args()
        ->check(CLI::ExistingFile);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return 1;
    }

    std::vector<redump::game> games;

    for (auto &&path : paths) {
        auto db = redump::load(path).value_or(std::vector<redump::game>{});
        std::move(db.begin(), db.end(), std::back_inserter(games));
    }

    ranges::sort(games, [](const redump::game &lhs, const redump::game &rhs) -> bool {
        return lhs.name < rhs.name;
    });

    std::map<std::string, redump::game> sha1_to_game;

    for (auto &&game : games)
        for (auto &&rom : game.roms)
            sha1_to_game[rom.sha1] = game;

    auto is_valid{true};

    for (auto file : files) {
        const auto hash = sha1(file);
        const auto valid = sha1_to_game.contains(hash);
        if (valid)
            fmt::print("{}: {} ({})\n", file, "ok", sha1_to_game[hash].name);
        else
            fmt::print("{}: {}\n", file, "nok");
        is_valid &= valid;
    }

    return is_valid ? 0 : 2;
}
