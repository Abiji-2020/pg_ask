cmake_minimum_required(VERSION 3.15)
project(ai_sandbox)

set(CMAKE_CXX_STANDARD 20)

# Point to the SDK
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/ai-sdk-cpp ai-sdk-build)

add_executable(ai_sandbox src/main.cpp)

# Link the SDK target (exported by their CMake)
target_link_libraries(ai_sandbox
    PRIVATE
        ai-sdk-cpp
)
