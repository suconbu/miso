#ifndef MISO_VALUE_H_
#define MISO_VALUE_H_

#include <string>
#include <vector>

#include "miso/color.h"
#include "miso/numeric.h"
#include "miso/string.h"

namespace miso {

class Value {
public:
    static std::vector<Value> FromString(const char* str);
    static std::vector<Value> FromString(const std::string& str) { return FromString(str.c_str()); }
    static bool TryParse(const char* str, Value& value_out, size_t* count_out = nullptr);

    Value() : type_(ValueType::Unknown) {}
    explicit Value(const char* str);
    explicit Value(const std::string& str) : Value(str.c_str()) {}

    bool IsValid() const;
    const Numeric& GetNumeric() const { return (type_ == ValueType::Numeric) ? numeric_ : Numeric::GetInvalid(); }
    const Color& GetColor() const { return (type_ == ValueType::Color) ? color_ : Color::GetInvalid(); }
    double ToLength(double view_width, double view_height, double pixel_scale, double base_length, double default_value = Numeric::kNaN) const;
    double ToRatio(double default_value = Numeric::kNaN) const;
    double ToMilliseconds(double default_value = Numeric::kNaN) const;
    std::string ToString(const char* format = nullptr) const;

private:
    enum class ValueType { Unknown, Numeric, Color };

    ValueType type_;
    union {
        Numeric numeric_;
        Color color_;
    };
};

inline std::vector<Value>
Value::FromString(const char* str)
{
    std::vector<Value> values;
    auto s = str;
    for (; *s != '\0'; ++s) {
        if (!isspace(*s)) {
            Value value;
            size_t count;
            if (!TryParse(s, value, &count)) return std::vector<Value>();
            values.push_back(value);
            s += (count - 1);
        }
    }
    return values;
}

inline bool
Value::TryParse(const char* str, Value& value_out, size_t* count_out)
{
    if (Color::TryParse(str, value_out.color_, count_out)) {
        value_out.type_ = ValueType::Color;
        return true;
    }
    if (Numeric::TryParse(str, value_out.numeric_, count_out)) {
        value_out.type_ = ValueType::Numeric;
        return true;
    }
    return false;
}

inline bool
Value::IsValid() const
{
    return
        (type_ == ValueType::Numeric) ? numeric_.IsValid() :
        (type_ == ValueType::Color) ? color_.IsValid() :
        false;
}

inline
Value::Value(const char* str) : Value()
{
    TryParse(str, *this);
}

inline double
Value::ToLength(double view_width, double view_height, double pixel_scale, double base_length, double default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToLength(view_width, view_height, pixel_scale, base_length, default_value) :
        default_value;
}

inline double
Value::ToRatio(double default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToRatio(default_value) :
        default_value;
}

inline double
Value::ToMilliseconds(double default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToMilliseconds(default_value) :
        default_value;
}

inline std::string
Value::ToString(const char* format) const
{
    return
        (type_ == ValueType::Numeric) ? numeric_.ToString(format) :
        (type_ == ValueType::Color) ? color_.ToString(format) :
        "";
}

} // namespace miso

#endif // MISO_VALUE_H_
