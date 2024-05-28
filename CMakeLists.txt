
cmake_minimum_required(VERSION 3.8)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(LBOD VERSION 1.0
    DESCRIPTION "Legacy Bios Operating system Development language"
    LANGUAGES C)

add_executable(lbodc src/main.c src/lex.c src/parse.c src/compile.c src/inc/main.h)
target_compile_features(lbodc PRIVATE c_std_90)
target_link_libraries(lbodc)
add_test(NAME lbodc COMMAND lbodc test/test.lb -o test/hello.asm)
