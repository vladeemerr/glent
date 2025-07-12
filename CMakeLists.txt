cmake_minimum_required(VERSION 3.20)
include(FetchContent)

project(glint LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
	source/glint.cpp
	source/input.cpp
	source/graphics_gl.cpp
	source/graphics_utils.cpp
	source/graphics.cpp
)

if(LINUX)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
elseif(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4)
	target_link_options(${PROJECT_NAME} PRIVATE /subsystem:windows /entry:mainCRTStartup)
else()
	message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED TRUE CXX_STANDARD 20)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)

FetchContent_Declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm
)

FetchContent_Declare(
	lodepng
	GIT_REPOSITORY https://github.com/lvandeve/lodepng
)

set(GLFW_LIBRARY_TYPE STATIC)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

set(GLM_BUILD_TESTS OFF)

FetchContent_MakeAvailable(glfw glm lodepng)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)
target_sources(${PROJECT_NAME} PRIVATE
	external/glad/src/gles2.c
	${lodepng_SOURCE_DIR}/lodepng.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE
	external/glad/include
	${lodepng_SOURCE_DIR}
)
