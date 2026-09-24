#pragma once

#include <cstdint>
#include <cstring>

// Host test mock of the firmware HalDisplay: owns a real 1bpp framebuffer in
// the same geometry as the X4 Pro panel (800x480) and records the grayscale
// planes that were pushed. Only the calls GfxRenderer makes are implemented;
// SDL/controller details are irrelevant here.
class HalDisplay {
 public:
  enum RefreshMode { FULL_REFRESH, HALF_REFRESH, FAST_REFRESH };

  static constexpr uint16_t DISPLAY_WIDTH = 800;
  static constexpr uint16_t DISPLAY_HEIGHT = 480;
  static constexpr uint16_t DISPLAY_WIDTH_BYTES = DISPLAY_WIDTH / 8;
  static constexpr uint32_t BUFFER_SIZE = DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT;

  HalDisplay() { std::memset(frameBufferStorage_, 0xFF, sizeof(frameBufferStorage_)); }

  uint8_t* getFrameBuffer() const { return frameBufferStorage_; }
  uint8_t* lendFrameBufferStorage(uint32_t* sizeOut) {
    *sizeOut = sizeof(frameBufferStorage_);
    return frameBufferStorage_;
  }
  void returnFrameBufferStorage() {}

  uint16_t getDisplayWidth() const { return DISPLAY_WIDTH; }
  uint16_t getDisplayHeight() const { return DISPLAY_HEIGHT; }
  uint16_t getDisplayWidthBytes() const { return DISPLAY_WIDTH_BYTES; }
  uint32_t getBufferSize() const { return BUFFER_SIZE; }

  void clearScreen(uint8_t color = 0xFF) const { std::memset(frameBufferStorage_, color, sizeof(frameBufferStorage_)); }
  void drawImage(const uint8_t* /*imageData*/, uint16_t /*x*/, uint16_t /*y*/, uint16_t /*w*/, uint16_t /*h*/,
                 bool /*fromProgmem*/ = false) const {}
  void drawImageTransparent(const uint8_t* /*imageData*/, uint16_t /*x*/, uint16_t /*y*/, uint16_t /*w*/,
                            uint16_t /*h*/, bool /*fromProgmem*/ = false) const {}

  bool isInverted() const { return false; }

  void displayBuffer(RefreshMode /*mode*/ = FAST_REFRESH, bool /*turnOffScreen*/ = false) {}
  void displayBufferAsync(RefreshMode /*mode*/ = FAST_REFRESH) {}
  void waitRefreshComplete() {}
  bool supportsAsyncRefresh() const { return false; }
  void displayGrayscaleBase(RefreshMode /*fallback*/ = HALF_REFRESH, bool /*turnOffScreen*/ = false) {}
  void preconditionGrayscale() {}
  void preconditionGrayscale(uint16_t /*x*/, uint16_t /*y*/, uint16_t /*w*/, uint16_t /*h*/) {}
  void displayGrayBuffer(bool /*turnOffScreen*/ = false, const unsigned char* /*lut*/ = nullptr,
                         bool /*factoryMode*/ = false) {}
  void writeGrayscalePlaneStrip(bool /*lsbPlane*/, const uint8_t* /*scratch*/, uint16_t /*yStart*/,
                                uint16_t /*numRows*/) {}
  bool supportsStripGrayscale() const { return false; }
  bool combinesGrayscaleBase() const { return false; }

  void copyGrayscaleLsbBuffers(const uint8_t* lsbBuffer) {
    std::memcpy(grayscaleLsb_, lsbBuffer, BUFFER_SIZE);
  }
  void copyGrayscaleMsbBuffers(const uint8_t* msbBuffer) {
    std::memcpy(grayscaleMsb_, msbBuffer, BUFFER_SIZE);
  }
  void cleanupGrayscaleBuffers(const uint8_t* bwBuffer) {
    std::memcpy(lastCleanedBw_, bwBuffer, BUFFER_SIZE);
  }

  // Test introspection.
  const uint8_t* grayscaleLsb() const { return grayscaleLsb_; }
  const uint8_t* grayscaleMsb() const { return grayscaleMsb_; }
  const uint8_t* lastCleanedBw() const { return lastCleanedBw_; }

 private:
  mutable uint8_t frameBufferStorage_[BUFFER_SIZE];
  uint8_t grayscaleLsb_[BUFFER_SIZE];
  uint8_t grayscaleMsb_[BUFFER_SIZE];
  uint8_t lastCleanedBw_[BUFFER_SIZE];
};
