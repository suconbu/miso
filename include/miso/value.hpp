#ifndef MISO_VALUE_HPP_
#define MISO_VALUE_HPP_

#include "miso/common.h"

#include <string>
#include <vector>

#include "miso/color.hpp"
#include "miso/numeric.hpp"

namespace miso {

class Value {
public:
    static const Value& GetInvalid();

    Value() {};
    Value(const Value& value) { Copy(value, *this); }
    Value(Value&& value) noexcept { Move(value, *this); }
    explicit Value(const char* str);
    explicit Value(const std::string& str) : Value(str.c_str()) {}
    explicit Value(const Numeric& numeric) : type_(ValueType::Numeric), numeric_(numeric) {}
    explicit Value(const Color& color) : type_(ValueType::Color), color_(color) {}
    ~Value();

    Value& operator=(const Value& value) { Copy(value, *this); return *this; }
    const Value& operator[](size_t index) { return GetAt(index); }
    bool operator==(const Value& other) const;
    bool operator!=(const Value& value) const;
    Value operator*(double multiplier) const;

    bool IsValid() const;
    bool IsTrue() const;
    size_t GetCount() const { return (type_ == ValueType::Array) ? array_->size() : 1; }
    const Value& GetAt(size_t index) const;
    const Numeric& AsNumeric(size_t index = 0) const;
    const Color& AsColor(size_t index = 0) const;
    Value GetInterpolated(const Value& end_value, const Interpolator& interpolator, float progress) const;
    std::string ToString(const char* format = nullptr) const;

private:
    enum class ValueType { Invalid, Array, Numeric, Color };

    static Value TryParse(const char* str, size_t* consumed_out = nullptr);
    static void Copy(const Value& from, Value& to);
    static void Move(Value& from, Value& to);

    void Dispose() { if (type_ == ValueType::Array) delete array_; }

    ValueType type_ = ValueType::Invalid;
    union {
        std::vector<Value>* array_ = 0;
        Numeric numeric_;
        Color color_;
    };
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "value.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_VALUE_HPP_
