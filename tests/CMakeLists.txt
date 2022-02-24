FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

FetchContent_MakeAvailable(googletest)

add_executable(thyme_tests globals.cpp test_filesystem.cpp)
target_link_libraries(thyme_tests gtest gtest_main)
target_compile_definitions(thyme_tests PRIVATE -DTESTDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data")

if(STANDALONE)
  target_link_libraries(thyme_tests thyme_lib)
else()
  target_link_libraries(thyme_tests thyme_dll)
endif()

include(GoogleTest)
gtest_discover_tests(thyme_tests)