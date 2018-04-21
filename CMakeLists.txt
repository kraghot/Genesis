cmake_minimum_required(VERSION 3.0)
project(GlowApp)

# Executable and libraries to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undefined-var-template -Wno-error")

# Setup test exe
file(GLOB_RECURSE APP_SOURCES "src/*.cc" "src/*.hh")
add_executable(GlowApp ${APP_SOURCES})
target_include_directories(GlowApp PRIVATE "src")
target_compile_options(GlowApp PRIVATE -Wall)

# Add AntTweakBar lib
add_subdirectory(libs/AntTweakBar)
# Add aion lib
add_subdirectory(libs/aion)
# Add GLOW lib
add_subdirectory(libs/glow)
# Add assimp lib
add_subdirectory(libs/assimp-lean)
# Add glow-extras lib
add_subdirectory(libs/glow-extras)

# Add GLFW lib (with disabled spam)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(BUILD_SHARED_LIBS "" ON)
add_subdirectory(libs/glfw)

target_link_libraries(GlowApp PUBLIC
    glow
    glow-extras
    AntTweakBar
    assimp
    glfw
    fmt
    stdc++fs
    )

# Binary target
file(GLOB_RECURSE BIN_FILES "bin/*")
add_custom_target(bin SOURCES ${BIN_FILES})
