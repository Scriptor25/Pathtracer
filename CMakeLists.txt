cmake_minimum_required(VERSION 3.25)

project(PathTracer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(glew-cmake_BUILD_SHARED OFF)
set(glew-cmake_BUILD_STATIC ON)
set(ONLY_LIBS ON)
add_subdirectory(deps/glew)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(deps/glfw)

add_subdirectory(deps/glm)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_INSTALL_PDB OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
add_subdirectory(deps/assimp)

add_subdirectory(deps/yaml)

add_library(imgui STATIC
        deps/imgui/imgui.cpp
        deps/imgui/imgui_demo.cpp
        deps/imgui/imgui_draw.cpp
        deps/imgui/imgui_tables.cpp
        deps/imgui/imgui_widgets.cpp
        deps/imgui/misc/cpp/imgui_stdlib.cpp
        deps/imgui/backends/imgui_impl_glfw.cpp
        deps/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC deps/imgui)
target_link_libraries(imgui PUBLIC glfw)

file(GLOB_RECURSE src src/*.cpp include/*.hpp)
add_executable(path_tracer ${src})
target_include_directories(path_tracer PRIVATE include)
target_link_libraries(path_tracer PRIVATE libglew_static glfw glm::glm assimp yaml-cpp::yaml-cpp imgui)

install(TARGETS path_tracer)
install(DIRECTORY assets DESTINATION bin)
