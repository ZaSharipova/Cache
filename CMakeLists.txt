cmake_minimum_required(VERSION 3.14)
project(multilevel_cache CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

find_package(GTest REQUIRED)
enable_testing()
include(GoogleTest)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${SRC_DIR}/tests)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CACHE_SOURCES
    ${SRC_DIR}/lru_cache.cpp
    ${SRC_DIR}/lfu_cache.cpp
    ${SRC_DIR}/2Qcache.cpp
    ${SRC_DIR}/lirs_cache.cpp
    ${SRC_DIR}/opt.cpp
)

add_library(cache_lib ${CACHE_SOURCES})
target_include_directories(cache_lib PUBLIC ${INCLUDE_DIR})

add_executable(test_single_cache
    ${TEST_DIR}/test_single_cache.cpp
)
target_link_libraries(test_single_cache PRIVATE cache_lib GTest::gtest_main)
gtest_discover_tests(test_single_cache)

add_executable(test_multilevel_cache
    ${TEST_DIR}/test_multilevel_cache.cpp
)
target_link_libraries(test_multilevel_cache PRIVATE cache_lib GTest::gtest_main)
gtest_discover_tests(test_multilevel_cache)


add_executable(test_invariants
    ${SRC_DIR}/subsidiary.cpp
    ${TEST_DIR}/test_invariants.cpp
)
target_link_libraries(test_invariants PRIVATE cache_lib GTest::gtest_main)
gtest_discover_tests(test_invariants)

add_executable(benchmarks_patterns
    ${SRC_DIR}/subsidiary.cpp
    ${TEST_DIR}/benchmarks_patterns.cpp
)
target_link_libraries(benchmarks_patterns PRIVATE cache_lib)

add_executable(benchmarks_multilevel
    ${SRC_DIR}/subsidiary.cpp
    ${TEST_DIR}/benchmarks_multilevel.cpp
)
target_link_libraries(benchmarks_multilevel PRIVATE cache_lib)
