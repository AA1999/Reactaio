cmake_minimum_required(VERSION 3.23)
project(Reactaio VERSION 1.0 DESCRIPTION "A multi-purpose AIO discord bot written in C++.")

file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(${PROJECT_NAME}
        ${sources}
)

find_package(nlohmann_json 3.11.2 REQUIRED)

target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        pqxx
        )

set(CMAKE_CXX_STANDARD 20)

target_compile_definitions(${PROJECT_NAME} PUBLIC DPP_CORO)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20  -fexperimental-library -v")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++20 -fexperimental-library")
set(CMAKE_CXX_COMPILER "clang++")