file(GLOB SOURCES ${PROJECT_SOURCE_DIR} *.cpp *.hpp)

if (LINUX)
	add_compile_options(-Wall -Wextra -fno-diagnostics-show-caret -fno-rtti)
elseif (MSVC)
	message(AUTHOR_WARNING "MSVC support has not been tested yet")
	add_compile_options(/W4 /GR-)
	add_link_options(/subsystem:windows /entry:mainCRTStartup)
else()
	message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not supported")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD_REQUIRED True
	CXX_STANDARD 17
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)
