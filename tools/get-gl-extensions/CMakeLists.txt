find_package(OpenGL MODULE REQUIRED)
#find_package(GLEW CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES "*.cpp" "*.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})

add_executable(get-gl-extensions WIN32 ${SRC_FILES})

set_target_properties(get-gl-extensions PROPERTIES
	LINK_FLAGS /SUBSYSTEM:CONSOLE
)

target_link_libraries(get-gl-extensions
    PUBLIC

    PRIVATE
        OpenGL::GL
#        GLEW::GLEW
)
