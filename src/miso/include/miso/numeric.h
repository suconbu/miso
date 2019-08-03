#ifndef MISO_CORE_NUMERIC_H_
#define MISO_CORE_NUMERIC_H_

#include <ctype.h>
#include <limits>
#include <map>
#include <string>
#include <vector>
#include <cassert>

#include "miso/scalar.h"
#include "miso/color.h"
#include "miso/string.h"

namespace miso {

class Numeric {
public:
    static std::vector<Numeric> Parse(const char* str);

    Numeric() : type_(NumericType::Unknown) {}
    explicit Numeric(const char* str);
    explicit Numeric(const std::string& str) : Numeric(str.c_str()) {}

    const Scalar& GetScalar() const { return (type_ == NumericType::Scalar) ? scalar_ : Scalar::GetInvalid(); }
    const Color& GetColor() const { return (type_ == NumericType::Color) ? color_ : Color::GetInvalid(); }
    bool IsValid() const;
    double ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value = Scalar::kNaN) const;
    double ToRatio(float default_value = Scalar::kNaN) const;
    double ToMilliseconds(float default_value = Scalar::kNaN) const;
    std::string ToString(const char* format = nullptr) const;

private:
    enum class NumericType { Unknown, Scalar, Color };

    NumericType type_;
    union {
        Scalar scalar_;
        Color color_;
    };
};

inline std::vector<Numeric>
Numeric::Parse(const char* str)
{
    std::vector<Numeric> numerics;
    for (auto& token : StringUtils::Split(str)) {
        numerics.push_back(Numeric(token));
    }
    return numerics;
}

inline bool
Numeric::IsValid() const
{
    return
        (type_ == NumericType::Scalar) ? scalar_.IsValid() :
        (type_ == NumericType::Color) ? color_.IsValid() :
        false;
}

inline
Numeric::Numeric(const char* str) : Numeric()
{
    if (Color::TryParse(str, color_) > 0) {
        type_ = NumericType::Color;
    } else if (Scalar::TryParse(str, scalar_) > 0) {
        type_ = NumericType::Scalar;
    } else {
        type_ = NumericType::Unknown;
    }
}

inline double
Numeric::ToLength(float view_width, float view_height, float pixel_scale, double base_length, double default_value) const
{
    return (type_ == NumericType::Scalar) ?
        scalar_.ToLength(view_width, view_height, pixel_scale, base_length, default_value) :
        default_value;
}

inline double
Numeric::ToRatio(float default_value) const
{
    return (type_ == NumericType::Scalar) ?
        scalar_.ToRatio(default_value) :
        default_value;
}

inline double
Numeric::ToMilliseconds(float default_value) const
{
    return (type_ == NumericType::Scalar) ?
        scalar_.ToMilliseconds(default_value) :
        default_value;
}

inline std::string
Numeric::ToString(const char* format) const
{
    return
        (type_ == NumericType::Scalar) ? scalar_.ToString(format) :
        (type_ == NumericType::Color) ? color_.ToString(format) :
        "";
}

} // namespace miso

#endif // MISO_CORE_NUMERIC_H_
