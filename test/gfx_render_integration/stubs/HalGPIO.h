#pragma once

// Host-test stub: GfxRenderer.cpp includes <HalGPIO.h> but never calls into it
// (the reader path is display/font only). Provide the type so the TU compiles.
class HalGPIO {
 public:
  static HalGPIO& instance();
};

// Minimal Logging replacement used by the host test build.
