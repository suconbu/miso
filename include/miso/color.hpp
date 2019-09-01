#ifndef MISO_COLOR_HPP_
#define MISO_COLOR_HPP_

#include "miso/common.h"

#include <string>

#include "miso/interpolator.hpp"

namespace miso {

class Color {
public:
    static const Color& GetInvalid();
    static const Color& GetZero();
    static Color TryParse(const char* str, size_t* consumed_out = nullptr);
    static Color FromHsla(float h, float s, float l, float a);
    static Color FromHsva(float h, float s, float v, float a);
    static Color FromHtmlColorName(const char* name);
    static Color FromHtmlColorName(const std::string& name) { return FromHtmlColorName(name.c_str()); }

    Color() = default;
    Color(const Color&) = default;
    Color& operator=(const Color&) = default;
    explicit Color(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
    explicit Color(uint32_t rgba) : R(((rgba >> 24) & 0xFF) / 255.0f), G(((rgba >> 16) & 0xFF) / 255.0f), B(((rgba >> 8) & 0xFF) / 255.0f), A(((rgba >> 0) & 0xFF) / 255.0f) {}
    explicit Color(const char* str) { *this = TryParse(str); }
    explicit Color(const std::string& str) : Color(str.c_str()) {}

    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;
    Color operator*(double multiplier) const;

    bool IsValid() const { return !std::isnan(R) && !std::isnan(G) && !std::isnan(B) && !std::isnan(A); }
    bool IsTrue() const { return IsValid() && (R != 0.0f || G != 0.0f || B != 0.0f || A != 0.0f); }
    uint32_t ToUint32() const;
    Color GetInterpolated(const Color& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

    float R = std::numeric_limits<float>::quiet_NaN();
    float G = std::numeric_limits<float>::quiet_NaN();
    float B = std::numeric_limits<float>::quiet_NaN();
    float A = std::numeric_limits<float>::quiet_NaN();

private:
    static constexpr float kEqualTolerance = 0.0001f;
    static bool TryParseHex(const char* str, Color* color_out, size_t* consumed_out);
    static bool TryParseDec(const char* str, Color* color_out, size_t* consumed_out);

    std::string ToStringHex(const char* format) const;
    std::string ToStringDec(const char* format) const;
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "color.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_COLOR_HPP_
