cmake_minimum_required(VERSION 3.25)
project(PulseVector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML and other dependencies
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
find_package(EnTT REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(libzip REQUIRED)

# --- Engine Target (Static Library) ---
add_library(pulse-engine STATIC)
target_include_directories(pulse-engine PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(pulse-engine PUBLIC 
    SFML::Graphics 
    SFML::Audio 
    EnTT::EnTT 
    fmt::fmt
)

# --- Game Target (Static Library) ---
add_library(pulse-game STATIC)
target_include_directories(pulse-game PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(pulse-game PUBLIC pulse-engine libzip::zip)

# --- Main Executable ---
add_executable(pulse-vector src/main.cpp)
target_link_libraries(pulse-vector PRIVATE pulse-game)

# --- Testing ---
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(pulse-tests ${TEST_SOURCES})
    target_link_libraries(pulse-tests PRIVATE 
        pulse-game 
        GTest::gtest_main
    )
    add_test(NAME AllTests COMMAND pulse-tests)
    
    # Symlink assets for tests too
    add_custom_command(TARGET pulse-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink 
            ${CMAKE_CURRENT_SOURCE_DIR}/assets 
            $<TARGET_FILE_DIR:pulse-tests>/assets
        COMMAND ${CMAKE_COMMAND} -E create_symlink 
            ${CMAKE_CURRENT_SOURCE_DIR}/levels 
            $<TARGET_FILE_DIR:pulse-tests>/levels
        COMMAND ${CMAKE_COMMAND} -E create_symlink 
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
            $<TARGET_FILE_DIR:pulse-tests>/shaders
        COMMENT "Creating symlinks for test assets"
    )
endif()

# Add source files (Globbing for brevity, but explicit is better in production)
file(GLOB_RECURSE ENGINE_SOURCES "src/engine/*.cpp")
file(GLOB_RECURSE GAME_SOURCES "src/game/*.cpp")
target_sources(pulse-engine PRIVATE ${ENGINE_SOURCES})
target_sources(pulse-game PRIVATE ${GAME_SOURCES})

# --- Asset Management ---
# Symlink assets and levels to the build directory for easy loading during development
add_custom_command(TARGET pulse-vector POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
        ${CMAKE_CURRENT_SOURCE_DIR}/assets 
        $<TARGET_FILE_DIR:pulse-vector>/assets
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
        ${CMAKE_CURRENT_SOURCE_DIR}/levels 
        $<TARGET_FILE_DIR:pulse-vector>/levels
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
        $<TARGET_FILE_DIR:pulse-vector>/shaders
    COMMENT "Creating symlinks for assets and levels"
)

# --- Compile Flags & Warnings ---
if(MSVC)
    target_compile_options(pulse-engine PRIVATE /W4 /WX)
else()
    target_compile_options(pulse-engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
