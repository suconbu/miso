#ifndef MISO_COLOR_CONTAINER_H_
#define MISO_COLOR_CONTAINER_H_

#include <algorithm>

namespace miso {

enum class ColorSpace { Unknown, Rgb, Hsl, Hsv };

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
    Hsla(float hue, float satulate, float lightness, float alpha = 1.0f) : H(hue), S(satulate), L(lightness), A(alpha) {}
    explicit Hsla(const Rgba& rgba);

    Rgba ToRgba() const;
    operator Rgba() const { return ToRgba(); }

    float H = 0.0f;
    float S = 0.0f;
    float L = 0.0f;
    float A = 0.0f;
};

struct Hsva {
    Hsva() = default;
    Hsva(float hue, float satulate, float value, float alpha = 1.0f) : H(hue), S(satulate), V(value), A(alpha) {}
    explicit Hsva(const Rgba& rgba);

    Rgba ToRgba() const;
    operator Rgba() const { return ToRgba(); }

    float H = 0.0f;
    float S = 0.0f;
    float V = 0.0f;
    float A = 0.0f;
};

inline
Hsla::Hsla(const Rgba& rgba)
{
    auto r = rgba.R;
    auto g = rgba.G;
    auto b = rgba.B;
    auto a = rgba.A;

    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    H =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0.0f)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    L = (max + min) / 2.0f;
    S = (L > 0.5f) ? (d / (2.0f - max - min)) : (d / (max + min));
    A = a;
}

inline Rgba
Hsla::ToRgba() const
{
    auto hueToRgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        return
            (t < 1.0f / 6.0f) ? (p + (q - p) * 6.0f * t) :
            (t < 3.0f / 6.0f) ? (q) :
            (t < 4.0f / 6.0f) ? (p + (q - p) * (4.0f / 6.0f - t) * 6.0f) :
            p;
    };

    auto h = H;
    auto s = S;
    auto l = L;

    auto r = l;
    auto g = l;
    auto b = l;

    if (s > 0.0f) {
        auto q = (l < 0.5f) ? (l * (1 + s)) : (l + s - l * s);
        auto p = 2.0f * l - q;
        r = hueToRgb(p, q, h + 2.0f / 6.0f);
        g = hueToRgb(p, q, h);
        b = hueToRgb(p, q, h - 2.0f / 6.0f);
    }

    return Rgba(r, g, b, A);
}

inline
Hsva::Hsva(const Rgba& rgba)
{
    auto r = rgba.R;
    auto g = rgba.G;
    auto b = rgba.B;
    auto a = rgba.A;

    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    H =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0.0f)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    S = (max == 0.0f) ? 0.0f : (d / max);
    V = max;
    A = a;
}

inline Rgba
Hsva::ToRgba() const
{
    auto h = H * 6.0f;
    auto s = S;
    auto v = V;

    auto i = std::floor(h);
    auto f = h - i;
    auto p = v * (1 - s);
    auto q = v * (1 - f * s);
    auto t = v * (1 - (1 - f) * s);
    auto m = (int)i % 6;

    float r6[] = { v, q, p, p, t, v };
    float g6[] = { t, v, v, q, p, p };
    float b6[] = { p, p, t, v, v, q };

    return Rgba(r6[m], g6[m], b6[m], A);
}

} // namespace miso

#endif // MISO_COLOR_CONTAINER_H_
