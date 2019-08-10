#ifndef MISO_VALUE_H_
#define MISO_VALUE_H_

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "miso/boolean.h"
#include "miso/color.h"
#include "miso/numeric.h"
#include "miso/string_utils.h"

namespace miso {

class Value {
public:
    static const Value& GetInvalid() { static const Value invalid; return invalid; }

    Value() {};
    Value(const Value& value) { Copy(value, *this); }
    Value(Value&& value) { Move(value, *this); }
    explicit Value(const char* str);
    explicit Value(const std::string& str) : Value(str.c_str()) {}
    ~Value() { if (type_ == ValueType::Array) delete array_; }
    Value& operator=(const Value& value) { Copy(value, *this); return *this; }

    bool IsValid() const;
    bool IsTrue() const;
    size_t GetCount() const { return (type_ == ValueType::Array) ? array_->size() : 1; }
    const Value& GetAt(size_t index) const;
    const Value& operator[](size_t index) { return GetAt(index); }
    const Numeric& GetNumeric() const { return (type_ == ValueType::Numeric) ? numeric_ : Numeric::GetInvalid(); }
    const Color& GetColor() const { return (type_ == ValueType::Color) ? color_ : Color::GetInvalid(); }
    template<typename T> T ToLength(float view_width, float view_height, float pixel_scale, float base_length, T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    template<typename T> T ToRatio(T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    template<typename T> T ToMilliseconds(T default_value = std::numeric_limits<T>::quiet_NaN()) const;
    std::string ToString(const char* format = nullptr) const;

private:
    enum class ValueType { Invalid, Array, Boolean, Numeric, Color };
    static constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

    ValueType type_ = ValueType::Invalid;
    union {
        std::vector<Value>* array_ = 0;
        Boolean boolean_;
        Numeric numeric_;
        Color color_;
    };

    static bool TryParse(const char* str, Value& value_out, size_t* count_out = nullptr);
    static void Copy(const Value& from, Value& to);
    static void Move(Value& from, Value& to);
};

inline bool
Value::TryParse(const char* str, Value& value_out, size_t* count_out)
{
    if (Boolean::TryParse(str, value_out.boolean_, count_out)) {
        value_out.type_ = ValueType::Boolean;
        return true;
    }
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

inline void
Value::Copy(const Value& from, Value& to)
{
    to.type_ = from.type_;
    if (from.type_ == ValueType::Array) {
        to.array_ = new std::vector<Value>();
        if (to.array_ != nullptr) {
            to.array_->reserve(from.array_->size());
            std::copy(from.array_->begin(), from.array_->end(), std::back_inserter(*to.array_));
        } else {
            to.type_ = ValueType::Invalid;
        }
    } else if (from.type_ == ValueType::Boolean) {
        to.boolean_ = from.boolean_;
    } else if (from.type_ == ValueType::Numeric) {
        to.numeric_ = from.numeric_;
    } else if (from.type_ == ValueType::Color) {
        to.color_ = from.color_;
    } else {
        ;
    }
}

inline void
Value::Move(Value& from, Value& to)
{
    to.type_ = from.type_;
    if (from.type_ == ValueType::Array) {
        to.array_ = from.array_;
        from.array_ = nullptr;
    } else if (from.type_ == ValueType::Boolean) {
        to.boolean_ = from.boolean_;
    } else if (from.type_ == ValueType::Numeric) {
        to.numeric_ = from.numeric_;
    } else if (from.type_ == ValueType::Color) {
        to.color_ = from.color_;
    } else {
        ;
    }
}

inline bool
Value::IsValid() const
{
    return
        (type_ == ValueType::Boolean) ? boolean_.IsValid() :
        (type_ == ValueType::Numeric) ? numeric_.IsValid() :
        (type_ == ValueType::Color) ? color_.IsValid() :
        (type_ == ValueType::Array) ? true :
        false;
}

inline bool
Value::IsTrue() const
{
    return
        (type_ == ValueType::Boolean) ? boolean_.IsTrue() :
        (type_ == ValueType::Numeric) ? (numeric_.IsValid() && numeric_.GetValue() != 0.0) :
        (type_ == ValueType::Color) ? (color_.IsValid() && color_.ToUint32() != 0) :
        (type_ == ValueType::Array) ? true :
        false;
}

inline const Value&
Value::GetAt(size_t index) const
{
    return
        (type_ == ValueType::Array && index < array_->size()) ? array_->at(index) :
        (type_ != ValueType::Array) ? *this :
        GetInvalid();
}

inline
Value::Value(const char* str) : Value()
{
    auto values = new std::vector<Value>();
    if (values == nullptr) return;

    values->reserve(10);
    auto s = str;
    for (; *s != '\0'; ++s) {
        if (!isspace(*s)) {
            Value value;
            size_t count;
            if (!TryParse(s, value, &count)) break;
            values->push_back(std::move(value));
            s += (count - 1);
        }
    }
    if (*s == '\0') {
        if (values->size() > 1) {
            type_ = ValueType::Array;
            array_ = values;
            values = nullptr;
        } else if (values->size() == 1) {
            Copy(values->at(0), *this);
            delete values;
            values = nullptr;
        } else {
            ;
        }
    }
}

template<typename T> inline T
Value::ToLength(float view_width, float view_height, float pixel_scale, float base_length, T default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToLength(view_width, view_height, pixel_scale, base_length, default_value) :
        default_value;
}

template<typename T> inline T
Value::ToRatio(T default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToRatio(default_value) :
        default_value;
}

template<typename T> inline T
Value::ToMilliseconds(T default_value) const
{
    return (type_ == ValueType::Numeric) ?
        numeric_.ToMilliseconds(default_value) :
        default_value;
}

inline std::string
Value::ToString(const char* format) const
{
    return
        (type_ == ValueType::Boolean) ? boolean_.ToString(format) :
        (type_ == ValueType::Numeric) ? numeric_.ToString(format) :
        (type_ == ValueType::Color) ? color_.ToString(format) :
        "";
}

} // namespace miso

#endif // MISO_VALUE_H_
