#pragma once
//
// ImagePluginHost.h - scans a directory for vgiv image-format plugins
// (.so files conforming to src/plugins/vgiv_plugin.h) and dispatches
// image loads to the first plugin that claims a given filename. Mirrors
// giv's own plugin loader (~/hd/github/giv/src/givplugin.cc), but with a
// deterministic (sorted-filename) load/dispatch order - giv's own docs
// (doc/plugins.txt) admit its order is arbitrary.
//
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace giv
{

struct LoadedImage
{
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba; // width*height*4, top-to-bottom row order
};

class ImagePluginHost
{
public:
    // Loads plugins (lazily, once) from VGIV_PLUGIN_DIR (env var override)
    // or the compile-time default plugin directory, then attempts to load
    // `filename` with the first plugin that claims it. Returns nullopt and
    // prints a "vgiv: ..." diagnostic to stderr on failure.
    static std::optional<LoadedImage> load(const std::string& filename);
};

} // namespace giv
