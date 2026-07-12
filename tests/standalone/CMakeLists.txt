#
# Standalone, fast TDD build for the mod-branded-bots pure core.
# Builds ONLY core/ + mod-branding core headers + GoogleTest -- no worldserver, no game lib.
# Configure + build in seconds:
#
#   cmake -S modules/mod-branded-bots/tests/standalone -B <builddir>
#   cmake --build <builddir> && ctest --test-dir <builddir> --output-on-failure
#
cmake_minimum_required(VERSION 3.16)
project(mod_branded_bots_core_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Match the googletest version AzerothCore pins (src/cmake/googletest-download.cmake).
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

set(BOTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(CORE_DIR ${BOTS_ROOT}/src/core)
set(TESTS_DIR ${BOTS_ROOT}/tests)
# mod-branding's pure core provides Branding::BrandId and Branding::IRng (single source of truth).
set(BRANDING_CORE_DIR ${BOTS_ROOT}/../mod-branding/src/core)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS ${CORE_DIR}/*.cpp)
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${TESTS_DIR}/*.cpp)

add_executable(branded_bots_core_tests ${CORE_SOURCES} ${TEST_SOURCES})
target_include_directories(branded_bots_core_tests PRIVATE
    ${CORE_DIR} ${BRANDING_CORE_DIR} ${TESTS_DIR})
target_link_libraries(branded_bots_core_tests PRIVATE gtest_main)

# -Wall -Wextra to mirror CI; the core must compile clean.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(branded_bots_core_tests PRIVATE -Wall -Wextra)
endif()

enable_testing()
include(GoogleTest)
gtest_discover_tests(branded_bots_core_tests)
