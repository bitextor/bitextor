cmake_minimum_required(VERSION 3.5.0)
project(bleualign-cpp)

set(CMAKE_CXX_STANDARD 11)
set(NAME bleualign-cpp)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# options
option(BUILD_TEST "Build tests" OFF)


# flags
#set(CMAKE_BUILD_TYPE release)
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -pthread -O3 -Ofast")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pthread -g ")


# Boost
find_package(Boost REQUIRED COMPONENTS filesystem log program_options system iostreams regex)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DU_USING_ICU_NAMESPACE=1")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess preprocess-bin)

# find *.h and *.cpp files
file(GLOB bleualign_cpp_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.h)
file(GLOB bleualign_cpp_cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp)

# make bleualign_cpp_lib library
add_library(bleualign_cpp_lib SHARED ${bleualign_cpp_headers} ${bleualign_cpp_cpp})
target_link_libraries(bleualign_cpp_lib ${Boost_LIBRARIES} preprocess_util)

# bleualign_cpp
add_executable(bleualign_cpp main.cpp)
target_link_libraries(bleualign_cpp bleualign_cpp_lib)


# build tests with GTEST
if (BUILD_TEST)

    ENABLE_TESTING()

    # Enable ExternalProject CMake module
    include(ExternalProject)

    # Download and install GoogleTest
    ExternalProject_Add(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
    )

    # Include
    ExternalProject_Get_Property(googletest source_dir)
    set(GTEST_INCLUDE_DIR ${source_dir}/googletest/include/)
    include_directories(${GTEST_INCLUDE_DIR})

    # The cloning of the above repo doesn't happen until make, however if the dir doesn't
    # exist, INTERFACE_INCLUDE_DIRECTORIES will throw an error.
    file(MAKE_DIRECTORY ${GTEST_INCLUDE_DIR})

    # Library
    ExternalProject_Get_Property(googletest binary_dir)
    set(GTEST_LIBRARY_PATH ${binary_dir}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
    set(GTEST_LIBRARY libgtest)
    add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
    set_target_properties(${GTEST_LIBRARY} PROPERTIES
        "IMPORTED_LOCATION" "${GTEST_LIBRARY_PATH}"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIR}")
    add_dependencies(${GTEST_LIBRARY} googletest)

    # Add test files
    add_subdirectory(tests)
    add_test(NAME test_all COMMAND ./tests/test_all)

endif (BUILD_TEST)
