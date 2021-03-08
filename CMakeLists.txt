cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project(bitextor)

set(PREPROCESS_PATH "${PROJECT_SOURCE_DIR}/preprocess")
set(SKIP_PREPROCESS_BUILD ON) # avoid compiling preprocess a million times
set(CMAKE_BUILD_TYPE "Release")
add_subdirectory(${PROJECT_SOURCE_DIR}/preprocess ${PROJECT_SOURCE_DIR}/preprocess)
add_subdirectory(${PROJECT_SOURCE_DIR}/bleualign-cpp ${PROJECT_SOURCE_DIR}/bleualign-cpp)
add_subdirectory(${PROJECT_SOURCE_DIR}/warc2text ${PROJECT_SOURCE_DIR}/warc2text)
set(BUILD_TESTING OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/document-aligner ${PROJECT_SOURCE_DIR}/document-aligner)
add_subdirectory(${PROJECT_SOURCE_DIR}/biroamer/fast_align ${PROJECT_SOURCE_DIR}/biroamer/fast_align/build)
add_subdirectory(${PROJECT_SOURCE_DIR}/mgiza/mgizapp ${PROJECT_SOURCE_DIR}/mgiza/mgizapp)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/hunalign/src/hunalign/hunalign
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hunalign/src/hunalign
)
add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/clustercat
    COMMAND make
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/clustercat
)
