// Host-test stubs for the Bitmap methods GfxRenderer links against. The
// integration test never renders storage-backed images, so these return the
// no-data defaults (never invoked on the tested paths).
#include "Bitmap.h"

bool Bitmap::ensureDrawScratch(size_t /*bytes*/) const { return false; }

BmpReaderError Bitmap::readNextRow(uint8_t* /*data*/, uint8_t* /*rowBuffer*/, uint8_t* /*opacityRow*/) const {
  return BmpReaderError::Ok;
}
