cmake_minimum_required(VERSION 3.29)
project(GOL C)

set(CMAKE_C_STANDARD 23)

find_package(OpenGL REQUIRED)

add_executable(GOL main.c glad/src/glad.c)

target_include_directories(GOL PUBLIC glad/include)

add_subdirectory(glfw)
target_include_directories(GOL PRIVATE glfw/include)
target_include_directories(GOL PRIVATE glfw/deps)
target_link_libraries(GOL PRIVATE glfw)

