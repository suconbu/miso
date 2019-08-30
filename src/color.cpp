#include "miso/color.hpp"

#include <algorithm>
#include <map>
#include <string>

#include "miso/colorspace_utils.hpp"
#include "miso/numeric.hpp"
#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE const Color&
Color::GetInvalid()
{
    static const Color invalid;
    return invalid;
}

MISO_INLINE Color
Color::FromHsla(float h, float s, float l, float a)
{
    float r, g, b;
    ColorSpaceUtils::HslToRgb(h, s, l, &r, &g, &b);
    return Color(r, g, b, a);
}

MISO_INLINE Color
Color::FromHsva(float h, float s, float v, float a)
{
    float r, g, b;
    ColorSpaceUtils::HsvToRgb(h, s, v, &r, &g, &b);
    return Color(r, g, b, a);
}

MISO_INLINE Color
Color::FromHtmlColorName(const char* name)
{
    static const std::map<std::string, const char*> named_colors = {
        { "black", "#000000" },
        { "silver", "#c0c0c0" },
        { "gray", "#808080" },
        { "white", "#ffffff" },
        { "maroon", "#800000" },
        { "red", "#ff0000" },
        { "purple", "#800080" },
        { "fuchsia", "#ff00ff" },
        { "green", "#008000" },
        { "lime", "#00ff00" },
        { "olive", "#808000" },
        { "yellow", "#ffff00" },
        { "navy", "#000080" },
        { "blue", "#0000ff" },
        { "teal", "#008080" },
        { "aqua", "#00ffff" },
        { "orange", "#ffa500" },
        { "aliceblue", "#f0f8ff" },
        { "antiquewhite", "#faebd7" },
        { "aquamarine", "#7fffd4" },
        { "azure", "#f0ffff" },
        { "beige", "#f5f5dc" },
        { "bisque", "#ffe4c4" },
        { "blanchedalmond", "#ffebcd" },
        { "blueviolet", "#8a2be2" },
        { "brown", "#a52a2a" },
        { "burlywood", "#deb887" },
        { "cadetblue", "#5f9ea0" },
        { "chartreuse", "#7fff00" },
        { "chocolate", "#d2691e" },
        { "coral", "#ff7f50" },
        { "cornflowerblue", "#6495ed" },
        { "cornsilk", "#fff8dc" },
        { "crimson", "#dc143c" },
        { "cyan", "#00ffff" },
        { "darkblue", "#00008b" },
        { "darkcyan", "#008b8b" },
        { "darkgoldenrod", "#b8860b" },
        { "darkgray", "#a9a9a9" },
        { "darkgreen", "#006400" },
        { "darkgrey", "#a9a9a9" },
        { "darkkhaki", "#bdb76b" },
        { "darkmagenta", "#8b008b" },
        { "darkolivegreen", "#556b2f" },
        { "darkorange", "#ff8c00" },
        { "darkorchid", "#9932cc" },
        { "darkred", "#8b0000" },
        { "darksalmon", "#e9967a" },
        { "darkseagreen", "#8fbc8f" },
        { "darkslateblue", "#483d8b" },
        { "darkslategray", "#2f4f4f" },
        { "darkslategrey", "#2f4f4f" },
        { "darkturquoise", "#00ced1" },
        { "darkviolet", "#9400d3" },
        { "deeppink", "#ff1493" },
        { "deepskyblue", "#00bfff" },
        { "dimgray", "#696969" },
        { "dimgrey", "#696969" },
        { "dodgerblue", "#1e90ff" },
        { "firebrick", "#b22222" },
        { "floralwhite", "#fffaf0" },
        { "forestgreen", "#228b22" },
        { "gainsboro", "#dcdcdc" },
        { "ghostwhite", "#f8f8ff" },
        { "gold", "#ffd700" },
        { "goldenrod", "#daa520" },
        { "greenyellow", "#adff2f" },
        { "grey", "#808080" },
        { "honeydew", "#f0fff0" },
        { "hotpink", "#ff69b4" },
        { "indianred", "#cd5c5c" },
        { "indigo", "#4b0082" },
        { "ivory", "#fffff0" },
        { "khaki", "#f0e68c" },
        { "lavender", "#e6e6fa" },
        { "lavenderblush", "#fff0f5" },
        { "lawngreen", "#7cfc00" },
        { "lemonchiffon", "#fffacd" },
        { "lightblue", "#add8e6" },
        { "lightcoral", "#f08080" },
        { "lightcyan", "#e0ffff" },
        { "lightgoldenrodyellow", "#fafad2" },
        { "lightgray", "#d3d3d3" },
        { "lightgreen", "#90ee90" },
        { "lightgrey", "#d3d3d3" },
        { "lightpink", "#ffb6c1" },
        { "lightsalmon", "#ffa07a" },
        { "lightseagreen", "#20b2aa" },
        { "lightskyblue", "#87cefa" },
        { "lightslategray", "#778899" },
        { "lightslategrey", "#778899" },
        { "lightsteelblue", "#b0c4de" },
        { "lightyellow", "#ffffe0" },
        { "limegreen", "#32cd32" },
        { "linen", "#faf0e6" },
        { "magenta", "#ff00ff" },
        { "mediumaquamarine", "#66cdaa" },
        { "mediumblue", "#0000cd" },
        { "mediumorchid", "#ba55d3" },
        { "mediumpurple", "#9370db" },
        { "mediumseagreen", "#3cb371" },
        { "mediumslateblue", "#7b68ee" },
        { "mediumspringgreen", "#00fa9a" },
        { "mediumturquoise", "#48d1cc" },
        { "mediumvioletred", "#c71585" },
        { "midnightblue", "#191970" },
        { "mintcream", "#f5fffa" },
        { "mistyrose", "#ffe4e1" },
        { "moccasin", "#ffe4b5" },
        { "navajowhite", "#ffdead" },
        { "oldlace", "#fdf5e6" },
        { "olivedrab", "#6b8e23" },
        { "orangered", "#ff4500" },
        { "orchid", "#da70d6" },
        { "palegoldenrod", "#eee8aa" },
        { "palegreen", "#98fb98" },
        { "paleturquoise", "#afeeee" },
        { "palevioletred", "#db7093" },
        { "papayawhip", "#ffefd5" },
        { "peachpuff", "#ffdab9" },
        { "peru", "#cd853f" },
        { "pink", "#ffc0cb" },
        { "plum", "#dda0dd" },
        { "powderblue", "#b0e0e6" },
        { "rosybrown", "#bc8f8f" },
        { "royalblue", "#4169e1" },
        { "saddlebrown", "#8b4513" },
        { "salmon", "#fa8072" },
        { "sandybrown", "#f4a460" },
        { "seagreen", "#2e8b57" },
        { "seashell", "#fff5ee" },
        { "sienna", "#a0522d" },
        { "skyblue", "#87ceeb" },
        { "slateblue", "#6a5acd" },
        { "slategray", "#708090" },
        { "slategrey", "#708090" },
        { "snow", "#fffafa" },
        { "springgreen", "#00ff7f" },
        { "steelblue", "#4682b4" },
        { "tan", "#d2b48c" },
        { "thistle", "#d8bfd8" },
        { "tomato", "#ff6347" },
        { "turquoise", "#40e0d0" },
        { "violet", "#ee82ee" },
        { "wheat", "#f5deb3" },
        { "whitesmoke", "#f5f5f5" },
        { "yellowgreen", "#9acd32" },
        { "rebeccapurple", "#663399" }
    };
    auto p = named_colors.find(StringUtils::ToLower(name));
    return (p != named_colors.end()) ? Color(p->second) : Color();
}

MISO_INLINE Color
Color::TryParse(const char* str, size_t* consumed_out)
{
    Color color;
    if (TryParseHex(str, color, consumed_out)) return color;
    if (TryParseDec(str, color, consumed_out)) return color;
    return color;
}

MISO_INLINE bool
Color::TryParseHex(const char* str, Color& color_out, size_t* consumed_out)
{
    if (str == nullptr) return false;

    auto s = str;
    auto start = s;

    if (*s != '#') return false;
    ++s;

    float r, g, b, a;

    uint32_t value = 0;
    uint32_t base = 16;
    int count = 0;
    for (; *s != '\0' && count < 8; ++s, ++count) {
        if ('0' <= *s  && *s <= '9') {
            value = value * base + (*s - '0');
        } else if ('a' <= *s  && *s <= 'f') {
            value = value * base + 10 + (*s - 'a');
        } else if ('A' <= *s  && *s <= 'F') {
            value = value * base + 10 + (*s - 'A');
        } else {
            break;
        }
    }
    if (count == 3 || count == 4) {
        auto ri = (value >> ((count - 1) * 4)) & 0xF;
        auto gi = (value >> ((count - 2) * 4)) & 0xF;
        auto bi = (value >> ((count - 3) * 4)) & 0xF;
        r = ((ri << 4) | ri) / 255.0f;
        g = ((gi << 4) | gi) / 255.0f;
        b = ((bi << 4) | bi) / 255.0f;
        if (count == 4) {
            auto ai = value & 0xF;
            a = ((ai << 4) | ai) / 255.0f;
        } else {
            a = 1.0f;
        }
    } else if (count == 6 || count == 8) {
        r = ((value >> ((count - 2) * 4)) & 0xFF) / 255.0f;
        g = ((value >> ((count - 4) * 4)) & 0xFF) / 255.0f;
        b = ((value >> ((count - 6) * 4)) & 0xFF) / 255.0f;
        if (count == 8) {
            a = (value & 0xFF) / 255.0f;
        } else {
            a = 1.0f;
        }
    } else {
        return false;
    }

    color_out.R = r;
    color_out.G = g;
    color_out.B = b;
    color_out.A = a;
    if (consumed_out != nullptr) *consumed_out = static_cast<size_t>(s - start);

    return true;
}

MISO_INLINE bool
Color::TryParseDec(const char* str, Color& color_out, size_t* consumed_out)
{
    if (str == nullptr) return false;

    auto s = str;
    auto start = s;

    size_t value_count = 0;
    auto format = ColorSpace::Invalid;
    if (StringUtils::StartsWith(s, "rgba")) {
        value_count = 4;
        format = ColorSpace::Rgb;
    } else if (StringUtils::StartsWith(s, "rgb")) {
        value_count = 3;
        format = ColorSpace::Rgb;
    } else if (StringUtils::StartsWith(s, "hsla")) {
        value_count = 4;
        format = ColorSpace::Hsl;
    } else if (StringUtils::StartsWith(s, "hsl")) {
        value_count = 3;
        format = ColorSpace::Hsl;
    } else if (StringUtils::StartsWith(s, "hsva")) {
        value_count = 4;
        format = ColorSpace::Hsv;
    } else if (StringUtils::StartsWith(s, "hsv")) {
        value_count = 3;
        format = ColorSpace::Hsv;
    } else {
        return false;
    }
    s += value_count;

    int vmax[4] = {};
    if (format == ColorSpace::Rgb) {
        vmax[0] = vmax[1] = vmax[2] = vmax[3] = 255;
    } else if (format == ColorSpace::Hsl || format == ColorSpace::Hsv) {
        vmax[0] = 360;
        vmax[1] = vmax[2] = vmax[3] = 100;
    } else {
        return false;
    }

    float value[4] = {};
    value[3] = 1.0f;
    size_t value_index = 0;
    bool paren = false;
    for (; *s != '\0' && (value_index < value_count || paren); ++s) {
        if (*s == '(') {
            if (paren || 0 < value_index) return false;
            paren = true;
        } else if (*s == ')') {
            if (!paren || value_index < value_count) return false;
            paren = false;
        } else {
            size_t count;
            auto numeric = Numeric::TryParse(s, &count);
            if (numeric.IsValid()) {
                if (numeric.GetUnit() == NumericUnit::Parcent || numeric.IsFloat()) {
                    float v = numeric.ToRatio(0.0f);
                    value[value_index] = std::max(0.0f, std::min(v, 1.0f));
                } else if (numeric.GetUnit() == NumericUnit::Unitless) {
                    float max = static_cast<float>(vmax[value_index]);
                    float v = static_cast<float>(numeric.GetValue());
                    value[value_index] = std::max(0.0f, std::min(v, max)) / max;
                } else {
                    return false;
                }
                s += (count - 1);
                ++value_index;
            }
        }
    }

    if (paren || value_index < value_count) return false;


    if (format == ColorSpace::Rgb) {
        color_out.R = value[0]; color_out.G = value[1]; color_out.B = value[2]; color_out.A = value[3];
    } else if (format == ColorSpace::Hsl) {
        ColorSpaceUtils::HslToRgb(value[0], value[1], value[2], &color_out.R, &color_out.G, &color_out.B);
        color_out.A = value[3];
    } else if (format == ColorSpace::Hsv) {
        ColorSpaceUtils::HsvToRgb(value[0], value[1], value[2], &color_out.R, &color_out.G, &color_out.B);
        color_out.A = value[3];
    } else {
        return false;
    }
    if (consumed_out != nullptr) *consumed_out = static_cast<size_t>(s - start);

    return true;
}

MISO_INLINE uint32_t
Color::ToUint32() const
{
    auto r = static_cast<uint8_t>(R * 255.0f + 0.5f);
    auto g = static_cast<uint8_t>(G * 255.0f + 0.5f);
    auto b = static_cast<uint8_t>(B * 255.0f + 0.5f);
    auto a = static_cast<uint8_t>(A * 255.0f + 0.5f);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

MISO_INLINE Color
Color::GetInterpolated(const Color& end_value, const Interpolator& interpolator, float progress) const
{
    return Color(
        static_cast<float>(interpolator.Interpolate(R, end_value.R, progress)),
        static_cast<float>(interpolator.Interpolate(G, end_value.G, progress)),
        static_cast<float>(interpolator.Interpolate(B, end_value.B, progress)),
        static_cast<float>(interpolator.Interpolate(A, end_value.A, progress)));
}

// hex3     | #rgb
// hex4     | #rgba
// hex,hex6 | #rrggbb
// hex8     | #rrggbbaa
// rgb      | rgb(r:0-255,g:0-255,b:0-255)
// rgb%     | rgb(r:0-100%,g:0-100%,b:0-100%)
// rgba     | rgb(r:0-255,g:0-255,b:0-255,a:0-255)
// rgba%    | rgb(r:0-100%,g:0-100%,b:0-100%,a:0-100%)
// hsl      | hsl(h:0-360,s:0-100,l:0-100)
// hsl%     | hsl(h:0-100%,s:0-100%,l:0-100%)
// hsla     | hsl(h:0-360,s:0-100,l:0-100,a:0-255)
// hsla%    | hsl(h:0-100%,s:0-100%,l:0-100%,a:0-100%)
// hsv      | hsv(h:0-360,s:0-100,v:0-100)
// hsv%     | hsv(h:0-100%,s:0-100%,v:0-100%)
// hsva     | hsv(h:0-360,s:0-100,v:0-100,a:0-255)
// hsva%    | hsv(h:0-100%,s:0-100%,v:0-100%,a:0-100%)
MISO_INLINE std::string
Color::ToString(const char* format) const
{
    auto f = (format == nullptr) ? "hex" : format;

    std::string s = "";
    if (!IsValid()) return s;

    s = ToStringHex(f);
    if (!s.empty()) return s;

    s = ToStringDec(f);
    if (!s.empty()) return s;

    return s;
}

MISO_INLINE std::string
Color::ToStringHex(const char* format) const
{
    auto f = format;

    if (!StringUtils::StartsWith(f, "hex", true)) return "";
    f += 3;

    auto r = static_cast<uint8_t>(R * 255.0f + 0.5f);
    auto g = static_cast<uint8_t>(G * 255.0f + 0.5f);
    auto b = static_cast<uint8_t>(B * 255.0f + 0.5f);
    auto a = static_cast<uint8_t>(A * 255.0f + 0.5f);
    if (*f == '3') {
        return StringUtils::Format("#%1x%1x%1x", r / 16, g / 16, b / 16);
    } else if (*f == '4') {
        return StringUtils::Format("#%1x%1x%1x%1x", r / 16, g / 16, b / 16, a / 16);
    } else if (*f == '6' || *f == '\0') {
        return StringUtils::Format("#%02x%02x%02x", r, g, b);
    } else if (*f == '8') {
        return StringUtils::Format("#%02x%02x%02x%02x", r, g, b, a);
    } else {
        return "";
    }
}

MISO_INLINE std::string
Color::ToStringDec(const char* format) const
{
    auto f = format;
    auto prefix = "";
    float v[4] = {};
    float vmax[4] = {};
    if (StringUtils::StartsWith(f, "rgb", true)) {
        prefix = "rgb";
        v[0] = R; v[1] = G; v[2] = G; v[3] = A;
        vmax[0] = vmax[1] = vmax[2] = vmax[3] = 255.0f;
    } else if (StringUtils::StartsWith(f, "hsl", true)) {
        prefix = "hsl";
        ColorSpaceUtils::RgbToHsl(R, G, B, &v[0], &v[1], &v[2]); v[3] = A;
        vmax[0] = 360.0f; vmax[1] = vmax[2] = vmax[3] = 100.0f;
    } else if (StringUtils::StartsWith(f, "hsv", true)) {
        prefix = "hsv";
        ColorSpaceUtils::RgbToHsv(R, G, B, &v[0], &v[1], &v[2]); v[3] = A;
        vmax[0] = 360.0f; vmax[1] = vmax[2] = vmax[3] = 100.0f;
    } else {
        return "";
    }

    f += 3;

    bool alpha = false;
    if (*f == 'a') { ++f; alpha = true; }

    bool percent = false;
    if (*f == '%') { ++f; percent = true; }

    if (percent) {
        std::fill_n(vmax, 4, 100.0f);
        format = alpha ? "%sa(%d%%,%d%%,%d%%,%d%%)" : "%s(%d%%,%d%%,%d%%)";
    } else {
        format = alpha ? "%sa(%d,%d,%d,%d)" : "%s(%d,%d,%d)";
    }

    uint8_t vi[4] = {};
    for (int i = 0; i < 4; i++) {
        vi[i] = static_cast<uint8_t>(v[i] * vmax[i] + 0.5f);
    }

    return alpha ?
        StringUtils::Format(format, prefix, vi[0], vi[1], vi[2], vi[3]) :
        StringUtils::Format(format, prefix, vi[0], vi[1], vi[2]);
}

MISO_INLINE bool
Color::operator==(const Color& other) const
{
    return
        (std::abs(R - other.R) < kEqualTolerance) &&
        (std::abs(G - other.G) < kEqualTolerance) &&
        (std::abs(B - other.B) < kEqualTolerance) &&
        (std::abs(A - other.A) < kEqualTolerance);
}

MISO_INLINE bool
Color::operator!=(const Color& other) const
{
    return !(*this == other);
}

} // namespace miso
