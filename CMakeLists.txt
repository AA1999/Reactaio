cmake_minimum_required(VERSION 3.23)

project(Reactaio)

add_subdirectory(lib/DPP)
add_subdirectory(lib/spdlog)
add_subdirectory(lib/ctre)
add_subdirectory(lib/libpqxx)

add_executable(${PROJECT_NAME}
        src/reactaio.cpp
        src/token.h
        src/exceptions/discord_exception.cpp
        src/exceptions/discord_exception.h
        src/exceptions/missing_permissions.cpp
        src/exceptions/missing_permissions.h
        src/exceptions/not_found.cpp
        src/exceptions/not_found.h
        src/exceptions/owner_exception.cpp
        src/exceptions/owner_exception.h
        src/modules/cog.h
        src/modules/moderation/moderation.h
        src/modules/moderation/moderation.cpp
        src/modules/moderation/utilities.cpp 
        src/modules/command/command.h)

target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        ctre
        libpqxx
)

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
