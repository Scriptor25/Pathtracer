cmake_minimum_required(VERSION 3.28)

project(Pathtracer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG master
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG latest
)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
)
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v5.4.1
)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0
)

set(glew-cmake_BUILD_SHARED OFF)
set(glew-cmake_BUILD_STATIC ON)
set(ONLY_LIBS ON)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)

FetchContent_MakeAvailable(glew glfw glm assimp yaml-cpp)

file(GLOB_RECURSE src src/*.cpp include/*.hpp)
add_executable(Pathtracer ${src})
target_include_directories(Pathtracer PRIVATE include)
target_link_libraries(Pathtracer PRIVATE libglew_static glfw glm::glm assimp yaml-cpp::yaml-cpp)

install(TARGETS Pathtracer)
install(DIRECTORY assets DESTINATION bin)