cmake_minimum_required(VERSION 3.12)

# Define a single cmake project
project(document-aligner)

enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
set(CMAKE_CXX_FLAGS_DEBUG "-Wextra -g")

# Compile all executables into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# We need boost
find_package(Boost 1.41.0 REQUIRED COMPONENTS
  program_options
  unit_test_framework
)

find_package(ICU REQUIRED COMPONENTS
  i18n uc data io
)

# Define where include files live
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIRS}
)

if (PREPROCESS_PATH)
  include_directories(${PREPROCESS_PATH})
  add_subdirectory(${PREPROCESS_PATH} preprocess-bin)
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess preprocess-bin)
endif()

# find *.h and *.cpp files
file(GLOB dalign_cpp_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB dalign_cpp_cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB dalign_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)

# Tool to score alignment between two sets of documents in the same language.
add_executable(docalign docalign.cpp ${dalign_cpp_headers} ${dalign_cpp_cpp})
target_link_libraries(docalign ${Boost_LIBRARIES} preprocess_util)

# Tool to (left) join documents from two sets into a single TSV stream
# Similar to coreutils join, but using line indices and works on gzipped files
add_executable(docjoin docjoin.cpp ${dalign_cpp_headers} ${dalign_cpp_cpp})
target_link_libraries(docjoin preprocess_util)

if (BUILD_TESTING)
  add_executable(ngram_test tests/ngram_test.cpp ${dalign_cpp_headers} ${dalign_cpp_cpp})
  target_compile_definitions(ngram_test PRIVATE "BOOST_TEST_DYN_LINK=1")
  target_link_libraries(ngram_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} preprocess_util)
  add_test(NAME ngram_test COMMAND ngram_test)
endif (BUILD_TESTING)

