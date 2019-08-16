#ifndef MISO_COLOR_H_
#define MISO_COLOR_H_

#include <stdint.h>
#include <string>
#include <algorithm>

#include "miso/color_space.h"
#include "miso/numeric.h"
#include "miso/string_utils.h"

namespace miso {

class Color {
public:
    static const Color& GetInvalid() { const static Color invalid; return invalid; }
    static bool TryParse(const char* str, Color& color_out, size_t* count_out = nullptr);
    static Color FromHsla(float h, float s, float l, float a);
    static Color FromHsva(float h, float s, float v, float a);
    //static Color FromName(const char* name);

    Color() = default;
    explicit Color(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
    explicit Color(uint32_t rgba) : R(((rgba >> 24) & 0xFF) / 255.0f), G(((rgba >> 16) & 0xFF) / 255.0f), B(((rgba >> 8) & 0xFF) / 255.0f), A(((rgba >> 0) & 0xFF) / 255.0f) {}
    explicit Color(const char* str) { TryParse(str, *this); }
    explicit Color(const std::string& str) : Color(str.c_str()) {}

    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    bool IsValid() const { return !std::isnan(R) && !std::isnan(G) && !std::isnan(B) && !std::isnan(A); }
    uint32_t ToUint32() const;
    Color GetInterpolated(const Color& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

    float R = std::numeric_limits<float>::quiet_NaN();
    float G = std::numeric_limits<float>::quiet_NaN();
    float B = std::numeric_limits<float>::quiet_NaN();
    float A = std::numeric_limits<float>::quiet_NaN();

private:
    static constexpr float kEqualTolerance = 0.0001f;
    static bool TryParseHex(const char* str, Color& color_out, size_t* count_out);
    static bool TryParseDec(const char* str, Color& color_out, size_t* count_out);
    std::string ToStringHex(const char* format) const;
    std::string ToStringDec(const char* format) const;
};

inline Color
Color::FromHsla(float h, float s, float l, float a)
{
    float r, g, b;
    ColorSpace::HslToRgb(h, s, l, &r, &g, &b);
    return Color(r, g, b, a);
}

inline Color
Color::FromHsva(float h, float s, float v, float a)
{
    float r, g, b;
    ColorSpace::HsvToRgb(h, s, v, &r, &g, &b);
    return Color(r, g, b, a);
}

inline bool
Color::TryParse(const char* str, Color& color_out, size_t* count_out)
{
    if (TryParseHex(str, color_out, count_out)) return true;
    if (TryParseDec(str, color_out, count_out)) return true;
    return false;
}

inline bool
Color::TryParseHex(const char* str, Color& color_out, size_t* count_out)
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
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline bool
Color::TryParseDec(const char* str, Color& color_out, size_t* count_out)
{
    if (str == nullptr) return false;

    auto s = str;
    auto start = s;

    int value_count = 0;
    auto format = ColorSpaceType::Invalid;
    if (StringUtils::StartsWith(s, "rgba")) {
        value_count = 4;
        format = ColorSpaceType::Rgb;
    } else if (StringUtils::StartsWith(s, "rgb")) {
        value_count = 3;
        format = ColorSpaceType::Rgb;
    } else if (StringUtils::StartsWith(s, "hsla")) {
        value_count = 4;
        format = ColorSpaceType::Hsl;
    } else if (StringUtils::StartsWith(s, "hsl")) {
        value_count = 3;
        format = ColorSpaceType::Hsl;
    } else if (StringUtils::StartsWith(s, "hsva")) {
        value_count = 4;
        format = ColorSpaceType::Hsv;
    } else if (StringUtils::StartsWith(s, "hsv")) {
        value_count = 3;
        format = ColorSpaceType::Hsv;
    } else {
        return false;
    }
    s += value_count;

    int vmax[4] = {};
    if (format == ColorSpaceType::Rgb) {
        vmax[0] = vmax[1] = vmax[2] = vmax[3] = 255;
    } else if (format == ColorSpaceType::Hsl || format == ColorSpaceType::Hsv) {
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
            Numeric numeric;
            size_t count;
            if (Numeric::TryParse(s, numeric, &count)) {
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


    if (format == ColorSpaceType::Rgb) {
        color_out.R = value[0]; color_out.G = value[1]; color_out.B = value[2]; color_out.A = value[3];
    } else if (format == ColorSpaceType::Hsl) {
        ColorSpace::HslToRgb(value[0], value[1], value[2], &color_out.R, &color_out.G, &color_out.B);
        color_out.A = value[3];
    } else if (format == ColorSpaceType::Hsv) {
        ColorSpace::HsvToRgb(value[0], value[1], value[2], &color_out.R, &color_out.G, &color_out.B);
        color_out.A = value[3];
    } else {
        return false;
    }
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline uint32_t
Color::ToUint32() const
{
    auto r = static_cast<uint8_t>(R * 255.0f + 0.5f);
    auto g = static_cast<uint8_t>(G * 255.0f + 0.5f);
    auto b = static_cast<uint8_t>(B * 255.0f + 0.5f);
    auto a = static_cast<uint8_t>(A * 255.0f + 0.5f);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

inline Color
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
inline std::string
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

inline std::string
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

inline std::string
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
        ColorSpace::RgbToHsl(R, G, B, &v[0], &v[1], &v[2]); v[3] = A;
        vmax[0] = 360.0f; vmax[1] = vmax[2] = vmax[3] = 100.0f;
    } else if (StringUtils::StartsWith(f, "hsv", true)) {
        prefix = "hsv";
        ColorSpace::RgbToHsv(R, G, B, &v[0], &v[1], &v[2]); v[3] = A;
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

inline bool
Color::operator==(const Color& other) const
{
    return
        (std::abs(R - other.R) < kEqualTolerance) &&
        (std::abs(G - other.G) < kEqualTolerance) &&
        (std::abs(B - other.B) < kEqualTolerance) &&
        (std::abs(A - other.A) < kEqualTolerance);
}

inline bool
Color::operator!=(const Color& other) const
{
    return !(*this == other);
}

} // namespace miso

#endif // MISO_COLOR_H_
