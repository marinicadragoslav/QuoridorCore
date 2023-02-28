cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(qdrma-plugin SHARED
   src/drma_player.cpp
   src/player_registration.cpp
   src/board.cpp
   src/debug.cpp
   src/tests.cpp
   src/min_path.cpp
   src/minimax.cpp
   src/plugin_core_interface.cpp
)

target_include_directories(qdrma-plugin PUBLIC include)
target_link_libraries(qdrma-plugin qcore)
