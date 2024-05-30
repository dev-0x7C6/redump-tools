file(GLOB_RECURSE redump-verify-headers *.hpp)
file(GLOB_RECURSE redump-verify-sources *.cpp)

add_executable(redump-verify
    ${redump-verify-headers}
    ${redump-verify-sources}
)

pkg_check_modules(cryptopp REQUIRED IMPORTED_TARGET libcryptopp)

target_link_libraries(redump-verify
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    pugixml::pugixml
    PkgConfig::cryptopp
)

install(TARGETS redump-verify RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
