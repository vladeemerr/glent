cmake_minimum_required(VERSION 3.5)

project(glent LANGUAGES CXX)

add_subdirectory(source)

target_include_directories(${PROJECT_NAME} PRIVATE vendor/include)

target_include_directories(${PROJECT_NAME} PRIVATE vendor/stb)

set(GLFW_LIBRARY_TYPE STATIC)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(vendor/glfw)

add_subdirectory(vendor/glm)
