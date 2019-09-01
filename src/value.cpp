#include "miso/value.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include "miso/color.hpp"
#include "miso/numeric.hpp"
#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE const Value&
Value::GetInvalid()
{
    static const Value invalid;
    return invalid;
}

MISO_INLINE Value
Value::TryParse(const char* str, size_t* consumed_out)
{
    Value value;
    if ((value.numeric_ = Numeric::TryParse(str, consumed_out)).IsValid()) {
        value.type_ = ValueType::Numeric;
    } else if ((value.color_ = Color::TryParse(str, consumed_out)).IsValid()) {
        value.type_ = ValueType::Color;
    }
    return value;
}

MISO_INLINE
Value::Value(const Value& other)
{
    type_ = other.type_;
    if (type_ == ValueType::Array) {
        array_ = new std::vector<Value>(*other.array_);
        if (array_ == nullptr) {
            type_ = ValueType::Invalid;
        }
    } else if (type_ == ValueType::Numeric) {
        numeric_ = other.numeric_;
    } else if (type_ == ValueType::Color) {
        color_ = other.color_;
    }
}

MISO_INLINE
Value::Value(Value&& other) noexcept
{
    type_ = other.type_;
    if (type_ == ValueType::Array) {
        array_ = new std::vector<Value>(std::move(*other.array_));
        if (array_ == nullptr) {
            type_ = ValueType::Invalid;
        }
    } else if (type_ == ValueType::Numeric) {
        numeric_ = other.numeric_;
    } else if (type_ == ValueType::Color) {
        color_ = other.color_;
    }
}

MISO_INLINE Value&
Value::operator=(Value other)
{
    if (other.type_ == ValueType::Array) {
        std::swap(array_, other.array_);
    } else if (other.type_ == ValueType::Numeric) {
        std::swap(numeric_, other.numeric_);
    } else if (other.type_ == ValueType::Color) {
        std::swap(color_, other.color_);
    }
    std::swap(type_, other.type_);
    return *this;
}

MISO_INLINE
Value::Value(const char* str) : Value()
{
    std::vector<Value> values;
    values.reserve(kInitialArrayCapacityOnParse);

    auto s = str;
    for (; *s != '\0'; ++s) {
        if (!isspace(*s)) {
            size_t count;
            auto value = TryParse(s, &count);
            if (!value.IsValid()) break;
            values.push_back(std::move(value));
            s += (count - 1);
        }
    }

    if (*s != '\0') {
        ;
    } else if (values.size() == 1) {
        *this = values.at(0);
    } else if (values.size() > 1) {
        array_ = new std::vector<Value>(std::move(values));
        type_ = (array_ != nullptr) ? ValueType::Array : ValueType::Invalid;
    } else {
        ;
    }
}

MISO_INLINE
Value::~Value()
{
    if (type_ == ValueType::Array) {
        delete array_;
    }
}

MISO_INLINE bool
Value::operator==(const Value& other) const
{
    if (type_ != other.type_) return false;

    if (type_ == ValueType::Array) {
        if (array_->size() != other.array_->size()) return false;
        for (size_t i = 0; i < array_->size(); ++i) {
            if (GetAt(i) != other.GetAt(i)) return false;
        }
        return true;
    } else if (type_ == ValueType::Numeric) {
        return numeric_ == other.numeric_;
    } else if (type_ == ValueType::Color) {
        return color_ == other.color_;
    } else {
        return false;
    }
}

MISO_INLINE bool
Value::operator!=(const Value& other) const
{
    return !(*this == other);
}

Value
Value::operator*(double multiplier) const
{
    Value value;
    value.type_ = type_;
    if (type_ == ValueType::Array) {
        value.array_ = new std::vector<Value>();
        if (value.array_ == nullptr) return Value::GetInvalid();
        value.array_->reserve(array_->size());
        for (size_t i = 0; i < array_->size(); ++i) {
            value.array_->push_back(array_->at(i) * multiplier);
        }
    } else if (type_ == ValueType::Numeric) {
        value.numeric_ = numeric_ * multiplier;
    } else if (type_ == ValueType::Color) {
        value.color_ = color_ * multiplier;
    } else {
        ;
    }
    return value;
}

MISO_INLINE bool
Value::IsValid() const
{
    return
        (type_ == ValueType::Numeric) ? numeric_.IsValid() :
        (type_ == ValueType::Color) ? color_.IsValid() :
        (type_ == ValueType::Array) ? true :
        false;
}

MISO_INLINE bool
Value::IsTrue() const
{
    return
        (type_ == ValueType::Numeric) ? numeric_.IsTrue() :
        (type_ == ValueType::Color) ? color_.IsTrue() :
        (type_ == ValueType::Array) ? true :
        false;
}

MISO_INLINE const Value&
Value::GetAt(size_t index) const
{
    return
        (type_ == ValueType::Array && index < array_->size()) ? array_->at(index) :
        (type_ != ValueType::Array && index == 0) ? *this :
        GetInvalid();
}

MISO_INLINE ValueType
Value::GetType(size_t index)
{
    auto type = ValueType::Invalid;
    if (type_ == ValueType::Invalid) {
        ;
    } else if (type_ == ValueType::Array) {
        type =
            (index == SIZE_MAX) ? ValueType::Array :
            (index < array_->size()) ? array_->at(index).type_ :
            ValueType::Invalid;
    } else {
        type =
            (index == SIZE_MAX || index == 0) ? type_ :
            ValueType::Invalid;
    }
    return type;
}

MISO_INLINE Value
Value::Added(const Value& value) const
{
    Value v;
    if (type_ == ValueType::Array) {
        v = *this;
    } else {
        v.type_ = ValueType::Array;
        v.array_ = new std::vector<Value>();
        if (v.array_ == nullptr) return GetInvalid();
        v.array_->push_back(*this);
    }
    v.array_->push_back(value);
    return v;
}

MISO_INLINE const Numeric&
Value::AsNumeric(size_t index) const
{
    auto& v = GetAt(index);
    return (v.type_ == ValueType::Numeric) ? v.numeric_ : Numeric::GetInvalid();
}

MISO_INLINE const Color&
Value::AsColor(size_t index) const
{
    auto& v = GetAt(index);
    return (v.type_ == ValueType::Color) ? v.color_ : Color::GetInvalid();
}

MISO_INLINE Value
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
        for (size_t i = 0; i < array_->size(); ++i) {
            value.array_->push_back(array_->at(i).GetInterpolated(end_value.array_->at(i), interpolator, progress));
        }
    } else if (type_ == ValueType::Numeric) {
        value.numeric_ = numeric_.GetInterpolated(end_value.numeric_, interpolator, progress);
    } else if (type_ == ValueType::Color) {
        value.color_ = color_.GetInterpolated(end_value.color_, interpolator, progress);
    } else {
        ;
    }
    return value;
}

MISO_INLINE std::string
Value::ToString(const char* format) const
{
    if (type_ == ValueType::Array) {
        std::stringstream s;
        for (size_t i = 0; i < array_->size() - 1; ++i) {
            s << array_->at(i).ToString(format) << " ";
        }
        s << array_->at(array_->size() - 1).ToString(format);
        return s.str();
    } else {
        return
            (type_ == ValueType::Numeric) ? numeric_.ToString(format) :
            (type_ == ValueType::Color) ? color_.ToString(format) :
            "";
    }
}

} // namespace miso
