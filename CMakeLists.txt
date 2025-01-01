cmake_minimum_required(VERSION 3.11)

project(glent LANGUAGES CXX)

add_executable(${PROJECT_NAME} source/glent.cpp)

if (LINUX)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -fno-diagnostics-show-caret -fno-rtti)
elseif (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /GR-)
	target_link_options(${PROJECT_NAME} PRIVATE /subsystem:windows /entry:mainCRTStartup)
else()
	message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD_REQUIRED True
	CXX_STANDARD 17
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE vendor/include)

set(GLFW_LIBRARY_TYPE STATIC)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(vendor/glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
