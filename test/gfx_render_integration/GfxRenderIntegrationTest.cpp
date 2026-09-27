// End-to-end host test for the single-pass 2-bit AA gray render path.
//
// Reproduces the exact renderer calls EpubReaderActivity::renderContents makes
// on the X4 Pro (non-tiled panel) with text AA enabled: render once into a
// caller-owned 2-bits-per-pixel frame, then export the BW base (and, in the
// extended cases, the LSB/MSB planes) into the 1bpp framebuffer.
//
// The purpose is to catch regressions that the pure mapping unit tests cannot:
// whether glyphs are actually rasterized into the 2-bit frame at all, and
// whether the exported planes put ink where the page renderer put glyphs.
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include "EpdFont.h"
#include "EpdFontFamily.h"
#include "GfxRenderer.h"

namespace {

// 8x8 2-bit glyph for 'A' (ASCII 0x41). Values are glyph-coverage semantics
// (0=white, 1=light gray, 2=dark gray, 3=black) as produced by fontconvert.py
// --2bit (bm>=12 -> 3, >=8 -> 2, >=4 -> 1, else 0). Packed MSB-first, 4
// pixels per byte.
constexpr uint8_t kGlyphABitmap[16] = {
    // row0: ..## ##..
    0x0F, 0xF0,
    // row1: .#######.
    0x3F, 0xFC,
    // row2: .##..##.
    0x3C, 0x3C,
    // row3: .##..##.
    0x3C, 0x3C,
    // row4: .######.
    0x3F, 0xFC,
    // row5: .##..##.
    0x3C, 0x3C,
    // row6: .##..##.
    0x3C, 0x3C,
    // row7: ........
    0x00, 0x00,
};

EpdFontData makeTestFontData() {
  static EpdGlyph glyphA{};
  glyphA.width = 8;
  glyphA.height = 8;
  glyphA.advanceX = 8 << 4;  // 12.4 fixed point
  glyphA.left = 0;
  glyphA.top = 8;
  glyphA.dataLength = sizeof(kGlyphABitmap);
  glyphA.dataOffset = 0;

  static EpdUnicodeInterval intervalA{};
  intervalA.first = 0x41;  // 'A'
  intervalA.last = 0x41;
  intervalA.offset = 0;

  EpdFontData data{};
  data.bitmap = kGlyphABitmap;
  data.glyph = &glyphA;
  data.intervals = &intervalA;
  data.intervalCount = 1;
  data.advanceY = 8;
  data.ascender = 8;
  data.descender = 0;
  data.is2Bit = true;
  data.groups = nullptr;
  data.groupCount = 0;
  data.glyphToGroup = nullptr;
  data.kernLeftClasses = nullptr;
  data.kernRightClasses = nullptr;
  data.kernLeftCodepoints = nullptr;
  data.kernLeftClassIds = nullptr;
  data.kernRightCodepoints = nullptr;
  data.kernRightClassIds = nullptr;
  data.kernMatrix = nullptr;
  data.kernRowOffsets = nullptr;
  data.kernSparseCols = nullptr;
  data.kernSparseValues = nullptr;
  data.kernLeftEntryCount = 0;
  data.kernRightEntryCount = 0;
  data.kernLeftClassCount = 0;
  data.kernRightClassCount = 0;
  data.ligaturePairs = nullptr;
  data.ligaturePairCount = 0;
  data.glyphMissHandler = nullptr;
  data.glyphMissCtx = nullptr;
  data.coverageHandler = nullptr;
  return data;
}

bool frameHasInk(const uint8_t* fb, size_t bytes) {
  for (size_t i = 0; i < bytes; i++) {
    if (fb[i] != 0xFF) return true;  // 1bpp framebuffer starts all-white (0xFF)
  }
  return false;
}

bool twoBitFrameHasInk(const uint8_t* frame, size_t bytes) {
  for (size_t i = 0; i < bytes; i++) {
    if (frame[i] != 0x00) return true;  // cleared to 0 (white) before render
  }
  return false;
}

}  // namespace

// The core regression: with a 2-bit target active, drawText must rasterize
// glyphs into that frame, and exportGrayFrameToBw must put ink in the 1bpp
// framebuffer. A blank 2-bit frame (or blank export) means "AA text invisible"
// on the X4 Pro.
TEST(GfxRenderIntegration, SinglePassTwoBitDrawTextExportsInk) {
  HalDisplay display;
  GfxRenderer renderer(display);
  renderer.begin();
  renderer.setOrientation(GfxRenderer::Portrait);

  EpdFontData data = makeTestFontData();
  EpdFont font(&data);
  EpdFontFamily family(&font);
  renderer.insertFont(1, family);

  const uint32_t frameBytes = GfxRenderer::gray2BitRowBytes(800) * 480;  // 200 * 480 = 96,000
  std::vector<uint8_t> gray2Bit(frameBytes, 0x00);

  // Mirror of the single-pass branch in renderContents.
  renderer.begin2BitTarget(gray2Bit.data());
  renderer.clear2BitTarget(0x00);
  renderer.setRenderMode(GfxRenderer::BW);
  renderer.drawText(1, 100, 100, "A");
  renderer.end2BitTarget();

  // 1) The 2-bit frame must actually contain the glyph.
  EXPECT_TRUE(twoBitFrameHasInk(gray2Bit.data(), frameBytes))
      << "AA 2-bit frame is blank: glyphs were not rasterized into the frame";

  // 2) Exporting the BW base must put ink into the 1bpp framebuffer.
  renderer.setRenderMode(GfxRenderer::BW);
  renderer.exportGrayFrameToBw();
  EXPECT_TRUE(frameHasInk(display.getFrameBuffer(), HalDisplay::BUFFER_SIZE))
      << "exportGrayFrameToBw produced a blank framebuffer: AA text would be invisible";
}

// Control: the same glyph drawn through the classic B/W path must also ink the
// framebuffer. If this passes while the 2-bit test above fails, the regression
// is specific to the single-pass path.
TEST(GfxRenderIntegration, ClassicBwDrawTextInksFramebuffer) {
  HalDisplay display;
  GfxRenderer renderer(display);
  renderer.begin();
  renderer.setOrientation(GfxRenderer::Portrait);

  EpdFontData data = makeTestFontData();
  EpdFont font(&data);
  EpdFontFamily family(&font);
  renderer.insertFont(1, family);

  renderer.setRenderMode(GfxRenderer::BW);
  renderer.drawText(1, 100, 100, "A");
  EXPECT_TRUE(frameHasInk(display.getFrameBuffer(), HalDisplay::BUFFER_SIZE))
      << "Classic B/W drawText produced a blank framebuffer (test fixture issue)";
}

// The two gray planes must be exported from the same 2-bit frame and be
// pushed to the controller exactly like the classic per-mode render:
// LSB gets value==1 pixels (light gray), MSB gets value==1||2 (light+dark).
TEST(GfxRenderIntegration, SinglePassExportsLsbAndMsbPlanes) {
  HalDisplay display;
  GfxRenderer renderer(display);
  renderer.begin();
  renderer.setOrientation(GfxRenderer::Portrait);

  EpdFontData data = makeTestFontData();
  EpdFont font(&data);
  EpdFontFamily family(&font);
  renderer.insertFont(1, family);

  const uint32_t frameBytes = GfxRenderer::gray2BitRowBytes(800) * 480;
  std::vector<uint8_t> gray2Bit(frameBytes, 0x00);

  renderer.begin2BitTarget(gray2Bit.data());
  renderer.clear2BitTarget(0x00);
  renderer.setRenderMode(GfxRenderer::BW);
  renderer.drawText(1, 100, 100, "A");
  renderer.end2BitTarget();

  renderer.setRenderMode(GfxRenderer::GRAYSCALE_LSB);
  renderer.exportGrayFrameToLsb();
  renderer.copyGrayscaleLsbBuffers();
  EXPECT_TRUE(frameHasInk(display.grayscaleLsb(), HalDisplay::BUFFER_SIZE))
      << "LSB plane is blank: light-gray AA pixels missing";

  renderer.setRenderMode(GfxRenderer::GRAYSCALE_MSB);
  renderer.exportGrayFrameToMsb();
  renderer.copyGrayscaleMsbBuffers();
  EXPECT_TRUE(frameHasInk(display.grayscaleMsb(), HalDisplay::BUFFER_SIZE))
      << "MSB plane is blank: dark-gray AA pixels missing";
}
