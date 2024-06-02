file(GLOB_RECURSE redump-common-headers *.hpp)
file(GLOB_RECURSE redump-common-sources *.cpp)

add_library(redump-common OBJECT
    ${redump-common-headers}
    ${redump-common-sources}
)

target_include_directories(redump-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(redump-common PUBLIC
    CLI11::CLI11
    range-v3::meta
    range-v3::concepts
    range-v3::range-v3
    spdlog::spdlog
    pugixml::pugixml
    PkgConfig::cryptopp
)
