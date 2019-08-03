#ifndef MISO_CORE_SCALAR_H_
#define MISO_CORE_SCALAR_H_

#include <ctype.h>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "miso/color.h"
#include "miso/string.h"

namespace miso {

enum class ScalarUnit {
    NaN,
    Pixel, ScaledPixel,
    Vw, Vh, Vmax, Vmin,
    Parcent,
    Second, Millisecond,
    Unitless,
};

class Scalar {
public:
    static constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

    static const Scalar& GetInvalid() { const static Scalar invalid; return invalid; }
    static int TryParse(const char* str, Scalar& scalar_out);

    Scalar() : value_(kNaN), unit_(ScalarUnit::NaN) {}
    explicit Scalar(const char* str) : Scalar() { Scalar::TryParse(str, *this); }
    explicit Scalar(const std::string& str) : Scalar(str.c_str()) {}

    double GetValue() const { return value_; }
    ScalarUnit GetUnit() const { return unit_; }
    bool IsValid() const { return unit_ != ScalarUnit::NaN; }
    double ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value = kNaN) const;
    double ToRatio(float default_value = kNaN) const;
    double ToMilliseconds(float default_value = kNaN) const;
    std::string ToString(const char* format = nullptr) const;

private:
    double value_;
    ScalarUnit unit_;

    static const std::map<ScalarUnit, const char*>& GetUnitToSuffixMap();
};

inline int
Scalar::TryParse(const char* str, Scalar& scalar_out)
{
    if (str == nullptr || *str == '\0') return 0;

    // [+-]?(\d+(\.(\d+)?)?)|(\.\d+)?(\w+|%)
    auto s = str;
    auto start = s;

    // Sign
    bool negative = false;
    if (*s == '+' || *s == '-') {
        negative = (*s == '-');
        ++s;
    }

    if (*s != '.' && !isdigit(*s)) return 0;

    // Digits
    auto digit_start = s;
    const int base = 10;
    double value = 0.0;
    int denominator = 0;
    for (; *s != '\0'; ++s) {
        if (isdigit(*s)) {
            int n = (*s - '0');
            if (denominator == 0) {
                value = value * base + n;
            } else {
                value = value + static_cast<double>(n) / denominator;
                denominator *= base;
            }
        } else if (*s == '.') {
            if (denominator != 0) return 0;
            denominator = base;
        } else {
            break;
        }
    }
    if (s == digit_start) return 0;

    // Unit
    auto unit = ScalarUnit::NaN;
    for (auto& pair : GetUnitToSuffixMap()) {
        auto len = strlen(pair.second);
        if (strncmp(pair.second, s, len) == 0 && (*(s + len) == '\0' || isspace(*(s + len)))) {
            unit = pair.first;
            s += len;
            break;
        }
    }
    if (unit == ScalarUnit::NaN) return 0;

    if (negative) value = -value;

    scalar_out.value_ = value;
    scalar_out.unit_ = unit;

    return static_cast<int>(s - start);
}

inline const std::map<ScalarUnit, const char*>&
Scalar::GetUnitToSuffixMap()
{
    static const std::map<ScalarUnit, const char*> kUnitToSuffix = {
        { ScalarUnit::Pixel, "px" },
        { ScalarUnit::ScaledPixel, "sp" },
        { ScalarUnit::Parcent, "%" },
        { ScalarUnit::Vw, "vw" },
        { ScalarUnit::Vh, "vh" },
        { ScalarUnit::Vmax, "vmax" },
        { ScalarUnit::Vmin, "vmin" },
        { ScalarUnit::Second, "s" },
        { ScalarUnit::Millisecond, "ms" },
        { ScalarUnit::Unitless, "" },
    };
    return kUnitToSuffix;
}

inline double
Scalar::ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value) const
{
    return
        (unit_ == ScalarUnit::Pixel) ? value_ :
        (unit_ == ScalarUnit::ScaledPixel) ? value_ * pixel_scale :
        (unit_ == ScalarUnit::Vw) ? value_ / 100.0f * view_width :
        (unit_ == ScalarUnit::Vh) ? value_ / 100.0f * view_height :
        (unit_ == ScalarUnit::Vmax) ? value_ / 100.0f * std::max(view_width, view_height) :
        (unit_ == ScalarUnit::Vmin) ? value_ / 100.0f * std::min(view_width, view_height) :
        (unit_ == ScalarUnit::Parcent) ? value_ / 100.0f * base_length :
        (unit_ == ScalarUnit::Unitless) ? value_ * base_length :
        default_value;
}

inline double
Scalar::ToRatio(float default_value) const
{
    return
        (unit_ == ScalarUnit::Parcent) ? value_ / 100.0f :
        (unit_ == ScalarUnit::Unitless) ? value_ :
        default_value;
}

inline double
Scalar::ToMilliseconds(float default_value) const
{
    return
        (unit_ == ScalarUnit::Second) ? value_ * 1000.0f :
        (unit_ == ScalarUnit::Millisecond) ? value_ :
        default_value;
}

inline std::string
Scalar::ToString(const char* format) const
{
    (void)format;
    if (unit_ == ScalarUnit::NaN) return "";
    if (floor(value_) == value_) {
        return StringUtils::Format("%.0Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    } else {
        return StringUtils::Format("%.3Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    }
}

} // namespace miso

#endif // MISO_CORE_SCALAR_H_
