cmake_minimum_required(VERSION 3.23)
<<<<<<< HEAD

project(Reactaio)
=======
project(Reactaio VERSION 1.0 DESCRIPTION "A multi-purpose AIO discord bot written in C++.")
>>>>>>> e164fa63db6f10723a27f9adb1e0b26236495e77

file(GLOB_RECURSE sources src/*.cpp src/*.h)

add_executable(${PROJECT_NAME}
        ${sources}
        src/cogs/moderation/wrappers/ban_processor.cpp src/cogs/moderation/wrappers/ban_processor.h)

find_package(nlohmann_json 3.11.2 REQUIRED)

target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        pqxx
        fmt
        )

<<<<<<< HEAD
target_include_directories( ${PROJECT_NAME} PRIVATE
        lib/DPP/include
        lib/spdlog/include
        lib/ctre/include
        lib/libpqxx/include
)

target_precompile_headers(${PROJECT_NAME} PRIVATE lib/ctre/include/ctre.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
=======
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++  -fexperimental-library -v")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++20 -stdlib=libc++ -fexperimental-library -lc++-abi")
#set(CMAKE_CXX_COMPILER "clang++")
>>>>>>> e164fa63db6f10723a27f9adb1e0b26236495e77
