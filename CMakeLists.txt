cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project(bitextor)

set(PREPROCESS_PATH "${PROJECT_SOURCE_DIR}/preprocess")
set(SKIP_PREPROCESS_BUILD ON) # avoid compiling preprocess a million times
set(CMAKE_BUILD_TYPE "Release")
include(GNUInstallDirs)

# override output paths of children to put everything in the same folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(NOT SKIP_PREPROCESS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/preprocess preprocess)
    install(TARGETS b64filter cache docenc foldfilter mmhsum
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(NOT SKIP_WARC2TEXT)
    add_subdirectory(${PROJECT_SOURCE_DIR}/warc2text warc2text)
    # install handled from warc2text/CMakeLists.txt
endif()

if(NOT SKIP_BLEUALIGN)
    add_subdirectory(${PROJECT_SOURCE_DIR}/bleualign-cpp bleualign-cpp)
    # install handled from bleualign-cpp/CMakeLists.txt
endif()

if (NOT SKIP_HUNALIGN)
    # add_custom_target(hunalign ALL
    #     COMMAND $(MAKE)
    #     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/hunalign/src/hunalign
    # )
    # add_custom_command(TARGET hunalign
    #     POST_BUILD
    #     COMMAND bash -c "mv ${PROJECT_SOURCE_DIR}/hunalign/src/hunalign/hunalign ${PROJECT_BINARY_DIR}/bin/hunalign"
    # )
    add_subdirectory(${PROJECT_SOURCE_DIR}/hunalign/src)
    install(PROGRAMS ${PROJECT_BINARY_DIR}/bin/hunalign
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if (NOT SKIP_DOCALIGN)
    set(BUILD_TESTING OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/document-aligner document-aligner)
    # install handled from document-aligner/CMakeLists.txt
endif()

if (NOT SKIP_BIROAMER)
    add_subdirectory(${PROJECT_SOURCE_DIR}/biroamer/fast_align fast_align)
    install(TARGETS fast_align atools DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (NOT SKIP_MGIZA)
    add_subdirectory(${PROJECT_SOURCE_DIR}/mgiza/mgizapp mgizapp)
endif()
