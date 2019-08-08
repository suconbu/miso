#ifndef MISO_NUMERIC_H_
#define MISO_NUMERIC_H_

#include <map>
#include <string>

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
    static constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

    static const Numeric& GetInvalid() { const static Numeric invalid; return invalid; }
    static bool TryParse(const char* str, Numeric& numeric_out, size_t* count_out = nullptr);

    Numeric() : value_(kNaN), unit_(NumericUnit::NaN), float_(false) {}
    explicit Numeric(const char* str) : Numeric() { Numeric::TryParse(str, *this); }
    explicit Numeric(const std::string& str) : Numeric(str.c_str()) {}

    bool IsValid() const { return unit_ != NumericUnit::NaN; }
    bool IsFloat() const { return float_; }
    double GetValue() const { return value_; }
    NumericUnit GetUnit() const { return unit_; }
    double ToLength(double view_width, double view_height, double pixel_scale, double base_length, double default_value = kNaN) const;
    double ToRatio(double default_value = kNaN) const;
    double ToMilliseconds(double default_value = kNaN) const;
    std::string ToString(const char* format = nullptr) const;

private:
    double value_;
    NumericUnit unit_;
    bool float_;

    static const std::map<NumericUnit, const char*>& GetUnitToSuffixMap();
};

inline bool
Numeric::TryParse(const char* str, Numeric& numeric_out, size_t* count_out)
{
    if (str == nullptr || *str == '\0') return false;

    // [+-]?(\d+(\.(\d+)?)?)|(\.\d+)?(\w+|%)
    auto s = str;
    auto start = s;

    // Sign
    bool negative = false;
    if (*s == '+' || *s == '-') {
        negative = (*s == '-');
        ++s;
    }

    if (*s != '.' && !isdigit(*s)) return false;

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
            if (denominator != 0) return false;
            denominator = base;
        } else {
            break;
        }
    }
    if (s == digit_start) return false;

    // Unit
    auto unit = NumericUnit::NaN;
    for (auto& pair : GetUnitToSuffixMap()) {
        auto len = strlen(pair.second);
        if (strncmp(pair.second, s, len) == 0 &&
            (*(s + len) == '\0' || !isalnum(*(s + len)))) {
            unit = pair.first;
            s += len;
            break;
        }
    }
    if (unit == NumericUnit::NaN) return false;

    if (negative) value = -value;

    numeric_out.value_ = value;
    numeric_out.unit_ = unit;
    numeric_out.float_ = (denominator != 0);
    if (count_out != nullptr) *count_out = static_cast<size_t>(s - start);

    return true;
}

inline const std::map<NumericUnit, const char*>&
Numeric::GetUnitToSuffixMap()
{
    static const std::map<NumericUnit, const char*> kUnitToSuffix = {
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
    return kUnitToSuffix;
}

inline double
Numeric::ToLength(double view_width, double view_height, double pixel_scale, double base_length, double default_value) const
{
    return
        (unit_ == NumericUnit::Pixel) ? value_ :
        (unit_ == NumericUnit::ScaledPixel) ? value_ * pixel_scale :
        (unit_ == NumericUnit::Vw) ? value_ / 100.0 * view_width :
        (unit_ == NumericUnit::Vh) ? value_ / 100.0 * view_height :
        (unit_ == NumericUnit::Vmax) ? value_ / 100.0 * std::max(view_width, view_height) :
        (unit_ == NumericUnit::Vmin) ? value_ / 100.0 * std::min(view_width, view_height) :
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0 * base_length :
        (unit_ == NumericUnit::Unitless) ? value_ * base_length :
        default_value;
}

inline double
Numeric::ToRatio(double default_value) const
{
    return
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0 :
        (unit_ == NumericUnit::Unitless) ? value_ :
        default_value;
}

inline double
Numeric::ToMilliseconds(double default_value) const
{
    return
        (unit_ == NumericUnit::Second) ? value_ * 1000.0 :
        (unit_ == NumericUnit::Millisecond) ? value_ :
        default_value;
}

inline std::string
Numeric::ToString(const char* format) const
{
    (void)format;
    if (unit_ == NumericUnit::NaN) return "";
    if (float_) {
        return StringUtils::Format("%.3Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    } else {
        return StringUtils::Format("%.0Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    }
}

} // namespace miso

#endif // MISO_NUMERIC_H_
