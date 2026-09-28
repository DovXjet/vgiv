#pragma once
//
// vgiv_plugin.h - the C ABI every vgiv image-format plugin (.so) must
// export. Mirrors giv's own plugin ABI (~/hd/github/giv/src/givplugin.h,
// givplugin.cc - the real, load-bearing symbol names there are
// giv_plugin_supports_file / giv_plugin_load_file). Every plugin normalizes
// an 8-bit RGBA preview at load time (used directly for color images, and
// as the fallback for formats with no raw-sample path below). Formats that
// carry real single-channel dynamic range (grayscale PGM/TIFF/16-bit PNG)
// additionally populate an optional raw sample buffer, which is what the
// Contrast/Color Table tools operate on (see DisplayImage.h) - RGBA8 alone
// would quantize away the dynamic range those tools are stretching.
//
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum VgivSampleType
    {
        VGIV_SAMPLE_NONE = 0, // no raw sample data; rgba is the only representation
        VGIV_SAMPLE_U8,
        VGIV_SAMPLE_U16,
        VGIV_SAMPLE_FLOAT
    } VgivSampleType;

    typedef struct VgivPluginImage
    {
        int width;
        int height;
        unsigned char* rgba; // width*height*4 bytes, top-to-bottom row order

        // EXIF/TIFF Orientation tag value (1-8, see the Exif spec's table),
        // or 1 ("normal", no transform) if the plugin doesn't parse one.
        // The host applies the corresponding rotation/flip centrally
        // (ImagePluginHost::load()) so every plugin shares one
        // implementation instead of duplicating it; a plugin that already
        // hands back upright pixels (e.g. tiff_plugin.cpp's
        // TIFFReadRGBAImageOriented) should just leave this at 1.
        int orientation;

        // Optional raw single-channel sample buffer: width*height samples,
        // row-major, top-to-bottom, each sampleType wide (1/2/4 bytes).
        // VGIV_SAMPLE_NONE/NULL for plugins/formats with no meaningful
        // dynamic range beyond the RGBA8 preview (e.g. color JPEG/WebP).
        VgivSampleType sampleType;
        void* samples;
    } VgivPluginImage;

    // Does this plugin recognize `filename` (by extension)? Called by the
    // host for every loaded plugin, in sorted-filename order, until one
    // returns true (first-match-wins).
    bool vgiv_plugin_supports_file(const char* filename);

    // Load `filename` into a freshly heap-allocated VgivPluginImage. On
    // failure returns NULL and, if error_msg is non-NULL, sets *error_msg
    // to a freshly heap-allocated message (free with vgiv_plugin_free_error).
    VgivPluginImage* vgiv_plugin_load_image(const char* filename, char** error_msg);

    // Frees an image returned by vgiv_plugin_load_image.
    void vgiv_plugin_free_image(VgivPluginImage* img);

    // Frees an error message set by vgiv_plugin_load_image.
    void vgiv_plugin_free_error(char* error_msg);

#ifdef __cplusplus
}
#endif
