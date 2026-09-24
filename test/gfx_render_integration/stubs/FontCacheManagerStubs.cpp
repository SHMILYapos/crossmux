// Host-test stubs for FontCacheManager methods GfxRenderer links against. The
// integration test leaves renderer.getFontCacheManager() null, so the scan
// paths are never entered.
#include "FontCacheManager.h"

bool FontCacheManager::isScanning() const { return false; }

void FontCacheManager::recordText(const char* /*text*/, int /*fontId*/, EpdFontFamily::Style /*style*/) {}
