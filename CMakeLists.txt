cmake_minimum_required(VERSION 3.10)
project(filters)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

option(ENABLE_BSD "include Bloomfilter BSD Library" OFF)

if (ENABLE_BSD)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
    # add bsd library first so it can set it own compiler flags
    set(BOOST_REQUESTED_VERSION "1.70.0")
    include("${CMAKE_SOURCE_DIR}/vendor/boost.cmake")

    add_subdirectory(lib/bsd)
endif ()

find_package(Numa)

set(NUMA_FOUND OFF)

# set compile flags for rest of project
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-vectorize -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} --coverage -fprofile-arcs -ftest-coverage -Wall -Wextra -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static-libasan -static-libubsan -fsanitize=address -fsanitize=leak -fsanitize=undefined -Wall -Wextra -Wno-unused-function -march=native")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include("${CMAKE_SOURCE_DIR}/vendor/googlebenchmark.cmake")
include("${CMAKE_SOURCE_DIR}/vendor/googletest.cmake")

include(cmake/coverage.cmake)


# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

message(STATUS "NUMA = ${NUMA_FOUND}")
message(STATUS "[COMPILATION] settings")
message(STATUS "    CMAKE_CXX_FLAGS_RELEASE         = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "    CMAKE_CXX_FLAGS_RELWITHDEBINFO  = ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "    CMAKE_CXX_FLAGS_DEBUG           = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "[BOOST] settings")
message(STATUS "    BOOST_INCLUDE_DIR               = ${BOOST_INCLUDE_DIR}")
message(STATUS "    BOOST_LIBRARY_DIR               = ${BOOST_LIBRARY_DIR}")
message(STATUS "[BENCHMARK] settings")
message(STATUS "    BENCHMARK_INCLUDE_DIR           = ${BENCHMARK_INCLUDE_DIR}")
message(STATUS "    BENCHMARK_LIBRARY_PATH          = ${BENCHMARK_LIBRARY_PATH}")
message(STATUS "[TEST] settings")
message(STATUS "    GTEST_INCLUDE_DIR               = ${GTEST_INCLUDE_DIR}")
message(STATUS "    GTEST_LIBRARY_PATH              = ${GTEST_LIBRARY_PATH}")


# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

add_subdirectory(lib/cityhash)
add_subdirectory(lib/libdivide)
add_subdirectory(lib/perfevent)
add_subdirectory(lib/fastfilter)
add_subdirectory(lib/impala)
add_subdirectory(lib/amd_mortonfilter)
add_subdirectory(lib/efficient_cuckoofilter)
add_subdirectory(lib/vacuumfilter)

add_subdirectory(src)

# ---------------------------------------------------------------------------
# Modules
# ---------------------------------------------------------------------------

add_subdirectory(test)
add_subdirectory(benchmark)
