// Host-test stub for the framebuffer loan registry. The integration test never
// lends the framebuffer (no memory-hungry chapter builds), so no-ops are safe.
#include <cstddef>
#include <cstdint>

namespace buildscratch {

void lend(uint8_t* /*buf*/, size_t /*len*/) {}
void reclaim() {}
uint8_t* claim(size_t /*minLen*/, size_t* lenOut) {
  if (lenOut) *lenOut = 0;
  return nullptr;
}
void release(const uint8_t* /*p*/) {}

}  // namespace buildscratch
