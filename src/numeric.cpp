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
    Numeric numeric;
    if (str == nullptr || *str == '\0') return numeric;
    if (TryParseBoolean(str, &numeric, consumed_out)) return numeric;
    if (TryParseNumeric(str, &numeric, consumed_out)) return numeric;
    return numeric;
}

MISO_INLINE bool
Numeric::TryParseBoolean(const char* str, Numeric* numeric_out, size_t* consumed_out)
{
    static const char* names[] = { "true", "false", "on", "off", "yes", "no" };

    auto s = str;
    auto start = s;

    while (isalpha(*s)) ++s;
    auto count = static_cast<size_t>(s - start);
    if (count < 2) return false;

    auto value = std::numeric_limits<double>::quiet_NaN();
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (StringUtils::CompareN(start, names[i], strlen(names[i]), true) == 0) {
            value = ((i % 2) == 0) ? 1.0 : 0.0;
            break;
        }
    }
    if (std::isnan(value)) return false;

    if (numeric_out != nullptr) {
        numeric_out->value_ = value;
        numeric_out->unit_ = NumericUnit::Unitless;
        numeric_out->float_ = false;
    }
    if (consumed_out != nullptr) {
        *consumed_out = static_cast<size_t>(s - start);
    }

    return true;
}

MISO_INLINE bool
Numeric::TryParseNumeric(const char* str, Numeric* numeric_out, size_t* consumed_out)
{
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

    if (numeric_out != nullptr) {
        numeric_out->value_ = negative ? -value : value;
        numeric_out->unit_ = unit;
        numeric_out->float_ = (denominator != 0);
    }
    if (consumed_out != nullptr) {
        *consumed_out = static_cast<size_t>(s - start);
    }

    return true;
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
