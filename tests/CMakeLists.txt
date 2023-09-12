FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)

FetchContent_MakeAvailable(googletest)

set(TEST_SRCS
  main.cpp
  globals.cpp
  test_audiofilecache.cpp
  test_compr.cpp
  test_audiomanager.cpp
  test_crc.cpp
  test_filesystem.cpp
  test_text.cpp
  test_videoplayer.cpp
  test_w3d_load.cpp
  test_w3d_math.cpp
)

add_executable(thyme_tests ${TEST_SRCS})
target_link_libraries(thyme_tests gtest)
set(THYME_TESTDATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data)
target_compile_definitions(thyme_tests PRIVATE -DTESTDATA_PATH="${THYME_TESTDATA_PATH}")

if(STANDALONE)
  target_link_libraries(thyme_tests thyme_lib)
else()
  target_link_libraries(thyme_tests thyme_dll)
endif()

include(GoogleTest)
gtest_discover_tests(thyme_tests)

if(BUILD_TOOLS)
  # Test compression & decompression
  set(COMPR_TYPES EAR ZL1 ZL2 ZL3 ZL4 ZL5 ZL6 ZL7 ZL8 ZL9)
  foreach(TYPE ${COMPR_TYPES})
    add_test(NAME "compr_${TYPE}" COMMAND compressor -t ${TYPE} -i ${THYME_TESTDATA_PATH}/compr/uncompr.txt -o ${CMAKE_CURRENT_BINARY_DIR}/compr_${TYPE}.data)
    set_tests_properties("compr_${TYPE}" PROPERTIES FIXTURES_SETUP DATA_${TYPE})
    add_test(NAME "decompr_${TYPE}" COMMAND compressor -d -i ${CMAKE_CURRENT_BINARY_DIR}/compr_${TYPE}.data -o ${CMAKE_CURRENT_BINARY_DIR}/uncompr_${TYPE}.txt)
    set_tests_properties("decompr_${TYPE}" PROPERTIES FIXTURES_REQUIRED DATA_${TYPE})
  endforeach()
endif()
