#ifndef MISO_ENUM_HPP_
#define MISO_ENUM_HPP_

#include "miso/common.hpp"

#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>

template <typename T>
class Enum {
public:
    using Type = T;

    static const char* ToString(T);
    static T ToEnum(const char* str);
    static T ToEnum(const std::string& str) { return ToEnum(str.c_str()); }
    static bool TryParse(const char* str, T* enum_out = nullptr);
    static bool TryParse(const std::string& str, T* enum_out = nullptr) { return TryParse(str.c_str(), enum_out); }

    Enum() = delete;
    Enum(const Enum&) = delete;
    Enum(Enum&&) = delete;
};

#define MISO_DEFINE_ENUM(T, ...)                                                        \
                                                                                        \
static const constexpr std::pair<const T, const char *> T##_names[] = __VA_ARGS__;      \
                                                                                        \
template <>                                                                             \
const char* Enum<T>::ToString(T t)                                                      \
{                                                                                       \
    auto it = std::find_if(std::begin(T##_names), std::end(T##_names),                  \
        [&](const auto& v) { return t == v.first; });                                   \
    return (it != std::end(T##_names)) ? it->second : "";                               \
}                                                                                       \
                                                                                        \
template <>                                                                             \
T Enum<T>::ToEnum(const char* str)                                                      \
{                                                                                       \
    auto it = std::find_if(std::begin(T##_names), std::end(T##_names),                  \
        [&](const auto& v) { return strcmp(str, v.second) == 0; });                     \
    return it == std::end(T##_names) ? static_cast<T>(0) : it->first;                   \
}                                                                                       \
                                                                                        \
template <>                                                                             \
bool Enum<T>::TryParse(const char* str, T* enum_out)                                    \
{                                                                                       \
    auto it = std::find_if(std::begin(T##_names), std::end(T##_names),                  \
        [&](const auto& v) { return strcmp(str, v.second) == 0; });                     \
    if (it == std::end(T##_names)) return false;                                        \
    if (enum_out != nullptr) { *enum_out = it->first; }                                 \
    return true;                                                                        \
}

#endif // MISO_ENUM_HPP_
