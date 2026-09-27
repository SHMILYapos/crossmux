#pragma once

// Host-test stub replacing lib/EpdFont/FontDecompressor.h (which drags in the
// third-party uzlib headers). The integration test only exercises built-in
// uncompressed fonts, so getGlyphBitmap() never reaches the decompressor.
#include <cstdint>

#include "EpdFontData.h"

class FontDecompressor {
 public:
  const uint8_t* getBitmap(const EpdFontData* /*fontData*/, const EpdGlyph* /*glyph*/,
                           uint32_t /*glyphIndex*/) {
    return nullptr;
  }
};
