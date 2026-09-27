#pragma once

// Minimal Arduino/ESP compatibility for the host test: millis(), assert, and
// the ESP heap shim that GfxRenderer.cpp references. GfxRenderer.cpp does not
// include Arduino.h on device; these symbols come from the platform build.
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cmath>

inline uint32_t millis() {
  return static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count() / 1000);
}

struct EspClass {
  size_t getFreeHeap() const { return 4u * 1024 * 1024; }
  size_t getMaxAllocHeap() const { return 2u * 1024 * 1024; }
  size_t getFreePsram() const { return 4u * 1024 * 1024; }
  size_t getMaxAllocPsram() const { return 2u * 1024 * 1024; }
  void restart() {}
};

static EspClass ESP;
