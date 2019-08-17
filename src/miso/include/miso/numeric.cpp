#include "miso/numeric.hpp"

#include <map>
#include <string>

#include "miso/interpolator.hpp"
#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE const Numeric&
Numeric::GetInvalid()
{
    static const Numeric invalid;
    return invalid;
}

MISO_INLINE Numeric
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

MISO_INLINE const std::map<NumericUnit, const char*>&
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

MISO_INLINE bool
Numeric::operator==(const Numeric& other) const
{
    return (unit_ == other.unit_) && (value_ == other.value_);
}

MISO_INLINE bool
Numeric::operator!=(const Numeric& other) const
{
    return !(*this == other);
}

MISO_INLINE Numeric
Numeric::GetInterpolated(const Numeric& end_value, const Interpolator& interpolator, float progress) const
{
    return Numeric(interpolator.Interpolate(static_cast<float>(value_), static_cast<float>(end_value.value_), progress), unit_);
}

MISO_INLINE std::string
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
