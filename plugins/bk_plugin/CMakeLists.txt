cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Create qcore shared library
add_library(BK_Plugin SHARED
   src/PluginMain.cpp
   src/PlayerRegistration.cpp
   src/mcts_impl.cpp
   src/mcts/JobScheduler.cpp
   src/mcts/mcts.cpp
)

target_include_directories(BK_Plugin PUBLIC include/mcts)
target_include_directories(BK_Plugin PUBLIC include)
target_link_libraries(BK_Plugin qcore)
