#include "miso/boolean.hpp"

#include <string>

#include "miso/interpolator.hpp"
#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE const Boolean&
Boolean::GetInvalid()
{
    const static Boolean invalid;
    return invalid;
}

MISO_INLINE Boolean
Boolean::TryParse(const char* str, size_t* consumed_out)
{
    if (str == nullptr || *str == '\0') return GetInvalid();

    auto s = str;
    auto start = s;

    while (isalpha(*s)) ++s;

    auto count = static_cast<size_t>(s - start);
    if (count < 2) return GetInvalid();

    const char* names[] = { "true", "false", "on", "off", "yes", "no" };
    bool value = false;
    bool valid = false;
    for (int i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (StringUtils::CompareN(start, names[i], strlen(names[i]), true) == 0) {
            value = (i % 2) == 0;
            valid = true;
            break;
        }
    }
    if (!valid) return GetInvalid();

    Boolean boolean;
    boolean.value_ = value;
    boolean.valid_ = valid;
    if (consumed_out != nullptr) *consumed_out = count;

    return boolean;
}

MISO_INLINE Boolean
Boolean::GetInterpolated(const Boolean& end_value, const Interpolator& interpolator, float progress) const
{
    float start = value_ ? 1.0f : 0.0f;
    float end = end_value.value_ ? 1.0f : 0.0f;
    return Boolean(interpolator.Interpolate(start, end, progress));
}

MISO_INLINE std::string
Boolean::ToString(const char* format) const
{
    (void)format;
    return value_ ? "true" : "false";
}

} // namespace miso
