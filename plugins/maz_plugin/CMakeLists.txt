cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Create qcore shared library
add_library(ABotV2 SHARED
   src/PluginMain.cpp
   src/PlayerRegistration.cpp
   src/ABotV2Analyser.cpp
   src/ABBoard.cpp
   src/SolutionNode.cpp
   src/MoveInserter.cpp
)

target_include_directories(ABotV2 PUBLIC include/mcts)
target_include_directories(ABotV2 PUBLIC include)
target_link_libraries(ABotV2 qcore)
