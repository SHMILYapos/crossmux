#include <gtest/gtest.h>

#include "DirectPixelWriter.h"
#include "GfxRenderer.h"

namespace {

bool mappedBit(const GfxRenderer::RenderMode mode, const GfxRenderer::TwoBitPixel pixel) {
  return pixel.draw && !GfxRenderer::framebufferState(mode, pixel.state);
}

bool directBit(const GfxRenderer::RenderMode mode, const uint8_t value) {
  uint8_t framebuffer = 0;
  DirectPixelWriter writer{};
  writer.fb = &framebuffer;
  writer.mode = mode;
  writer.displayWidthBytes = 1;
  writer.originY = 0;
  writer.clipRows = 1;
  writer.phyXBase = 0;
  writer.phyYBase = 0;
  writer.phyXStepX = 1;
  writer.phyYStepX = 0;
  writer.phyXStepY = 0;
  writer.phyYStepY = 1;
  writer.beginRow(0);
  writer.writePixel(0, value);
  return (framebuffer & 0x80) != 0;
}

}  // namespace

#if FREEINK_DEVICE_EEGO_A4
TEST(GfxTwoBitMapping, A4DirectGlyphAndBitmapProduceTheSameFramebufferBit) {
  for (const auto mode : {GfxRenderer::GRAYSCALE_MSB, GfxRenderer::GRAYSCALE_LSB}) {
    for (uint8_t value = 0; value < 4; ++value) {
      const bool direct = directBit(mode, value);
      const bool glyph = mappedBit(mode, GfxRenderer::mapTwoBitGlyphCoverage(mode, 3 - value));
      const bool bitmap = mappedBit(mode, GfxRenderer::mapTwoBitPixel(mode, value));
      EXPECT_EQ(direct, glyph) << "mode=" << mode << " value=" << static_cast<int>(value);
      EXPECT_EQ(direct, bitmap) << "mode=" << mode << " value=" << static_cast<int>(value);
    }
  }
}

#endif  // FREEINK_DEVICE_EEGO_A4

TEST(GfxTwoBitMapping, BwKeepsLogicalPixelState) {
  for (uint8_t value = 0; value < 4; ++value) {
    const auto pixel = GfxRenderer::mapTwoBitPixel(GfxRenderer::BW, value);
    EXPECT_EQ(pixel.draw, value < 3);
    EXPECT_TRUE(GfxRenderer::framebufferState(GfxRenderer::BW, pixel.state));
  }
}

// Single-pass 2-bit frame export must produce the same framebuffer bits as the
// classic three-pass render (BW base + LSB/MSB planes). The export mapping
// (grayFrameExportBit, values 0=white..3=black) is the inverse read of
// mapTwoBitPixel (values 0=black..3=white), and the bit semantics line up:
// BW clears the bit (ink), LSB/MSB set it.
TEST(GfxTwoBitMapping, SinglePassGrayExportMatchesThreePassMapping) {
  for (const auto mode :
       {GfxRenderer::BW, GfxRenderer::GRAYSCALE_LSB, GfxRenderer::GRAYSCALE_MSB}) {
    for (uint8_t frameValue = 0; frameValue < 4; ++frameValue) {
      const bool exportParticipates = GfxRenderer::grayFrameExportBit(mode, frameValue);
      const auto pixel = GfxRenderer::mapTwoBitPixel(mode, static_cast<uint8_t>(3 - frameValue));
      EXPECT_EQ(exportParticipates, pixel.draw)
          << "mode=" << static_cast<int>(mode) << " frameValue=" << static_cast<int>(frameValue);
      // Framebuffer bit written by the export: BW clears (ink), LSB/MSB set.
      // The per-mode render writes via drawPixel(x, y, state):
      //   eff = framebufferState(mode, state); eff -> clear bit; !eff -> set bit.
      const bool exportWritesOne = exportParticipates && (mode != GfxRenderer::BW);
      const bool renderWritesOne =
          pixel.draw && !GfxRenderer::framebufferState(mode, pixel.state);
      EXPECT_EQ(exportWritesOne, renderWritesOne)
          << "mode=" << static_cast<int>(mode) << " frameValue=" << static_cast<int>(frameValue);
    }
  }
}

// A full 2-bit frame export, simulated at byte level: for a frame of 4 pixels
// with known values, the exported plane must equal the per-pixel mapping
// applied to the same values.
TEST(GfxTwoBitMapping, ExportFramePacksPixelsLikePerPixelMapping) {
  // 4 pixels in one byte of the 2-bit frame: [0,1,2,3] as raw values.
  const uint8_t frameByte = static_cast<uint8_t>((0u << 6) | (1u << 4) | (2u << 2) | (3u << 0));
  for (const auto mode :
       {GfxRenderer::BW, GfxRenderer::GRAYSCALE_LSB, GfxRenderer::GRAYSCALE_MSB}) {
    uint8_t plane = 0;
    for (uint8_t px = 0; px < 4; ++px) {
      const uint8_t value = static_cast<uint8_t>((frameByte >> ((3 - px) * 2)) & 0x3);
      if (GfxRenderer::grayFrameExportBit(mode, value)) {
        if (mode == GfxRenderer::BW) {
          plane &= static_cast<uint8_t>(~(0x80 >> px));  // ink: clear bit
        } else {
          plane |= static_cast<uint8_t>(0x80 >> px);  // plane member: set bit
        }
      }
    }
    // Reference: same values through mapTwoBitPixel with the render's drawPixel
    // bit convention (framebuffer starts 0xFF for BW-base-like context; here we
    // only compare participation + bit direction per pixel).
    uint8_t ref = 0;
    for (uint8_t px = 0; px < 4; ++px) {
      const uint8_t value = static_cast<uint8_t>((frameByte >> ((3 - px) * 2)) & 0x3);
      const auto pixel = GfxRenderer::mapTwoBitPixel(mode, static_cast<uint8_t>(3 - value));
      if (!pixel.draw) continue;
      if (GfxRenderer::framebufferState(mode, pixel.state)) {
        ref &= static_cast<uint8_t>(~(0x80 >> px));  // clear
      } else {
        ref |= static_cast<uint8_t>(0x80 >> px);  // set
      }
    }
    EXPECT_EQ(plane, ref) << "mode=" << static_cast<int>(mode);
  }
}

// The 256-entry export mask tables used by exportGrayFrameTo* must agree with
// the per-pixel mapping (grayFrameExportBit) for every possible frame byte.
// Reconstructed here from the same formula: for each 2-bit byte, bits 7..4
// mark which of its four pixels participate in each plane.
TEST(GfxTwoBitMapping, ExportMaskTablesMatchPerPixelMapping) {
  struct Masks { uint8_t bw, lsb, msb; };
  Masks table[256];
  for (int b = 0; b < 256; ++b) {
    uint8_t bw = 0, lsb = 0, msb = 0;
    for (int px = 0; px < 4; ++px) {
      const uint8_t c = static_cast<uint8_t>((b >> ((3 - px) * 2)) & 0x3);
      if (GfxRenderer::grayFrameExportBit(GfxRenderer::BW, c)) bw |= static_cast<uint8_t>(0x80 >> px);
      if (GfxRenderer::grayFrameExportBit(GfxRenderer::GRAYSCALE_LSB, c)) lsb |= static_cast<uint8_t>(0x80 >> px);
      if (GfxRenderer::grayFrameExportBit(GfxRenderer::GRAYSCALE_MSB, c)) msb |= static_cast<uint8_t>(0x80 >> px);
    }
    // Reference: per-pixel draw flag through the classic mapping.
    uint8_t bwRef = 0, lsbRef = 0, msbRef = 0;
    for (int px = 0; px < 4; ++px) {
      const uint8_t c = static_cast<uint8_t>((b >> ((3 - px) * 2)) & 0x3);
      if (GfxRenderer::mapTwoBitPixel(GfxRenderer::BW, static_cast<uint8_t>(3 - c)).draw) {
        bwRef |= static_cast<uint8_t>(0x80 >> px);
      }
      if (GfxRenderer::mapTwoBitPixel(GfxRenderer::GRAYSCALE_LSB, static_cast<uint8_t>(3 - c)).draw) {
        lsbRef |= static_cast<uint8_t>(0x80 >> px);
      }
      if (GfxRenderer::mapTwoBitPixel(GfxRenderer::GRAYSCALE_MSB, static_cast<uint8_t>(3 - c)).draw) {
        msbRef |= static_cast<uint8_t>(0x80 >> px);
      }
    }
    EXPECT_EQ(bw, bwRef) << "byte=" << b;
    EXPECT_EQ(lsb, lsbRef) << "byte=" << b;
    EXPECT_EQ(msb, msbRef) << "byte=" << b;
  }
}
