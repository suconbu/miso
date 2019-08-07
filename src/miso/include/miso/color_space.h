#ifndef MISO_COLOR_SPACE_H_
#define MISO_COLOR_SPACE_H_

namespace miso {

enum class ColorSpaceType { Unknown, Rgb, Hsl, Hsv };

class ColorSpace {
public:
    ColorSpace() = delete;

    static void HslToRgb(float h, float s, float l, float* r_out, float* g_out, float* b_out);
    static void RgbToHsl(float r, float g, float b, float* h_out, float* s_out, float* l_out);

    static void HsvToRgb(float h, float s, float v, float* r_out, float* g_out, float* b_out);
    static void RgbToHsv(float r, float g, float b, float* h_out, float* s_out, float* v_out);
};

inline void
ColorSpace::HslToRgb(float h, float s, float l, float* r_out, float* g_out, float* b_out)
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

    if (r_out != nullptr) *r_out = r;
    if (g_out != nullptr) *g_out = g;
    if (b_out != nullptr) *b_out = b;
}

inline void
ColorSpace::RgbToHsl(float r, float g, float b, float* h_out, float* s_out, float* l_out)
{
    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    auto h =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0.0f)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    auto l = (max + min) / 2.0f;
    auto s = (l > 0.5f) ? (d / (2.0f - max - min)) : (d / (max + min));

    if (h_out != nullptr) *h_out = h;
    if (s_out != nullptr) *s_out = s;
    if (l_out != nullptr) *l_out = l;
}

inline void
ColorSpace::HsvToRgb(float h, float s, float v, float* r_out, float* g_out, float* b_out)
{
    auto i = std::floor(h * 6.0f);
    auto f = (h * 6.0f) - i;
    auto p = v * (1 - s);
    auto q = v * (1 - f * s);
    auto t = v * (1 - (1 - f) * s);
    auto m = (int)i % 6;

    float r6[] = { v, q, p, p, t, v };
    float g6[] = { t, v, v, q, p, p };
    float b6[] = { p, p, t, v, v, q };

    if (r_out != nullptr) *r_out = r6[m];
    if (g_out != nullptr) *g_out = g6[m];
    if (b_out != nullptr) *b_out = b6[m];
}

inline void
ColorSpace::RgbToHsv(float r, float g, float b, float* h_out, float* s_out, float* v_out)
{
    auto max = std::max(r, std::max(g, b));
    auto min = std::min(r, std::min(g, b));

    float d = max - min;

    auto h =
        (max == min) ? 0.0f :
        (max == r) ? ((g - b) / d + (g < b ? 6.0f : 0.0f)) / 6.0f :
        (max == g) ? ((b - r) / d + 2.0f) / 6.0f :
        (max == b) ? ((r - g) / d + 4.0f) / 6.0f :
        0.0f;
    auto s = (max == 0.0f) ? 0.0f : (d / max);
    auto v = max;

    if (h_out != nullptr) *h_out = h;
    if (s_out != nullptr) *s_out = s;
    if (v_out != nullptr) *v_out = v;
}

} // namespace miso

#endif // MISO_COLOR_SPACE_H_
