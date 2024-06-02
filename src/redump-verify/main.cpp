
#include <algorithm>

#include <cryptopp/sha.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <range/v3/algorithm/sort.hpp>

#include "checksums.hpp"
#include "common.hpp"

auto main(int argc, char **argv) -> int {
    CLI::App app("redump-verify");

    std::vector<std::string> paths;
    std::vector<std::string> files;

    app.add_option("-i,--input", paths, "xml redump database") //
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

    const auto pw = getpwuid(getuid());
    std::filesystem::path home_dir(pw->pw_dir);
    std::filesystem::path db_dir(home_dir / ".cache" / "redump" / "db");

    std::filesystem::create_directories(db_dir);

    for (auto &&entry : std::filesystem::directory_iterator{db_dir})
        if (entry.is_regular_file())
            paths.emplace_back(entry.path());

    redump::games games;

    for (auto &&path : paths) {
        fmt::print("loading {}...\n", path);
        auto db = redump::load(path).value_or(std::vector<redump::game>{});
        std::move(db.begin(), db.end(), std::back_inserter(games));
    }

    // ranges::sort(games, [](const redump::game &lhs, const redump::game &rhs) -> bool {
    //     return lhs.name < rhs.name;
    // });

    std::map<std::string, redump::game> sha1_to_game;

    for (auto &&game : games)
        for (auto &&rom : game.roms)
            sha1_to_game[rom.sha1] = game;

    auto is_valid{true};

    std::atomic<double> progress;
    for (auto file : files) {
        const auto hash = checksum::compute<CryptoPP::SHA1>(file, progress);
        const auto valid = sha1_to_game.contains(hash);
        if (valid)
            fmt::print("{}: {} ({})\n", file, "ok", sha1_to_game[hash].name);
        else
            fmt::print("{}: {}\n", file, "nok");
        is_valid &= valid;
    }

    return is_valid ? 0 : 2;
}
