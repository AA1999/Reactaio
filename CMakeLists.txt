cmake_minimum_required(VERSION 3.25)
project(Reactaio VERSION 1.0 DESCRIPTION "A multi-purpose AIO discord bot written in C++.")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp src/*.h src/*.hpp)

find_package(nlohmann_json 3.11.2 REQUIRED)
find_package(DPP 10.0.30 REQUIRED)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME}
        ${DPP_LIBRARIES}
        spdlog
        pqxx
        pq
        fmt
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${DPP_INCLUDE_DIR}
)

set(CMAKE_CXX_STANDARD 20)

target_compile_definitions(${PROJECT_NAME} PUBLIC DPP_CORO)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pipe -O3  -fexperimental-library -v -Wall -Wextra -Wpedantic -fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++20 -fexperimental-library")