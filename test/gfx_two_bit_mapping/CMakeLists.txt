add_executable(GfxTwoBitMappingTest GfxTwoBitMappingTest.cpp)

target_include_directories(GfxTwoBitMappingTest BEFORE PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/test/memory_safety/stubs
)
target_include_directories(GfxTwoBitMappingTest PRIVATE
  ${REPO_ROOT}/lib/Epub/Epub/converters
  ${REPO_ROOT}/lib/EpdFont
  ${REPO_ROOT}/lib/GfxRenderer
)
target_compile_definitions(GfxTwoBitMappingTest PRIVATE FREEINK_DEVICE_EEGO_A4=1)
target_link_libraries(GfxTwoBitMappingTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(GfxTwoBitMappingTest)

# Non-A4 (e.g. X4 Pro) build of the same test file: exercises the #else
# branches of the mapping tables (FREEINK_DEVICE_EEGO_A4 stays undefined).
add_executable(GfxTwoBitMappingNoA4Test GfxTwoBitMappingTest.cpp)

target_include_directories(GfxTwoBitMappingNoA4Test BEFORE PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/test/memory_safety/stubs
)
target_include_directories(GfxTwoBitMappingNoA4Test PRIVATE
  ${REPO_ROOT}/lib/Epub/Epub/converters
  ${REPO_ROOT}/lib/EpdFont
  ${REPO_ROOT}/lib/GfxRenderer
)
target_link_libraries(GfxTwoBitMappingNoA4Test PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(GfxTwoBitMappingNoA4Test)
