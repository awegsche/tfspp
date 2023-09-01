cmake_minimum_required(VERSION 3.12.0)
project(tfs_cpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------------------------------
# ---- The library ---------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

add_library(tfs_cpp
    src/lib.cpp
)

# --------------------------------------------------------------------------------------------------
# ---- Testing -------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------

include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    tests
    tests/tests.cpp
    )

target_link_libraries(
    tests
    GTest::gtest_main
    )

include(GoogleTest)
gtest_discover_tests(tests)

# --------------------------------------------------------------------------------------------------
# ---- CPack ---------------------------------------------------------------------------------------
# --------------------------------------------------------------------------------------------------
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
