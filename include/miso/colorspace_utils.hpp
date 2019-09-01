#ifndef MISO_COLORSPACE_UTILS_HPP_
#define MISO_COLORSPACE_UTILS_HPP_

#include "miso/common.hpp"

namespace miso {

enum class ColorSpace { Invalid, Rgb, Hsl, Hsv };

class ColorSpaceUtils {
public:
    ColorSpaceUtils() = delete;
    ColorSpaceUtils(const ColorSpaceUtils&) = delete;
    ColorSpaceUtils(ColorSpaceUtils&&) = delete;

    // HSL
    static void HslToRgb(float h, float s, float l, float* r_out, float* g_out, float* b_out);
    static void RgbToHsl(float r, float g, float b, float* h_out, float* s_out, float* l_out);

    // HSV
    static void HsvToRgb(float h, float s, float v, float* r_out, float* g_out, float* b_out);
    static void RgbToHsv(float r, float g, float b, float* h_out, float* s_out, float* v_out);
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "colorspace_utils.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_COLORSPACE_UTILS_HPP_
