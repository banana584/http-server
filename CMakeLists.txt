cmake_minimum_required(VERSION 3.10)
project(HTTPServer)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

# Add all .cpp files in src/ directory
file(GLOB_RECURSE SOURCES "src/**/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp ${SOURCES})

# Optionally, include header files
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")