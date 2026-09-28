#pragma once
//
// vgiv_plugin_common.h - tiny shared helpers for vgiv image plugins.
// Header-only for allocImage/makeError (both are actually called from
// every plugin's own TU, so they get emitted); vgiv_plugin_free_image/
// vgiv_plugin_free_error are declared here but defined in
// vgiv_plugin_common.cpp (compiled into each plugin target) - an `inline`
// extern "C" definition that's never ODR-used within its own TU is not
// guaranteed to be emitted at all, which silently dropped both symbols
// from every plugin's dynamic symbol table the first time this was tried.
//
#include "vgiv_plugin.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace vgiv_plugin
{

inline VgivPluginImage* allocImage(int width, int height)
{
    auto* img = static_cast<VgivPluginImage*>(std::malloc(sizeof(VgivPluginImage)));
    img->width = width;
    img->height = height;
    img->rgba = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(width) * height * 4));
    img->orientation = 1;
    img->sampleType = VGIV_SAMPLE_NONE;
    img->samples = nullptr;
    return img;
}

inline size_t sampleTypeSize(VgivSampleType type)
{
    switch (type)
    {
        case VGIV_SAMPLE_U8:
            return 1;
        case VGIV_SAMPLE_U16:
            return 2;
        case VGIV_SAMPLE_FLOAT:
            return 4;
        default:
            return 0;
    }
}

// Allocates and attaches a raw single-channel sample buffer to `img`
// (already created via allocImage). Called by plugins that decode
// genuinely single-channel/grayscale source data.
inline void allocSamples(VgivPluginImage* img, VgivSampleType type)
{
    img->sampleType = type;
    img->samples =
        std::malloc(static_cast<size_t>(img->width) * img->height * sampleTypeSize(type));
}

inline char* makeError(const char* fmt, const char* arg)
{
    char buf[1024];
    std::snprintf(buf, sizeof(buf), fmt, arg);
    return strdup(buf);
}

} // namespace vgiv_plugin
