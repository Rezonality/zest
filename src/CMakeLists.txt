find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_path(TINYDIR_INCLUDE_DIR "tinydir.h")

# Used Packages external to vcpkg
include(GNUInstallDirs)

set(UTILS_SOURCES
    ${ZEST_ROOT}/src/CMakeLists.txt
    ${ZEST_ROOT}/src/algorithm/container_utils.cpp
    ${ZEST_ROOT}/src/callback/callback.cpp
    ${ZEST_ROOT}/src/file/file.cpp
    ${ZEST_ROOT}/src/file/runtree.cpp
    ${ZEST_ROOT}/src/file/toml_utils.cpp
    ${ZEST_ROOT}/src/math/math_utils.cpp
    ${ZEST_ROOT}/src/settings/settings.cpp
    ${ZEST_ROOT}/src/string/murmur_hash.cpp
    ${ZEST_ROOT}/src/string/string_utils.cpp
    ${ZEST_ROOT}/src/time/profiler.cpp
    ${ZEST_ROOT}/src/time/time_provider.cpp
    ${ZEST_ROOT}/src/time/timer.cpp
    ${ZEST_ROOT}/src/ui/colors.cpp
    ${ZEST_ROOT}/src/ui/dpi.cpp
    ${ZEST_ROOT}/src/ui/imgui_extras.cpp

    ${ZEST_ROOT}/include/zest/algorithm/container_utils.h
    ${ZEST_ROOT}/include/zest/algorithm/ringiterator.h
    ${ZEST_ROOT}/include/zest/file/runtree.h
    ${ZEST_ROOT}/include/zest/file/toml_utils.h
    ${ZEST_ROOT}/include/zest/thread/thread_utils.h
    ${ZEST_ROOT}/include/zest/thread/threadpool.h
    ${ZEST_ROOT}/include/zest/ui/colors.h
    ${ZEST_ROOT}/include/zest/ui/dpi.h
    ${ZEST_ROOT}/include/zest/ui/imgui_extras.h
    )

# Create the library
add_library(Zest STATIC
    ${UTILS_SOURCES}
    )

target_compile_definitions(Zest
    PUBLIC
    )

add_library(Zest::Zest ALIAS Zest)

target_include_directories(Zest
    PRIVATE
        $<BUILD_INTERFACE:${ZEST_ROOT}/libs>
        $<BUILD_INTERFACE:${ZEST_ROOT}/include>
        ${TINYDIR_INCLUDE_DIR}
        ${ZEST_ROOT}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
    PUBLIC
        $<BUILD_INTERFACE:${ZEST_ROOT}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Zest 
PUBLIC
    imgui::imgui
    glm::glm
    unofficial::concurrentqueue::concurrentqueue
    )

# Set locations for components
set_target_properties(Zest PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

# Install the binary
install(TARGETS Zest
    EXPORT zest-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
    )

# CMake
install(FILES ${ZEST_ROOT}/cmake/utils.cmake
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cmake
    )

# Install the include files
install(DIRECTORY ${ZEST_ROOT}/include/zest
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

source_group ("Zest\\Timing" .*time.*)
source_group ("Zest\\String" .*string.*)
source_group ("Zest\\Math" .*math.*)
source_group ("Zest\\Geometry" .*geometry.*)
source_group ("Zest\\File" .*file.*)
source_group ("Zest\\Compile" .*compile.*)
source_group ("Zest\\Chibi" .*chibi.*)
source_group ("Zest\\Callback" .*callback.*)
source_group ("Zest\\GL" .*gl.*)
source_group ("Zest\\Thread" .*thread.*)
source_group ("Zest\\UI" .*ui.*)
source_group ("Zest\\VM" .*vm.*)
source_group ("Zest\\Device" .*device.*)
source_group ("Zest\\Algorithm" .*algorithm.*)
