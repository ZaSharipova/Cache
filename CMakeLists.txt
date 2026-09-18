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
    lirs_cache.cpp
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

add_executable(test_lirs
    test_lirs.cpp
    ${CACHE_SOURCES}
)
target_link_libraries(test_lirs PRIVATE GTest::gtest_main)
gtest_discover_tests(test_lirs)

add_executable(invariant_tests
    invariant_tests.cpp
    ${CACHE_SOURCES}
)
target_link_libraries(invariant_tests PRIVATE GTest::gtest_main)
gtest_discover_tests(invariant_tests)

add_executable(benchmarks
    subsidiary.cpp
    benchmarks.cpp
    ${CACHE_SOURCES}
)

add_executable(benchmarks_multilevel
    subsidiary.cpp
    benchmarks_multilevel.cpp
    ${CACHE_SOURCES}
)
