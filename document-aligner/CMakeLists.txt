cmake_minimum_required(VERSION 2.8.3)

# Define a single cmake project
project(document-aligner)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Ofast")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g ")

# Compile all executables into bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# We need boost
find_package(Boost 1.41.0 REQUIRED COMPONENTS
  program_options
)

# Define where include files live
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIR}
)

if (PREPROCESS_PATH)
  include_directories(${PREPROCESS_PATH})
  add_subdirectory(${PREPROCESS_PATH} preprocess-bin)
else()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../preprocess preprocess-bin)
endif()

# find *.h and *.cpp files
file(GLOB dalign_cpp_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB dalign_cpp_cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(REMOVE_ITEM dalign_cpp_cpp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/catdoc.cpp)

# dalign_cpp
add_executable(dalign main.cpp ${dalign_cpp_headers} ${dalign_cpp_cpp})
target_link_libraries(dalign ${Boost_LIBRARIES} preprocess_util)

# Stupid tool to read base64 gzipped files
add_executable(catdoc catdoc.cpp ${dalign_cpp_headers} ${dalign_cpp_cpp})
target_link_libraries(catdoc ${Boost_LIBRARIES} preprocess_util)