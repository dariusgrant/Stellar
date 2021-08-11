cmake_minimum_required(VERSION 3.5)
include(ExternalProject)

project(StellarRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Werror -fPIC")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories("include")

find_package(Vulkan 1.2 REQUIRED)
if(Vulkan_FOUND)
    message("Vulkan found.")
    include_directories(${Vulkan_INCLUDE_DIRS})
    link_libraries(${Vulkan_LIBRARIES})
endif(Vulkan_FOUND)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    X11Extras
)
if(Qt5_FOUND)
    MESSAGE("Qt5 Found.")
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
endif(Qt5_FOUND)

find_package(glm REQUIRED)
if(glm_FOUND)
    MESSAGE("GLM Found.")
    include_directories(${glm_INCLUDE_DIRS})
endif(glm_FOUND)

find_package(glfw3 3.3 REQUIRED)
if(glfw3_FOUND)
    MESSAGE("GLFW Found.")
    MESSAGE(STATUS ${glfw3_LIBRARIES})
    include_directories(${glfw3_INCLUDE_DIRS})
#    link_libraries(${glfw3_LIBRARIES})
endif(glfw3_FOUND)

find_package(spirv_cross_core)
find_package(spirv_cross_glsl REQUIRED)
find_package(spirv_cross_cpp REQUIRED)

include_directories(${SOURCE_DIR})

add_library(StellarRendererLib
    include/DGVulkan.hpp
    include/Timer.hpp src/Timer.cpp
    include/Window.hpp src/Window.cpp
    include/RendererCore.hpp src/RendererCore.cpp
    include/ExtensionMap.hpp
#    include/renderer.hpp src/renderer.cpp
#    include/vma_usage.hpp src/vma_usage.cpp
#    include/base.hpp src/base.cpp
#    include/resource.hpp src/resource.cpp
)

#add_library(SEL
#    include/context.hpp src/context.cpp
#)

#add_executable(StellarRenderer main.cpp)
add_executable(Triangle src/Triangle.cpp)
target_link_libraries(Triangle
    StellarRendererLib
    glfw
    #SEL
    Qt5::Core
    Qt5::Widgets
    Qt5::X11Extras
    spirv-cross-cpp
)

add_executable(RotatingCube src/RotatingCube.cpp)
target_link_libraries(RotatingCube
    StellarRendererLib
    glfw
    #SEL
    Qt5::Core
    Qt5::Widgets
    Qt5::X11Extras
    spirv-cross-cpp
    ${CMAKE_DL_LIBS}
)

#add_executable(Texture src/Texture.cpp)
