#ifndef MISO_CORE_COLOR_H_
#define MISO_CORE_COLOR_H_

#include <ctype.h>
#include <stdint.h>
#include <limits>
#include <string>
#include <algorithm>

#include "miso/scalar.h"
#include "miso/string.h"

namespace miso {

enum class ColorFormat { Invalid, Rgb, Rgba, Hsl, Hsla, Hsv, Hsva, Hex3, Hex4, Hex6, Hex8 };

struct Rgba {
    Rgba() = default;
    Rgba(float red, float green, float blue, float alpha = 1.0f) : R(red), G(green), B(blue), A(alpha) {}

    float R = 0.0f;
    float G = 0.0f;
    float B = 0.0f;
    float A = 0.0f;
};

struct Hsla {
    Hsla() = default;
    Hsla(float hue, float satulate, float lightness, float alpha = 1.0f) :H(hue), S(satulate), L(lightness), A(alpha) {}

    float H = 0.0f;
    float S = 0.0f;
    float L = 0.0f;
    float A = 0.0f;
};

struct Hsva {
    Hsva() = default;
    Hsva(float hue, float satulate, float value, float alpha = 1.0f) :H(hue), S(satulate), V(value), A(alpha) {}

    float H = 0.0f;
    float S = 0.0f;
    float V = 0.0f;
    float A = 0.0f;
};

class Color {
public:
    static const Color& GetInvalid() { const static Color invalid; return invalid; }
    static bool TryParse(const char* str, Color& color_out, size_t* count_out = nullptr);
    static Hsla RgbaToHsla(const Rgba& rgba);
    static Hsva RgbaToHsva(const Rgba& rgba);
    static Rgba HslaToRgba(const Hsla& hsla);
    static Rgba HsvaToRgba(const Hsva& hsva);

    Color() : rgba_{}, format_(ColorFormat::Invalid) {}
    explicit Color(const Rgba& rgba) : rgba_(rgba), format_(ColorFormat::Rgba) {}
    explicit Color(const Hsla& hsla) : rgba_(HslaToRgba(hsla)), format_(ColorFormat::Hsla) {}
    explicit Color(const Hsva& hsva) : rgba_(HsvaToRgba(hsva)), format_(ColorFormat::Hsva) {}
    explicit Color(const char* str) : Color() { TryParse(str, *this); }
    explicit Color(const std::string& str) : Color(str.c_str()) {}

    ColorFormat GetFormat() const { return format_; }
    bool IsValid() const { return format_ != ColorFormat::Invalid; }
    Rgba GetRgba() const { return rgba_; }
    Hsla GetHsla() const { return RgbaToHsla(rgba_); }
    Hsva GetHsva() const { return RgbaToHsva(rgba_); }
    float GetAlpha() const { return rgba_.A; }
    std::string ToString(const char* format = nullptr) const;

    bool operator == (const Color& other) const { return Equals(rgba_, other.rgba_); }
    bool operator != (const Color& other) const { return !Equals(rgba_, other.rgba_); }

private:
    Rgba rgba_;
    ColorFormat format_;

    static bool TryParseHex(const char* str, Color& color_out, size_t* count_out);
    static bool TryParseColor(const char* str, Color& color_out, size_t* count_out);
    static float HueToRgb(float p, float q, float t);
    static bool Equals(const Rgba& a, const Rgba& b) { return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A; }
};

inline bool
Color::TryParse(const char* str, Color& color_out, size_t* count_out)
{
    if (TryParseHex(str, color_out, count_out)) return true;
    if (TryParseColor(str, color_out, count_out)) return true;
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

    Rgba rgba;
    ColorFormat format = ColorFormat::Invalid;

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
        // #123  -> r:0x11, g:0x22, b:0x33
        // #1234 -> r:0x11, g:0x22, b:0x33, a:0x44
        auto r = (value >> ((count - 1) * 4)) & 0xF;
        auto g = (value >> ((count - 2) * 4)) & 0xF;
        auto b = (value >> ((count - 3) * 4)) & 0xF;
        rgba.R = ((r << 4) | r) / 255.0f;
        rgba.G = ((g << 4) | g) / 255.0f;
        rgba.B = ((b << 4) | b) / 255.0f;
        if (count == 4) {
            auto a = value & 0xF;
            rgba.A = ((a << 4) | a) / 255.0f;
            format = ColorFormat::Hex4;
        } else {
            rgba.A = 1.0f;
            format = ColorFormat::Hex3;
        }
    } else if (count == 6 || count == 8) {
        rgba.R = ((value >> ((count - 2) * 4)) & 0xFF) / 255.0f;
        rgba.G = ((value >> ((count - 4) * 4)) & 0xFF) / 255.0f;
        rgba.B = ((value >> ((count - 6) * 4)) & 0xFF) / 255.0f;
        if (count == 8) {
            rgba.A = (value & 0xFF) / 255.0f;
            format = ColorFormat::Hex8;
        } else {
            rgba.A = 1.0f;
            format = ColorFormat::Hex6;
        }
    } else {
        return false;
    }

    color_out.rgba_ = rgba;
    color_out.format_ = format;
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline bool
Color::TryParseColor(const char* str, Color& color_out, size_t* count_out)
{
    if (str == nullptr) return false;

    auto s = str;
    auto start = s;

    ColorFormat format = ColorFormat::Invalid;

    // (rgb|rgba|...)\s*\(?\s*\d+\s*,?\s*

    int value_count = 0;
    if (StringUtils::CompareIgnoreCase(s, "rgba", 4) == 0) {
        value_count = 4;
        format = ColorFormat::Rgba;
    } else if (StringUtils::CompareIgnoreCase(s, "rgb", 3) == 0) {
        value_count = 3;
        format = ColorFormat::Rgb;
    } else if (StringUtils::CompareIgnoreCase(s, "hsla", 4) == 0) {
        value_count = 4;
        format = ColorFormat::Hsla;
    } else if (StringUtils::CompareIgnoreCase(s, "hsl", 3) == 0) {
        value_count = 3;
        format = ColorFormat::Hsl;
    } else if (StringUtils::CompareIgnoreCase(s, "hsva", 4) == 0) {
        value_count = 4;
        format = ColorFormat::Hsva;
    } else if (StringUtils::CompareIgnoreCase(s, "hsv", 3) == 0) {
        value_count = 3;
        format = ColorFormat::Hsv;
    } else {
        return false;
    }
    s += value_count;

    int value_max[4] = {};
    if (format == ColorFormat::Rgb || format == ColorFormat::Rgba) {
        value_max[0] = value_max[1] = value_max[2] = value_max[3] = 255;
    } else if (
        format == ColorFormat::Hsl || format == ColorFormat::Hsla ||
        format == ColorFormat::Hsv || format == ColorFormat::Hsva) {
        value_max[0] = 360;
        value_max[1] = value_max[2] = value_max[3] = 100;
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
            Scalar scalar;
            size_t count;
            if (Scalar::TryParse(s, scalar, &count)) {
                if (scalar.GetUnit() == ScalarUnit::Parcent || scalar.IsFloat()) {
                    float v = static_cast<float>(scalar.ToRatio());
                    value[value_index] = std::max(0.0f, std::min(v, 1.0f));
                } else if (scalar.GetUnit() == ScalarUnit::Unitless) {
                    float max = static_cast<float>(value_max[value_index]);
                    float v = static_cast<float>(scalar.GetValue());
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

    color_out.rgba_ =
        (format == ColorFormat::Rgb || format == ColorFormat::Rgba) ? Rgba(value[0], value[1], value[2], value[3]) :
        (format == ColorFormat::Hsl || format == ColorFormat::Hsla) ? HslaToRgba(Hsla(value[0], value[1], value[2], value[3])) :
        (format == ColorFormat::Hsv || format == ColorFormat::Hsva) ? HsvaToRgba(Hsva(value[0], value[1], value[2], value[3])) :
        Rgba();
    color_out.format_ = format;
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline Hsla
Color::RgbaToHsla(const Rgba& rgba)
{
    auto r = rgba.R;
    auto g = rgba.G;
    auto b = rgba.B;
    auto a = rgba.A;

    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    float h =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    float l = (max + min) / 2.0f;
    float s = (l > 0.5f) ? (d / (2.0f - max - min)) : (d / (max + min));

    return Hsla(h, s, l, a);
}

inline Hsva
Color::RgbaToHsva(const Rgba& rgba)
{
    auto r = rgba.R;
    auto g = rgba.G;
    auto b = rgba.B;
    auto a = rgba.A;

    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    float h =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    float s = (max == 0.0f) ? 0.0f : (d / max);
    float v = max;

    return Hsva(h, s, v, a);
}

inline Rgba
Color::HslaToRgba(const Hsla& hsla)
{
    auto h = hsla.H;
    auto s = hsla.S;
    auto l = hsla.L;
    auto a = hsla.A;

    auto r = l;
    auto g = l;
    auto b = l;

    if (s > 0.0f) {
        auto q = (l < 0.5f) ? (l * (1 + s)) : (l + s - l * s);
        auto p = 2.0f * l - q;
        r = HueToRgb(p, q, h + 2 / 6.0f);
        g = HueToRgb(p, q, h);
        b = HueToRgb(p, q, h - 2 / 6.0f);
    }

    return Rgba(r, g, b, a);
}

inline float
Color::HueToRgb(float p, float q, float t)
{
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    return
        (t < 1 / 6.0f) ? (p + (q - p) * 6.0f * t) :
        (t < 3 / 6.0f) ? (q) :
        (t < 4 / 6.0f) ? (p + (q - p) * (4 / 6.0f - t) * 6.0f) :
        p;
}

inline Rgba
Color::HsvaToRgba(const Hsva& hsva)
{
    auto h = hsva.H * 6.0f;
    auto s = hsva.S;
    auto v = hsva.V;
    auto a = hsva.A;

    auto i = std::floor(h);
    auto f = h - i;
    auto p = v * (1 - s);
    auto q = v * (1 - f * s);
    auto t = v * (1 - (1 - f) * s);
    auto m = (int)i % 6;

    float r6[] = { v, q, p, p, t, v };
    float g6[] = { t, v, v, q, p, p };
    float b6[] = { p, p, t, v, v, q };

    return Rgba(r6[m], g6[m], b6[m], a);
}

inline std::string
Color::ToString(const char* format) const
{
    if (format_ == ColorFormat::Invalid) return "";
    return StringUtils::Format("#%02x%02x%02x%02x",
        static_cast<uint8_t>(rgba_.R * 255 + 0.5f),
        static_cast<uint8_t>(rgba_.G * 255 + 0.5f),
        static_cast<uint8_t>(rgba_.B * 255 + 0.5f),
        static_cast<uint8_t>(rgba_.A * 255 + 0.5f));
}

} // namespace miso

#endif // MISO_CORE_COLOR_H_
