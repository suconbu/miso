#ifndef MISO_NUMERIC_HPP_
#define MISO_NUMERIC_HPP_

#include "miso/common.h"

#include <map>
#include <string>

#include "miso/interpolator.hpp"

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
    static const Numeric& GetInvalid();
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

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "numeric.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_NUMERIC_HPP_
