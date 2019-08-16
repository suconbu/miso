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
    static const Boolean& GetInvalid() { const static Boolean invalid; return invalid; }
    static Boolean TryParse(const char* str, size_t* consumed_out = nullptr);

    explicit Boolean(const char* str) : Boolean() { *this = Boolean::TryParse(str); }
    explicit Boolean(const std::string& str) : Boolean(str.c_str()) {}
    explicit Boolean(bool value) : value_(value) {}

    bool operator==(const Boolean& other) const { return IsTrue() == other.IsTrue(); }
    bool operator!=(const Boolean& other) const { return !(*this == other); }

    bool IsValid() const { return valid_; }
    bool IsTrue() const { return valid_ && value_; }
    Boolean GetInterpolated(const Boolean& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

private:
    Boolean() = default;

    bool value_ = false;
    bool valid_ = false;
};

inline Boolean
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
