#pragma once

#include <filesystem>
#include <pwd.h>
#include <unistd.h>

auto get_home_dir() -> std::filesystem::path {
    return getpwuid(getuid())->pw_dir;
}
