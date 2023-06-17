cmake_minimum_required(VERSION 3.2)

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

option (VCPKG_IMGUI ON)

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

# Make the CMake bits that ensure find_package does the right thing
install(EXPORT zest-targets
    FILE zest-targets.cmake
    NAMESPACE Zest::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zest
    )

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${ZEST_ROOT}/cmake/zest-config.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/zest-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zest
    PATH_VARS ZEST_INCLUDE_DIR
    PATH_VARS ZEST_LIB_DIR
    )

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/zest-config-version.cmake
    VERSION 1.0
    COMPATIBILITY AnyNewerVersion
    )

install(
    FILES
    ${CMAKE_BINARY_DIR}/cmake/zest-config.cmake
    ${CMAKE_BINARY_DIR}/cmake/zest-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zest
    )

