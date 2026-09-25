//
// webp_plugin.cpp - vgiv image plugin for WebP, via libwebp. Modeled on
// giv's ~/hd/github/giv/src/plugins/webp.cc: read the whole file, decode
// straight into the pre-allocated destination buffer with
// WebPDecodeRGBAInto.
//
#include "../vgiv_plugin_common.h"

#include <webp/decode.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{

bool hasExtension(const std::string& filename, const char* ext)
{
    size_t n = std::strlen(ext);
    if (filename.size() < n)
        return false;
    return std::equal(filename.end() - n, filename.end(), ext, [](char a, char b)
                       { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

} // namespace

extern "C" bool vgiv_plugin_supports_file(const char* filename)
{
    return hasExtension(std::string(filename), ".webp");
}

extern "C" VgivPluginImage* vgiv_plugin_load_image(const char* filename, char** error_msg)
{
    FILE* fh = std::fopen(filename, "rb");
    if (!fh)
    {
        if (error_msg)
            *error_msg = vgiv_plugin::makeError("webp: failed to open %s", filename);
        return nullptr;
    }
    std::fseek(fh, 0, SEEK_END);
    long size = std::ftell(fh);
    std::fseek(fh, 0, SEEK_SET);
    if (size <= 0)
    {
        std::fclose(fh);
        if (error_msg)
            *error_msg = vgiv_plugin::makeError("webp: empty file %s", filename);
        return nullptr;
    }
    std::vector<unsigned char> data(static_cast<size_t>(size));
    size_t nread = std::fread(data.data(), 1, data.size(), fh);
    std::fclose(fh);
    if (nread != data.size())
    {
        if (error_msg)
            *error_msg = vgiv_plugin::makeError("webp: failed to read %s", filename);
        return nullptr;
    }

    int width = 0, height = 0;
    if (!WebPGetInfo(data.data(), data.size(), &width, &height))
    {
        if (error_msg)
            *error_msg = vgiv_plugin::makeError("webp: not a valid WebP file: %s", filename);
        return nullptr;
    }

    VgivPluginImage* img = vgiv_plugin::allocImage(width, height);
    size_t stride = static_cast<size_t>(width) * 4;
    unsigned char* decoded =
        WebPDecodeRGBAInto(data.data(), data.size(), img->rgba, stride * height, static_cast<int>(stride));
    if (!decoded)
    {
        vgiv_plugin_free_image(img);
        if (error_msg)
            *error_msg = vgiv_plugin::makeError("webp: failed to decode %s", filename);
        return nullptr;
    }

    return img;
}
