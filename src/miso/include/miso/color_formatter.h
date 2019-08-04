#ifndef MISO_CORE_COLOR_FORMATTER_H_
#define MISO_CORE_COLOR_FORMATTER_H_

#include <stdint.h>
#include <string>
#include <algorithm>

#include "miso/color_container.h"
#include "miso/string.h"

namespace miso {

class ColorFormatter {
public:
    static std::string ToHexString(const Rgba& rgba, size_t digit);
    static std::string ToColorSpaceString(const Rgba& rgba, ColorSpace colorspace, bool alpha, bool percent);
};

inline std::string
ColorFormatter::ToHexString(const Rgba& rgba, size_t digit)
{
    auto r = static_cast<uint8_t>(rgba.R * 255.0f + 0.5f);
    auto g = static_cast<uint8_t>(rgba.G * 255.0f + 0.5f);
    auto b = static_cast<uint8_t>(rgba.B * 255.0f + 0.5f);
    auto a = static_cast<uint8_t>(rgba.A * 255.0f + 0.5f);
    if (digit == 3) {
        return StringUtils::Format("#%1x%1x%1x", r / 16, g / 16, b / 16);
    } else if (digit == 4) {
        return StringUtils::Format("#%1x%1x%1x%1x", r / 16, g / 16, b / 16, a / 16);
    } else if (digit == 6) {
        return StringUtils::Format("#%02x%02x%02x", r, g, b);
    } else if (digit == 8) {
        return StringUtils::Format("#%02x%02x%02x%02x", r, g, b, a);
    } else {
        return "";
    }
}

inline std::string
ColorFormatter::ToColorSpaceString(const Rgba& rgba, ColorSpace colorspace, bool alpha, bool percent)
{
    auto format = "";
    auto prefix = "";

    float fv[4] = {};
    if (colorspace == ColorSpace::Rgb) {
        prefix = "rgb";
        fv[0] = rgba.R; fv[1] = rgba.G; fv[2] = rgba.G; fv[3] = rgba.A;
    } else if (colorspace == ColorSpace::Hsl) {
        prefix = "hsl";
        Hsla hsla(rgba);
        fv[0] = hsla.H; fv[1] = hsla.S; fv[2] = hsla.L; fv[3] = hsla.A;
    } else if (colorspace == ColorSpace::Hsv) {
        prefix = "hsv";
        Hsva hsva(rgba);
        fv[0] = hsva.H; fv[1] = hsva.S; fv[2] = hsva.V; fv[3] = hsva.A;
    } else {
        return "";
    }

    auto max = 0.0f;
    if (percent) {
        max = 100.0f;
        format = alpha ? "%sa(%d%%,%d%%,%d%%,%d%%)" : "%s(%d%%,%d%%,%d%%)";
    } else {
        max = 255.0f;
        format = alpha ? "%sa(%d,%d,%d,%d)" : "%s(%d,%d,%d)";
    }

    uint8_t v[4] = {};
    v[0] = static_cast<uint8_t>(fv[0] * max + 0.5f);
    v[1] = static_cast<uint8_t>(fv[1] * max + 0.5f);
    v[2] = static_cast<uint8_t>(fv[2] * max + 0.5f);
    v[3] = static_cast<uint8_t>(fv[3] * max + 0.5f);

    return alpha ?
        StringUtils::Format(format, prefix, v[0], v[1], v[2], v[3]) :
        StringUtils::Format(format, prefix, v[0], v[1], v[2]);
}

} // namespace miso

#endif // MISO_CORE_COLOR_FORMATTER_H_
