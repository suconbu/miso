#ifndef MISO_BOOLEAN_H_
#define MISO_BOOLEAN_H_

#include <cmath>
#include <map>
#include <string>

#include "miso/interpolator.h"
#include "miso/string_utils.h"

namespace miso {

class Boolean {
public:
    static bool TryParse(const char* str, Boolean& boolean_out, size_t* count_out = nullptr);

    Boolean() = default;
    explicit Boolean(const char* str) : Boolean() { Boolean::TryParse(str, *this); }
    explicit Boolean(const std::string& str) : Boolean(str.c_str()) {}
    explicit Boolean(bool value) : value_(value) {}

    bool IsValid() const { return valid_; }
    bool IsTrue() const { return valid_ && value_; }
    Boolean GetInterpolated(const Boolean& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

private:
    bool value_ = false;
    bool valid_ = false;
};

inline bool
Boolean::TryParse(const char* str, Boolean& boolean_out, size_t* count_out)
{
    if (str == nullptr || *str == '\0') return false;

    auto s = str;
    auto start = s;

    while (isalpha(*s)) ++s;

    auto count = static_cast<size_t>(s - start);
    if (count < 2) return false;

    const char* names[] = { "true", "false", "on", "off", "yes", "no" };
    bool value = false;
    bool valid = false;
    for (int i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (StringUtils::CompareIgnoreCase(start, names[i], strlen(names[i])) == 0) {
            value = (i % 2) == 0;
            valid = true;
            break;
        }
    }
    if (!valid) return false;

    boolean_out.value_ = value;
    boolean_out.valid_ = valid;
    if (count_out != nullptr) *count_out = count;

    return true;
}

inline Boolean
Boolean::GetInterpolated(const Boolean& end_value, const Interpolator& interpolator, float progress) const
{
    float start = value_ ? 1.0f : 0.0f;
    float end = end_value.value_ ? 1.0f : 0.0f;
    return Boolean(interpolator.Interpolate(start, end, progress));
}

inline std::string
Boolean::ToString(const char* format) const
{
    (void)format;
    return value_ ? "true" : "false";
}

} // namespace miso

#endif // MISO_BOOLEAN_H_
