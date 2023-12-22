cmake_minimum_required(VERSION 3.23)
project(Reactaio VERSION 1.0 DESCRIPTION "A multi-purpose AIO discord bot written in C++.")

file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(${PROJECT_NAME}
        ${sources}
        src/cogs/moderation/wrappers/bans/ban_processor.cpp src/cogs/moderation/wrappers/bans/ban_processor.h
        src/cogs/moderation/wrappers/bans/guild_bans_wrapper.cpp
        src/cogs/moderation/wrappers/bans/guild_bans_wrapper.h
        src/cogs/base/coro.h
        src/cogs/create_folder.cpp)

find_package(nlohmann_json 3.11.2 REQUIRED)

target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        pqxx
        fmt
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC DPP_CORO)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++  -fexperimental-library -v")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++20 -stdlib=libc++ -fexperimental-library -lc++-abi")
#set(CMAKE_CXX_COMPILER "clang++")