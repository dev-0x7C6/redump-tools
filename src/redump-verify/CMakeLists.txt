file(GLOB_RECURSE redump-verify-sources *.cpp)

add_executable(redump-verify ${redump-verify-sources})

target_link_libraries(redump-verify PUBLIC redump-common)
install(TARGETS redump-verify RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
