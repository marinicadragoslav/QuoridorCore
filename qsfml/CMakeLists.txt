cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
find_library(consoleApp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if (ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Create quoridor-sfml executable
add_executable(quoridor-sfml
   src/main.cpp
   src/FontLoader.cpp
   src/TextFactory.cpp
   src/Board.cpp
   src/TileMap.cpp
   src/ImGUIConsoleWidget.cpp
   src/ImGUIBoardWidget.cpp
   src/ImGuiTimelineWidget.cpp
   src/TileMap.cpp
   src/ManualPlayer.cpp
)
target_include_directories(quoridor-sfml PRIVATE include)

set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES}
                         imgui-sfml/2.1@bincrafters/stable)

include(../cmake/conan.cmake)
run_conan()

set (source "${CMAKE_CURRENT_SOURCE_DIR}/resources")
IF (WIN32)
  set (destination "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/resources")

  add_custom_command(
    TARGET quoridor-sfml POST_BUILD
    COMMAND xcopy /S /E /I /Y \"${source}\" \"${destination}\" 
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
  )

  set_property(TARGET quoridor-sfml PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
ELSE()
  set (destination "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")

  add_custom_command(
    TARGET quoridor-sfml POST_BUILD
    COMMAND echo "cp -Ru ${source} ${destination}" && cp -Ru ${source} ${destination}
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
  )
ENDIF()

target_link_libraries(
  quoridor-sfml PRIVATE qcore consoleApp project_options project_warnings CONAN_PKG::docopt.cpp
  CONAN_PKG::fmt CONAN_PKG::spdlog CONAN_PKG::imgui-sfml)
