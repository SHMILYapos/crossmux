#pragma once

// Host-test stub: Bitmap.h holds a HalFile* (storage-backed image loading)
// but the integration test never loads images from storage. A forward
// declaration is enough to compile the pointer member.
class HalFile;
