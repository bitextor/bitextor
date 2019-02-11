
# Find all cpp files
file(GLOB test_cpps ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Build all
add_executable(test_all ${test_cpps})
target_link_libraries(test_all bleualign_cpp_lib ${GTEST_LIBRARY})
install(TARGETS test_all DESTINATION tests)
