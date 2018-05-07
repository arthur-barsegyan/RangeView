cmake_minimum_required(VERSION 3.9)
project(ViewsAPI)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("include")

include("3rd-party/DownloadProject/DownloadProject.cmake")

download_project(PROJ                cmake-modules
        GIT_REPOSITORY      https://github.com/NikitkoCent/cmake-modules
        GIT_TAG             changes
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )
# ================================== GOOGLETEST =========================================
enable_testing()

download_project(PROJ                googletest
        GIT_REPOSITORY      https://github.com/google/googletest.git
        GIT_TAG             master
        ${UPDATE_DISCONNECTED_IF_AVAILABLE}
        )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
endif()
# ================================ GOOGLETEST END =======================================

add_executable(unittesting include/views.hpp test/test.cpp)
target_link_libraries(unittesting gtest gmock_main)
add_test(NAME do_unittests COMMAND unittesting)

# ================================= TEST COVERAGE =======================================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(${cmake-modules_SOURCE_DIR}/CodeCoverage.cmake)

    if (CMAKE_CXX_FLAGS_COVERAGE)
        APPEND_COVERAGE_COMPILER_FLAGS()
        SETUP_TARGET_FOR_COVERAGE(NAME collect_coverage
                EXECUTABLE ctest
                DEPENDENCIES unittesting)
    endif()
endif()
# ===============================  END TEST COVERAGE ====================================

MESSAGE("Build type : ${CMAKE_BUILD_TYPE}")
MESSAGE("Used compiler : ${CMAKE_CXX_COMPILER}")

    #add_executable(ViewsAPI
    #        main.cpp
    #        include/views.hpp)
