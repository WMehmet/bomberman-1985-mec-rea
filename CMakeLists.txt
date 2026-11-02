cmake_minimum_required(VERSION 3.12)
project(bomberman C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3 /O2)
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

# Find SDL2
find_package(SDL2 REQUIRED)

set(SOURCES
    src/main.c
    src/game.c
    src/graphics.c
    src/audio.c
    src/settings.c
)

add_executable(bomberman ${SOURCES})

target_include_directories(bomberman PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(bomberman PRIVATE
    ${SDL2_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(bomberman PRIVATE m)
endif()

if(WIN32)
    # Enable Windows subsystem (or remove for console window)
    if(MINGW)
        target_link_libraries(bomberman PRIVATE mingw32 SDL2main m)
    endif()
endif()
