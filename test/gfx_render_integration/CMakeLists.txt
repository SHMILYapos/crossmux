add_executable(GfxRenderIntegrationTest
  GfxRenderIntegrationTest.cpp
  ${REPO_ROOT}/lib/GfxRenderer/GfxRenderer.cpp
  ${REPO_ROOT}/lib/GfxRenderer/BitmapHelpers.cpp
  ${REPO_ROOT}/lib/EpdFont/EpdFontFamily.cpp
  ${REPO_ROOT}/lib/EpdFont/EpdFont.cpp
  ${REPO_ROOT}/lib/MiniBidi/BidiUtils.cpp
  ${REPO_ROOT}/lib/MiniBidi/minibidi.c
  ${REPO_ROOT}/lib/Utf8/Utf8.cpp
  stubs/BuildScratchStubs.cpp
  stubs/SdCardFontStubs.cpp
  stubs/BitmapStubs.cpp
  stubs/FontCacheManagerStubs.cpp
)

target_include_directories(GfxRenderIntegrationTest BEFORE PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
)
target_include_directories(GfxRenderIntegrationTest PRIVATE
  ${REPO_ROOT}
  ${REPO_ROOT}/lib
  ${REPO_ROOT}/lib/EpdFont
  ${REPO_ROOT}/lib/GfxRenderer
  ${REPO_ROOT}/lib/MiniBidi
  ${REPO_ROOT}/lib/Utf8
  ${REPO_ROOT}/lib/Memory
  ${REPO_ROOT}/lib/hal
  ${REPO_ROOT}/lib/InflateReader
)
target_compile_options(GfxRenderIntegrationTest PRIVATE
  -Wall
  -Wextra
)
# GfxRenderer.cpp references millis()/ESP (platform-provided symbols on device);
# inject the host shim into that TU only, so the C translation unit (minibidi.c)
# is not forced to parse C++ headers.
set_source_files_properties(${REPO_ROOT}/lib/GfxRenderer/GfxRenderer.cpp PROPERTIES
  COMPILE_OPTIONS "-include;${CMAKE_CURRENT_SOURCE_DIR}/stubs/ArduinoCompat.h"
)
target_compile_definitions(GfxRenderIntegrationTest PRIVATE
  SIMULATOR=1
)
target_link_libraries(GfxRenderIntegrationTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(GfxRenderIntegrationTest)
