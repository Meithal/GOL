cmake_minimum_required(VERSION 3.29)
project(GOL C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

# find_package(OpenGL REQUIRED)

add_executable(GOL
        main.c
        glad/src/glad.c
        smoothlife.c
        smoothlife.h
        util_glfw.c
        util_glfw.h
        lenia.c
        lenia.h
        )

target_include_directories(GOL PUBLIC glad/include)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

target_include_directories(GOL PRIVATE glfw/include)
target_include_directories(GOL PRIVATE glfw/deps)
target_link_libraries(GOL PRIVATE glfw)

# Define the source and destination directories for shader files
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BUILD_DIR "${CMAKE_BINARY_DIR}/shaders")

# Create a list of shader files
file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*")

# Custom command to copy the shaders only when they are changed
add_custom_command(
        OUTPUT ${SHADER_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_FILES} ${SHADER_BUILD_DIR}
        DEPENDS ${SHADER_FILES}
        COMMENT "Copying shader files to the build directory if they have changed"
)

# Add a custom target that depends on the shaders to ensure they are copied
add_custom_target(copy_shaders ALL DEPENDS ${SHADER_BUILD_DIR})
ADD_DEPENDENCIES ( GOL copy_shaders )

add_executable(Rafler
        raffler.c
        rafler.h
        glad/src/glad.c
)

target_include_directories(Rafler PRIVATE glfw/include)
target_include_directories(Rafler PRIVATE glfw/deps)
target_link_libraries(Rafler PRIVATE glfw)
target_include_directories(Rafler PUBLIC glad/include)

ADD_DEPENDENCIES ( Rafler copy_shaders )

add_executable(Mandel
        mandel.cpp
        glad/src/glad.c
)

target_include_directories(Mandel PRIVATE glfw/include)
target_include_directories(Mandel PRIVATE glfw/deps)
target_link_libraries(Mandel PRIVATE glfw)
target_include_directories(Mandel PUBLIC glad/include)

ADD_DEPENDENCIES ( Mandel copy_shaders )
