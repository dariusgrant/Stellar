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
include_directories(CMAKE_PREFIX_PATH)

find_package(Vulkan 1.2 REQUIRED)

find_package(glm REQUIRED)

find_package(glfw3 3.3 REQUIRED)

#add_library(stellar
#    include/Timer.hpp src/Timer.cpp
#    include/Window.hpp src/Window.cpp
#    include/RendererCore.hpp src/RendererCore.cpp
#    include/ExtensionMap.hpp
#    include/DGVulkan.hpp
#    include/Utils.hpp
#)

#target_link_libraries(StellarRendererLib
#    Vulkan::Vulkan
#    glfw
#)

add_executable(Triangle src/Triangle.cpp)
target_include_directories(Triangle PRIVATE glm)
target_link_libraries(Triangle
    Vulkan::Vulkan
    glfw
)

add_executable(Texture src/Texture.cpp)
target_include_directories(Texture PRIVATE glm)
target_link_libraries(Texture
    Vulkan::Vulkan
    glfw
)

#add_executable(RotatingCube src/RotatingCube.cpp)
#target_link_libraries(RotatingCube
#    Vulkan::Vulkan
#    glfw
#    stellar
#)
