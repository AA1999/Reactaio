cmake_minimum_required(VERSION 3.23)
project(Reactaio VERSION 1.0 DESCRIPTION "A multi-purpose AIO discord bot written in C++.")

file(GLOB_RECURSE sources src/*.cppm)

add_executable(${PROJECT_NAME}
        ${sources}
        src/cogs/core/datatypes/duration.cppm
        src/cogs/core/strings.cppm
        src/cogs/core/datatypes/split_proxy.cppm
        src/cogs/core/datatypes/splitting_iterator.cppm
        src/cogs/core/aliases.cppm
        src/cogs/core/datatypes/colors.cppm
        src/cogs/core/datatypes/fifo_map.cppm
        src/cogs/core/datatypes/message_paginator.cppm
        src/cogs/core/datatypes/discord_command.cppm
        src/cogs/core/datatypes/parser_item.cppm
        src/cogs/core/consts.cppm
)

find_package(nlohmann_json 3.11.2 REQUIRED)

target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        pqxx
        pq
        )

set(CMAKE_CXX_STANDARD 20)

target_compile_definitions(${PROJECT_NAME} PUBLIC DPP_CORO)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pipe -O3  -fexperimental-library -v -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++20 -fexperimental-library")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_LINK_DEPENDS_USE_LINKER "lld")