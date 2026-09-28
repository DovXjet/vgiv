#include "vgiv_plugin.h"

#include <cstdlib>

extern "C" void vgiv_plugin_free_image(VgivPluginImage* img)
{
    if (!img)
        return;
    std::free(img->rgba);
    std::free(img->samples);
    std::free(img);
}

extern "C" void vgiv_plugin_free_error(char* error_msg)
{
    std::free(error_msg);
}
