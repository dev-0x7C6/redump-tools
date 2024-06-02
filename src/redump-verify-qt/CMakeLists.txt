file(GLOB_RECURSE redump-verify-sources *.cpp)

find_package(Qt6 COMPONENTS Widgets Core)

add_executable(redump-verify-qt ${redump-verify-sources})
target_link_libraries(redump-verify-qt PUBLIC redump-common Qt6::Widgets Qt6::Core)
install(TARGETS redump-verify-qt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
