#ifndef MISO_CORE_COLOR_H_
#define MISO_CORE_COLOR_H_

#include <ctype.h>
#include <stdint.h>
#include <limits>
#include <string>
#include <algorithm>

#include "miso/string.h"

namespace miso {

enum class ColorFormat { Invalid, Rgb, Rgba, Hsv, Hsva, Hex3, Hex4, Hex6, Hex8 };

struct Rgba {
    Rgba() = default;
    Rgba(float red, float green, float blue, float alpha = 1.0f) : R(red), G(green), B(blue), A(alpha) {}

    float R = 0.0f;
    float G = 0.0f;
    float B = 0.0f;
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
    static int TryParse(const char* str, Color& color_out);
    static Hsva RgbaToHsva(const Rgba& rgba);
    static Rgba HsvaToRgba(const Hsva& hsva);

    Color() : rgba_{}, format_(ColorFormat::Invalid) {}
    explicit Color(const Rgba& rgba) : rgba_(rgba), format_(ColorFormat::Rgba) {}
    explicit Color(const Hsva& hsva) : rgba_(HsvaToRgba(hsva)), format_(ColorFormat::Hsva) {}
    explicit Color(const char* str) : Color() { TryParse(str, *this); }
    explicit Color(const std::string& str) : Color(str.c_str()) {}

    ColorFormat GetFormat() const { return format_; }
    bool IsValid() const { return format_ != ColorFormat::Invalid; }
    Rgba GetRgba() const { return rgba_; }
    Hsva GetHsva() const { return RgbaToHsva(rgba_); }
    float GetAlpha() const { return rgba_.A; }
    std::string ToString(const char* format = nullptr) const;

private:
    Rgba rgba_;
    ColorFormat format_;
};

inline int
Color::TryParse(const char* str, Color& color_out)
{
    //TODO:
    color_out.rgba_ = Rgba();
    color_out.format_ = ColorFormat::Rgba;

    return 0;
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
    return ""; //TODO:
}

} // namespace miso

#endif // MISO_CORE_COLOR_H_
