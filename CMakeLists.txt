cmake_minimum_required(VERSION 3.26)

include(FetchContent)
project(Cyrano_seul_contre_tous VERSION 1.0.0 LANGUAGES CXX)

if(WIN32)
  # SonarLint support for CMake requires the following flag to be set on.
  #(see https://community.sonarsource.com/t/sonarlint-for-visual-studio-v4-38-support-cmake-projects/50127)
  set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set (BUILD_SHARED_LIBS FALSE)

FetchContent_Declare( sfml
  URL https://github.com/SFML/SFML/archive/refs/tags/3.0.2.tar.gz
)
option(SFML_BUILD_AUDIO "Build audio" ON)
option(SFML_BUILD_NETWORK "Build network" OFF)
FetchContent_MakeAvailable(sfml)

FetchContent_Declare(
    pugixml
    URL https://github.com/zeux/pugixml/archive/refs/tags/v1.15.tar.gz
)
FetchContent_MakeAvailable(pugixml)

set(CMAKE_CXX_STANDARD 23)

#
# Tmxlite : A simple FetchContent does not work We generate its library
# in the same manner as GoogleTest
#

# Download and unpack tmxlite at configure time
set (TMXLITE_STATIC_LIB TRUE)
configure_file(CMakeLists_tmxlite.txt.in tmxlite-download/tmxlite/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tmxlite-download/tmxlite )
if(result)
  message(FATAL_ERROR "CMake step for tmxlite failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tmxlite-download/tmxlite )
if(result)
  message(FATAL_ERROR "Build step for tmxlite failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add tmxlite directly to our build. This defines tmxlite target
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/tmxlite-src/tmxlite
                 ${CMAKE_CURRENT_BINARY_DIR}/tmxlite-build/tmxlite
                 EXCLUDE_FROM_ALL)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/tmxlite-src/tmxlite/include")

#
# Now, we can include subdirectoiries containing sources of our project
#
add_subdirectory(src/core)
add_subdirectory(src/main)

#
# Include GoogleTest
#
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# enable_testing() must be in the source directory root (see cmake documenation at https://cmake.org/cmake/help/latest/command/enable_testing.html)
# Otherwise, Visual Studio test explorer does not see unit tests (See ticket https://developercommunity.visualstudio.com/t/No-tests-discovered-for-googletest-and-C/1148799#T-ND1150621)
include(GoogleTest)
enable_testing()
add_subdirectory(src/test)
