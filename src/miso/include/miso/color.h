#ifndef MISO_COLOR_H_
#define MISO_COLOR_H_

#include <ctype.h>
#include <stdint.h>
#include <limits>
#include <string>
#include <algorithm>

#include "miso/color_container.h"
#include "miso/color_formatter.h"
#include "miso/scalar.h"
#include "miso/string.h"

namespace miso {

class Color {
public:
    static const Color& GetInvalid() { const static Color invalid; return invalid; }
    static bool TryParse(const char* str, Color& color_out, size_t* count_out = nullptr);

    Color() : rgba_{}, valid_(false) {}
    explicit Color(const Rgba& rgba) : rgba_(rgba), valid_(true) {}
    explicit Color(const char* str) : Color() { TryParse(str, *this); }
    explicit Color(const std::string& str) : Color(str.c_str()) {}

    bool IsValid() const { return valid_; }
    Rgba GetRgba() const { return rgba_; }
    float GetAlpha() const { return rgba_.A; }
    std::string ToString(const char* format = nullptr) const;

    bool operator == (const Color& other) const { return Equals(rgba_, other.rgba_); }
    bool operator != (const Color& other) const { return !Equals(rgba_, other.rgba_); }

private:
    Rgba rgba_;
    bool valid_;

    static bool TryParseHex(const char* str, Color& color_out, size_t* count_out);
    static bool TryParseColorSpace(const char* str, Color& color_out, size_t* count_out);
    static bool Equals(const Rgba& a, const Rgba& b) { return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A; }
};

inline bool
Color::TryParse(const char* str, Color& color_out, size_t* count_out)
{
    if (TryParseHex(str, color_out, count_out)) return true;
    if (TryParseColorSpace(str, color_out, count_out)) return true;
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
        auto r = (value >> ((count - 1) * 4)) & 0xF;
        auto g = (value >> ((count - 2) * 4)) & 0xF;
        auto b = (value >> ((count - 3) * 4)) & 0xF;
        rgba.R = ((r << 4) | r) / 255.0f;
        rgba.G = ((g << 4) | g) / 255.0f;
        rgba.B = ((b << 4) | b) / 255.0f;
        if (count == 4) {
            auto a = value & 0xF;
            rgba.A = ((a << 4) | a) / 255.0f;
        } else {
            rgba.A = 1.0f;
        }
    } else if (count == 6 || count == 8) {
        rgba.R = ((value >> ((count - 2) * 4)) & 0xFF) / 255.0f;
        rgba.G = ((value >> ((count - 4) * 4)) & 0xFF) / 255.0f;
        rgba.B = ((value >> ((count - 6) * 4)) & 0xFF) / 255.0f;
        if (count == 8) {
            rgba.A = (value & 0xFF) / 255.0f;
        } else {
            rgba.A = 1.0f;
        }
    } else {
        return false;
    }

    color_out.rgba_ = rgba;
    color_out.valid_ = true;
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline bool
Color::TryParseColorSpace(const char* str, Color& color_out, size_t* count_out)
{
    if (str == nullptr) return false;

    auto s = str;
    auto start = s;

    int value_count = 0;
    auto format = ColorSpace::Unknown;
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

    int value_max[4] = {};
    if (format == ColorSpace::Rgb) {
        value_max[0] = value_max[1] = value_max[2] = value_max[3] = 255;
    } else if (format == ColorSpace::Hsl || format == ColorSpace::Hsv) {
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
        (format == ColorSpace::Rgb) ? Rgba(value[0], value[1], value[2], value[3]) :
        (format == ColorSpace::Hsl) ? Hsla(value[0], value[1], value[2], value[3]).ToRgba() :
        (format == ColorSpace::Hsv) ? Hsva(value[0], value[1], value[2], value[3]).ToRgba() :
        Rgba();
    color_out.valid_ = true;
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
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

    if (!valid_) {
        return "";
    } else if (StringUtils::StartsWith(f, "hex")) {
        f += 3;
        if (*f == '3') {
            return ColorFormatter::ToHexString(rgba_, 3);
        } else if (*f == '4') {
            return ColorFormatter::ToHexString(rgba_, 4);
        } else if (*f == '6' || *f == '\0') {
            return ColorFormatter::ToHexString(rgba_, 6);
        } else if (*f == '8') {
            return ColorFormatter::ToHexString(rgba_, 8);
        } else {
            return "";
        }
    } else {
        auto colorspace = ColorSpace::Unknown;
        if (StringUtils::StartsWith(f, "rgb")) {
            colorspace = ColorSpace::Rgb;
        } else if (StringUtils::StartsWith(f, "hsl")) {
            colorspace = ColorSpace::Hsl;
        } else if (StringUtils::StartsWith(f, "hsv")) {
            colorspace = ColorSpace::Hsv;
        } else {
            return "";
        }

        f += 3;

        bool alpha = false;
        if (*f == 'a') { ++f; alpha = true; }

        bool percent = false;
        if (*f == '%') { ++f; percent = true; }

        return ColorFormatter::ToColorSpaceString(rgba_, colorspace, alpha, percent);
    }
}

} // namespace miso

#endif // MISO_COLOR_H_
