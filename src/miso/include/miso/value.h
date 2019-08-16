#ifndef MISO_VALUE_H_
#define MISO_VALUE_H_

#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
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
    Value(Value&& value) noexcept { Move(value, *this); }
    explicit Value(const char* str);
    explicit Value(const std::string& str) : Value(str.c_str()) {}
    ~Value() { Dispose(); }

    Value& operator=(const Value& value) { Copy(value, *this); return *this; }
    const Value& operator[](size_t index) { return GetAt(index); }
    bool operator==(const Value& value) const { return Equal(*this, value); }
    bool operator!=(const Value& value) const { return !Equal(*this, value); }

    bool IsValid() const;
    size_t GetCount() const { return (type_ == ValueType::Array) ? array_->size() : 1; }
    const Value& GetAt(size_t index) const;
    bool AsBool(size_t index = 0) const;
    const Numeric& AsNumeric(size_t index = 0) const;
    const Color& AsColor(size_t index = 0) const;
    Value GetInterpolated(const Value& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

private:
    enum class ValueType { Invalid, Array, Boolean, Numeric, Color };

    static Value TryParse(const char* str, size_t* consumed_out = nullptr);
    static void Copy(const Value& from, Value& to);
    static void Move(Value& from, Value& to);
    static bool Equal(const Value& a, const Value& b);

    void Dispose() { if (type_ == ValueType::Array) delete array_; }

    ValueType type_ = ValueType::Invalid;
    union {
        std::vector<Value>* array_ = 0;
        Boolean boolean_;
        Numeric numeric_;
        Color color_;
    };
};

inline Value
Value::TryParse(const char* str, size_t* consumed_out)
{
    Value value;
    if ((value.boolean_ = Boolean::TryParse(str, consumed_out)).IsValid()) {
        value.type_ = ValueType::Boolean;
    } else if ((value.numeric_ = Numeric::TryParse(str, consumed_out)).IsValid()) {
        value.type_ = ValueType::Numeric;
    } else if ((value.color_ = Color::TryParse(str, consumed_out)).IsValid()) {
        value.type_ = ValueType::Color;
    }
    return value;
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
            size_t count;
            auto value = TryParse(s, &count);
            if (!value.IsValid()) break;
            values->push_back(std::move(value));
            s += (count - 1);
        }
    }

    if (*s != '\0') return;

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

inline void
Value::Copy(const Value& from, Value& to)
{
    to.Dispose();
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
    to.Dispose();
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
Value::Equal(const Value& a, const Value& b)
{
    if (a.type_ != b.type_) return false;

    if (a.type_ == ValueType::Array) {
        if (a.array_->size() != b.array_->size()) return false;
        for (int i = 0; i < a.array_->size(); ++i) {
            if (a.GetAt(i) != b.GetAt(i)) return false;
        }
        return true;
    } else if (a.type_ == ValueType::Boolean) {
        return a.boolean_ == b.boolean_;
    } else if (a.type_ == ValueType::Numeric) {
        return a.numeric_ == b.numeric_;
    } else if (a.type_ == ValueType::Color) {
        return a.color_ == b.color_;
    } else {
        return false;
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

inline const Value&
Value::GetAt(size_t index) const
{
    return
        (type_ == ValueType::Array && index < array_->size()) ? array_->at(index) :
        (type_ != ValueType::Array) ? *this :
        GetInvalid();
}

inline bool
Value::AsBool(size_t index) const
{
    auto v = GetAt(index);
    return (v.type_ == ValueType::Boolean) ? v.boolean_.IsTrue() : false;
}

inline const Numeric&
Value::AsNumeric(size_t index) const
{
    auto v = GetAt(index);
    return (v.type_ == ValueType::Numeric) ? v.numeric_ : Numeric::GetInvalid();
}

inline const Color&
Value::AsColor(size_t index) const
{
    auto v = GetAt(index);
    return (v.type_ == ValueType::Color) ? v.color_ : Color::GetInvalid();
}

inline Value
Value::GetInterpolated(const Value& end_value, const Interpolator& interpolator, float progress) const
{
    Value value;
    if (type_ != end_value.type_) return Value::GetInvalid();
    value.type_ = type_;
    if (type_ == ValueType::Array) {
        if (array_->size() != end_value.array_->size()) return Value::GetInvalid();
        value.array_ = new std::vector<Value>();
        if (value.array_ == nullptr) return Value::GetInvalid();
        value.array_->reserve(array_->size());
        for (int i = 0; i < array_->size(); ++i) {
            value.array_->push_back(array_->at(i).GetInterpolated(end_value.array_->at(i), interpolator, progress));
        }
    } else if (type_ == ValueType::Boolean) {
        value.boolean_ = boolean_.GetInterpolated(end_value.boolean_, interpolator, progress);
    } else if (type_ == ValueType::Numeric) {
        value.numeric_ = numeric_.GetInterpolated(end_value.numeric_, interpolator, progress);
    } else if (type_ == ValueType::Color) {
        value.color_ = color_.GetInterpolated(end_value.color_, interpolator, progress);
    } else {
        ;
    }
    return value;
}

inline std::string
Value::ToString(const char* format) const
{
    if (type_ == ValueType::Array) {
        std::stringstream s;
        for (int i = 0; i < array_->size() - 1; ++i) {
            s << array_->at(i).ToString(format) << " ";
        }
        s << array_->at(array_->size() - 1).ToString(format);
        return s.str();
    } else {
        return
            (type_ == ValueType::Boolean) ? boolean_.ToString(format) :
            (type_ == ValueType::Numeric) ? numeric_.ToString(format) :
            (type_ == ValueType::Color) ? color_.ToString(format) :
            "";
    }
}

} // namespace miso

#endif // MISO_VALUE_H_
