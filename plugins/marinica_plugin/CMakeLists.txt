cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(qmarinica-plugin SHARED
   src/marinica_player.cpp
   src/player_registration.cpp
   src/board.cpp
   src/debug.cpp
   src/tests.cpp
   src/min_path.cpp
)

target_include_directories(qmarinica-plugin PUBLIC include)
target_link_libraries(qmarinica-plugin qcore)
