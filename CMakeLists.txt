cmake_minimum_required(VERSION 3.10)
project(fstring VERSION 0.1.0)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add executable for the test
add_executable(fstring_test fstring_test.cpp)

# Add testing
enable_testing()
add_test(NAME FStringTest COMMAND fstring_test)

# Install target for header-only library
install(FILES fstring.hpp DESTINATION include)

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  C++ Standard    : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Project Version : ${PROJECT_VERSION}")