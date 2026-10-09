# Image-format plugins for vgiv, loaded at runtime via dlopen (see
# ImagePluginHost) - never linked into the `vgiv` executable itself.
# Each plugin is built as a bare-named shared library (no "lib" prefix)
# so ImagePluginHost's sorted directory scan sees plain format names.

function(vgiv_add_plugin name)
  add_library(vgiv_plugin_${name} SHARED ${ARGN} ${CMAKE_CURRENT_SOURCE_DIR}/vgiv_plugin_common.cpp)
  set_target_properties(vgiv_plugin_${name} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "$<IF:$<BOOL:${WIN32}>,vgiv_${name},${name}>"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
  )
  target_include_directories(vgiv_plugin_${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  install(TARGETS vgiv_plugin_${name} LIBRARY DESTINATION lib/vgiv/plugins RUNTIME DESTINATION lib/vgiv/plugins)
endfunction()

vgiv_add_plugin(stbimage stbimage/stbimage_plugin.cpp)
target_include_directories(vgiv_plugin_stbimage PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# Takes over JPEG decoding from stbimage (see jpegturbo_plugin.cpp's doc
# comment for why: SIMD-accelerated decode matters for $image cycling's
# decode-on-demand path) - "jpegturbo" sorts before "stbimage" in
# ImagePluginHost's first-match-wins dispatch, so this one always wins.
find_package(JPEG REQUIRED)
vgiv_add_plugin(jpegturbo jpegturbo/jpegturbo_plugin.cpp)
target_link_libraries(vgiv_plugin_jpegturbo PRIVATE JPEG::JPEG)

vgiv_add_plugin(pgm pgm/pgm_plugin.cpp)

find_package(TIFF REQUIRED)
vgiv_add_plugin(tiff tiff/tiff_plugin.cpp)
target_link_libraries(vgiv_plugin_tiff PRIVATE TIFF::TIFF)

find_package(WebP CONFIG REQUIRED)
vgiv_add_plugin(webp webp/webp_plugin.cpp)
target_link_libraries(vgiv_plugin_webp PRIVATE WebP::webp)

vgiv_add_plugin(npy npy/npy_plugin.cpp)

vgiv_add_plugin(dicom dicom/dicom_plugin.cpp)

# cfitsio ships a CMake config (CFITSIO::cfitsio) via vcpkg, but Fedora's
# cfitsio-devel package only provides a pkg-config file - fall back to that
# when the CMake config isn't found (e.g. building against the system lib).
find_package(cfitsio CONFIG QUIET)
if(NOT cfitsio_FOUND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CFITSIO REQUIRED IMPORTED_TARGET cfitsio)
endif()
vgiv_add_plugin(fits fits/fits_plugin.cpp)
target_link_libraries(vgiv_plugin_fits PRIVATE $<IF:$<TARGET_EXISTS:CFITSIO::cfitsio>,CFITSIO::cfitsio,PkgConfig::CFITSIO>)
