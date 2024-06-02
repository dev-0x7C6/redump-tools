cmake_minimum_required(VERSION 3.20...3.26)

project(
  redump-tools
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

find_package(CLI11 REQUIRED)
find_package(pugixml REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(cryptopp REQUIRED IMPORTED_TARGET libcryptopp)

add_subdirectory("src")
