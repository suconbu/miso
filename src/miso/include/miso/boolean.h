#ifndef MISO_BOOLEAN_H_
#define MISO_BOOLEAN_H_

#include <cmath>
#include <map>
#include <string>

#include "miso/string_utils.h"

namespace miso {

class Boolean {
public:
    static bool TryParse(const char* str, Boolean& boolean_out, size_t* count_out = nullptr);

    Boolean() = default;
    explicit Boolean(const char* str) : Boolean() { Boolean::TryParse(str, *this); }
    explicit Boolean(const std::string& str) : Boolean(str.c_str()) {}

    bool IsValid() const { return !std::isnan(value_); }
    bool IsTrue() const { return !std::isnan(value_) && 0.5f <= value_; }

    std::string ToString(const char* format = nullptr) const;

private:
    static constexpr float kNaN = std::numeric_limits<float>::quiet_NaN();

    float value_ = kNaN;
};

inline bool
Boolean::TryParse(const char* str, Boolean& numeric_out, size_t* count_out)
{
    if (str == nullptr || *str == '\0') return false;

    auto s = str;
    auto start = s;

    while (isalpha(*s)) ++s;

    auto count = static_cast<size_t>(s - start);
    if (count < 2) return false;

    const char* names[] = { "true", "false", "on", "off", "yes", "no" };
    float value = kNaN;
    for (int i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (StringUtils::CompareIgnoreCase(start, names[i], strlen(names[i])) == 0) {
            value = ((i % 2) == 0) ? 1.0f : 0.0f;
            break;
        }
    }
    if (std::isnan(value)) return false;

    numeric_out.value_ = value;
    if (count_out != nullptr) *count_out = count;

    return true;
}

inline std::string
Boolean::ToString(const char* format) const
{
    (void)format;
    return (value_ != 0) ? "true" : "false";
}

} // namespace miso

#endif // MISO_BOOLEAN_H_
