#ifndef MISO_CORE_NUMERIC_H_
#define MISO_CORE_NUMERIC_H_

#include <ctype.h>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "miso/string.h"

namespace miso {

enum class NumericUnit {
    NaN,
    Pixel, ScaledPixel,
    Vw, Vh, Vmax, Vmin,
    Parcent,
    Second, Millisecond,
    Unitless,
};

class Numeric {
public:
    static std::vector<Numeric> Parse(const char* str);

    Numeric() : value_(kNaN), unit_(NumericUnit::NaN) {}
    explicit Numeric(const char* str);
    explicit Numeric(const std::string& str) : Numeric(str.c_str()) {}

    double GetValue() const { return value_; }
    NumericUnit GetUnit() const { return unit_; }
    bool IsNaN() const { return unit_ == NumericUnit::NaN; }
    double ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value = kNaN) const;
    double ToRatio(float default_value = kNaN) const;
    double ToMilliseconds(float default_value = kNaN) const;
    std::string ToString() const;

private:
    constexpr static const double kNaN = std::numeric_limits<double>::quiet_NaN();

    static int Parse(const char* str, double* value_out, NumericUnit* unit_out);

    double value_;
    NumericUnit unit_;
};

inline
int Numeric::Parse(const char* str, double* value_out, NumericUnit* unit_out)
{
    if (str == nullptr || *str == '\0') return 0;

    static const std::pair<const char*, NumericUnit> kStringToUnit[] = {
        { "px", NumericUnit::Pixel },
        { "sp", NumericUnit::ScaledPixel },
        { "%", NumericUnit::Parcent },
        { "vw", NumericUnit::Vw },
        { "vh", NumericUnit::Vh },
        { "vmax", NumericUnit::Vmax },
        { "vmin", NumericUnit::Vmin },
        { "s", NumericUnit::Second },
        { "ms", NumericUnit::Millisecond },
        { "", NumericUnit::Unitless },
    };

    // [+-]?(\d+(\.(\d+)?)?)|(\.\d+)?(\w+|%)
    auto s = str;

    // Sign
    auto start = s;
    char pre = '\0';
    for (; *s != '\0' && *s != '.' && !isdigit(*s); ++s) pre = *s;
    const bool negative = (pre == '-');

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
    auto unit = NumericUnit::NaN;
    for (auto pair : kStringToUnit) {
        auto len = strlen(pair.first);
        if (strncmp(pair.first, s, len) == 0 && (*(s + len) == '\0' || isspace(*(s + len)))) {
            unit = pair.second;
            s += len;
            break;
        }
    }
    if (unit == NumericUnit::NaN) return 0;

    if (negative) value = -value;

    if (value_out != nullptr) *value_out = value;
    if (unit_out != nullptr) *unit_out = unit;

    return static_cast<int>(s - start);
}

inline std::vector<Numeric>
Numeric::Parse(const char* str)
{
    std::vector<Numeric> numerics;
    for (auto& token : StringUtils::Split(str)) {
        numerics.push_back(Numeric(token));
    }
    return numerics;
}

inline
Numeric::Numeric(const char* str) : Numeric()
{
    Numeric::Parse(str, &value_, &unit_);
}

inline double
Numeric::ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value) const
{
    return
        (unit_ == NumericUnit::Pixel) ? value_ :
        (unit_ == NumericUnit::ScaledPixel) ? value_ * pixel_scale :
        (unit_ == NumericUnit::Vw) ? value_ / 100.0f * view_width :
        (unit_ == NumericUnit::Vh) ? value_ / 100.0f * view_height :
        (unit_ == NumericUnit::Vmax) ? value_ / 100.0f * std::max(view_width, view_height) :
        (unit_ == NumericUnit::Vmin) ? value_ / 100.0f * std::min(view_width, view_height) :
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0f * base_length :
        (unit_ == NumericUnit::Unitless) ? value_ * base_length :
        default_value;
}

inline double
Numeric::ToRatio(float default_value) const
{
    return
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0f :
        (unit_ == NumericUnit::Unitless) ? value_ :
        default_value;
}

inline double
Numeric::ToMilliseconds(float default_value) const
{
    return
        (unit_ == NumericUnit::Second) ? value_ * 1000.0f :
        (unit_ == NumericUnit::Millisecond) ? value_ :
        default_value;
}

inline std::string
Numeric::ToString() const
{
    if (unit_ == NumericUnit::NaN) return "NaN";
    static const std::map<NumericUnit, const char*> kUnitToString = {
        { NumericUnit::Pixel, "px" },
        { NumericUnit::ScaledPixel, "sp" },
        { NumericUnit::Parcent, "%" },
        { NumericUnit::Vw, "vw" },
        { NumericUnit::Vh, "vh" },
        { NumericUnit::Vmax, "vmax" },
        { NumericUnit::Vmin, "vmin" },
        { NumericUnit::Second, "s" },
        { NumericUnit::Millisecond, "ms" },
        { NumericUnit::Unitless, "" },
    };
    if (floor(value_) == value_) {
        return StringUtils::Format("%.0Lf%s", value_, kUnitToString.at(unit_));
    } else {
        return StringUtils::Format("%.3Lf%s", value_, kUnitToString.at(unit_));
    }
}

} // namespace miso

#endif // MISO_CORE_NUMERIC_H_
