cmake_minimum_required(VERSION 3.14)
project(multilevel_cache CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall)

find_package(GTest REQUIRED)
enable_testing()
include(GoogleTest)

set(CACHE_SOURCES
    lru_cache.cpp
    lfu_cache.cpp
    2Qcache.cpp
    opt.cpp
)

add_executable(test_cache
    test.cpp
    ${CACHE_SOURCES}
)
target_link_libraries(test_cache PRIVATE GTest::gtest_main)
gtest_discover_tests(test_cache)

add_executable(test_multilevel_cache
    test_multilevel_cache.cpp
    ${CACHE_SOURCES}
)
target_link_libraries(test_multilevel_cache PRIVATE GTest::gtest_main)
gtest_discover_tests(test_multilevel_cache)
