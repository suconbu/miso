#ifndef MISO_NUMERIC_H_
#define MISO_NUMERIC_H_

#include <map>
#include <string>

#include "miso/interpolator.h"
#include "miso/string_utils.h"

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
    static const Numeric& GetInvalid() { const static Numeric invalid; return invalid; }
    static Numeric TryParse(const char* str, size_t* consumed_out = nullptr);

    explicit Numeric(const char* str) : Numeric() { *this = Numeric::TryParse(str); }
    explicit Numeric(const std::string& str) : Numeric(str.c_str()) {}
    explicit Numeric(double value, NumericUnit unit) : value_(value), unit_(unit), float_(value_ != floor(value_)) {}

    bool operator==(const Numeric& other) const;
    bool operator!=(const Numeric& other) const;

    bool IsValid() const { return unit_ != NumericUnit::NaN; }
    bool IsFloat() const { return float_; }
    double GetValue() const { return value_; }
    NumericUnit GetUnit() const { return unit_; }
    template<typename T> T ToLength(float view_width, float view_height, float pixel_scale, float base_length, T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    template<typename T> T ToRatio(T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    template<typename T> T ToMilliseconds(T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    Numeric GetInterpolated(const Numeric& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

private:
    static const std::map<NumericUnit, const char*>& GetUnitToSuffixMap();

    Numeric() = default;

    double value_ = std::numeric_limits<double>::quiet_NaN();
    NumericUnit unit_ = NumericUnit::NaN;
    bool float_ = false;
};

inline Numeric
Numeric::TryParse(const char* str, size_t* consumed_out)
{
    if (str == nullptr || *str == '\0') return GetInvalid();

    // [+-]?(\d+(\.(\d+)?)?)|(\.\d+)?(\w+|%)
    auto s = str;
    auto start = s;

    // Sign
    bool negative = false;
    if (*s == '+' || *s == '-') {
        negative = (*s == '-');
        ++s;
    }

    if (*s != '.' && !isdigit(*s)) return GetInvalid();

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
            if (denominator != 0) return GetInvalid();
            denominator = base;
        } else {
            break;
        }
    }
    if (s == digit_start) return GetInvalid();

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
    if (unit == NumericUnit::NaN) return GetInvalid();

    if (negative) value = -value;

    Numeric numeric;
    numeric.value_ = value;
    numeric.unit_ = unit;
    numeric.float_ = (denominator != 0);
    if (consumed_out != nullptr) *consumed_out = static_cast<size_t>(s - start);

    return numeric;
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

inline bool
Numeric::operator==(const Numeric& other) const
{
    return (unit_ == other.unit_) && (value_ == other.value_);
}

inline bool
Numeric::operator!=(const Numeric& other) const
{
    return !(*this == other);
}

template<typename T> inline T
Numeric::ToLength(float view_width, float view_height, float pixel_scale, float base_length, T default_value) const
{
    return static_cast<T>(
        (unit_ == NumericUnit::Pixel) ? value_ :
        (unit_ == NumericUnit::ScaledPixel) ? value_ * pixel_scale :
        (unit_ == NumericUnit::Vw) ? value_ / 100.0 * view_width :
        (unit_ == NumericUnit::Vh) ? value_ / 100.0 * view_height :
        (unit_ == NumericUnit::Vmax) ? value_ / 100.0 * std::max(view_width, view_height) :
        (unit_ == NumericUnit::Vmin) ? value_ / 100.0 * std::min(view_width, view_height) :
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0 * base_length :
        (unit_ == NumericUnit::Unitless) ? value_ * base_length :
        default_value);
}

template<typename T> inline T
Numeric::ToRatio(T default_value) const
{
    return static_cast<T>(
        (unit_ == NumericUnit::Parcent) ? value_ / 100.0 :
        (unit_ == NumericUnit::Unitless) ? value_ :
        default_value);
}

template<typename T> inline T
Numeric::ToMilliseconds(T default_value) const
{
    return static_cast<T>(
        (unit_ == NumericUnit::Second) ? value_ * 1000.0 :
        (unit_ == NumericUnit::Millisecond) ? value_ :
        default_value);
}

inline Numeric
Numeric::GetInterpolated(const Numeric& end_value, const Interpolator& interpolator, float progress) const
{
    return Numeric(interpolator.Interpolate(static_cast<float>(value_), static_cast<float>(end_value.value_), progress), unit_);
}

inline std::string
Numeric::ToString(const char* format) const
{
    (void)format;
    if (unit_ == NumericUnit::NaN) return "";
    if (IsFloat()) {
        return StringUtils::Format("%.3Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    } else {
        return StringUtils::Format("%.0Lf%s", value_, GetUnitToSuffixMap().at(unit_));
    }
}

} // namespace miso

#endif // MISO_NUMERIC_H_
