cmake_minimum_required(VERSION 3.5)

project(StellarRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If using vcpkg, set the argument -DCMAKE_TOOLCHAIN_FILE to
# <PathToInstall>/vcpkg/scripts/buildsystems/vcpkg.cmake

if(CMAKE_TOOLCHAIN_FILE STREQUAL "")
    MESSAGE("No toolchain file.")
endif()

include_directories("include")

find_package(Vulkan 1.2 REQUIRED)

find_package(glm REQUIRED)

find_package(glfw3 3.3 REQUIRED)

add_library(StellarRendererLib
    include/Timer.hpp src/Timer.cpp
    include/Window.hpp src/Window.cpp
    include/RendererCore.hpp src/RendererCore.cpp
    include/ExtensionMap.hpp
)

target_link_libraries(StellarRendererLib
    Vulkan::Vulkan
    glfw
)

add_executable(RotatingCube src/RotatingCube.cpp)
target_include_directories(RotatingCube PRIVATE glm)
target_link_libraries(RotatingCube
    StellarRendererLib
)
