#ifndef MISO_BOOLEAN_HPP_
#define MISO_BOOLEAN_HPP_

#include "miso/common.h"

#include <string>

#include "miso/interpolator.hpp"

namespace miso {

class Boolean {
public:
    static const Boolean& GetInvalid();
    static Boolean TryParse(const char* str, size_t* consumed_out = nullptr);

    explicit Boolean(const char* str) : Boolean() { *this = Boolean::TryParse(str); }
    explicit Boolean(const std::string& str) : Boolean(str.c_str()) {}
    explicit Boolean(bool value) : value_(value), valid_(true) {}

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

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "boolean.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_BOOLEAN_HPP_
