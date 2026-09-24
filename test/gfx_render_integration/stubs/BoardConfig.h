#pragma once

// Host-test stub: the firmware BoardConfig selects the active device profile.
// GfxRenderer only reads BoardConfig::ACTIVE and probes for a viewableInsets
// member; omitting it sends the renderer to its VIEWABLE_MARGIN_* constants,
// which is fine for the host test.
namespace BoardConfig {
struct HostProfile {};
inline constexpr HostProfile ACTIVE{};
}  // namespace BoardConfig
