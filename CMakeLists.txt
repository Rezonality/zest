cmake_minimum_required(VERSION 3.5)

message(STATUS " CMakeLists: Zest")

# Global Options
option(ZING_BUILD_TESTS "Build Tests" ON)

# Global Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_DEBUG_POSTFIX "-debug")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-reldbg")

if (WIN32)
    set(VCPKG_TARGET_TRIPLET x64-windows-static-md)
endif()

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(Zest
    LANGUAGES CXX C
    VERSION 0.5.0
    )

# config_app.h checks
# This makes a config_shared.h file which can be included for system settings
#  Must be after setting up the platforms
set(ZEST_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(M3RDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/libs)
set(ZEST_INCLUDE_DIR include)
set(ZEST_LIB_DIR libs)

include(cmake/all.cmake)
include(cmake/utils.cmake)

# The library
include(src/CMakeLists.txt)

if (ZEST_BUILD_TESTS)
# The tests
include(tests/CMakeLists.txt)
endif()


