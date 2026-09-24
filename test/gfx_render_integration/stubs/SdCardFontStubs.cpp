// Host-test stubs for the SdCardFont methods that GfxRenderer links against.
// The integration test only exercises built-in (RAM) fonts, so every SD-font
// path returns its safe default: no glyph, no advance, no overflow bitmap.
#include "SdCardFont.h"

#include <deque>
#include <string>

int SdCardFont::buildAdvanceTable(const char* /*utf8Text*/, uint8_t /*styleMask*/, const char* /*extraText*/) {
  return 0;
}

int SdCardFont::buildAdvanceTable(const std::deque<std::string>& /*words*/, bool /*includeHyphen*/,
                                  uint8_t /*styleMask*/, const char* /*extraText*/) {
  return 0;
}

uint16_t SdCardFont::getAdvanceOrLoad(uint32_t /*codepoint*/, uint8_t /*style*/) const { return 0; }

bool SdCardFont::isOverflowGlyph(const EpdGlyph* /*glyph*/) const { return false; }

const uint8_t* SdCardFont::getOverflowBitmap(const EpdGlyph* /*glyph*/) const { return nullptr; }

SdCardFont* SdCardFont::fromMissCtx(void* /*ctx*/) { return nullptr; }

int SdCardFont::prewarm(const char* /*utf8Text*/, uint8_t /*styleMask*/, bool /*metadataOnly*/,
                        bool /*loadKernLig*/) {
  return 0;
}

int SdCardFont::prewarm(TextGetter /*getter*/, const void* /*ctx*/, uint32_t /*textCount*/, uint8_t /*styleMask*/,
                        bool /*metadataOnly*/, bool /*loadKernLig*/) {
  return 0;
}

uint16_t SdCardFont::getAdvance(uint32_t /*codepoint*/, uint8_t /*style*/) const { return 0; }

bool SdCardFont::hasAdvanceTable() const { return false; }

uint8_t SdCardFont::resolveStyle(uint8_t style) const { return style; }
