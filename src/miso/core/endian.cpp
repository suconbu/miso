#include "miso/endian.h"

#include <algorithm>

namespace miso {

Endian
EndianUtils::GetNativeEndian()
{
    const std::uint16_t s(1);
    return *(reinterpret_cast<const std::uint8_t*>(std::addressof(s))) != 0 ? Endian::Little : Endian::Big;
}

template<typename T> T
EndianUtils::Flip(const T value)
{
    if (sizeof(T) == 1)
    {
        return value;
    }
    if (sizeof(T) == 2)
    {
        auto v = static_cast<uint16_t>(value);
        v = (v << 8) | ((v >> 8) & 0xFF);
        return static_cast<T>(v);
    }
    else if (sizeof(T) == 4)
    {
        auto v = static_cast<uint32_t>(value);
        v = ((v << 8) & 0xFF00FF00UL) | ((v >> 8) & 0x00FF00FFUL);
        v = (v << 16) | ((v >> 16) & 0xFFFF);
        return static_cast<T>(v);
    }
    else if (sizeof(T) == 8)
    {
        auto v = static_cast<uint64_t>(value);
        v = ((v << 8) & 0xFF00FF00FF00FF00ULL) | ((v >> 8) & 0x00FF00FF00FF00FFULL);
        v = ((v << 16) & 0xFFFF0000FFFF0000ULL) | ((v >> 16) & 0x0000FFFF0000FFFFULL);
        v = (v << 32) | ((v >> 32) & 0xFFFFFFFFULL);
        return static_cast<T>(v);
    }
    else
    {
        T t = value;
        auto p = reinterpret_cast<char*>(&t);
        for (size_t i = 0; i < (sizeof(T) / 2); i++)
        {
            std::swap(p[i], p[sizeof(T) - i - 1]);
        }
        return t;
    }
}
#define MAKE_TEMPLATE_INSTANCE_WITH(T) \
    template T EndianUtils::Flip(T);

MAKE_TEMPLATE_INSTANCE_WITH(char);
MAKE_TEMPLATE_INSTANCE_WITH(short);
MAKE_TEMPLATE_INSTANCE_WITH(int);
MAKE_TEMPLATE_INSTANCE_WITH(long);
MAKE_TEMPLATE_INSTANCE_WITH(long long);
MAKE_TEMPLATE_INSTANCE_WITH(signed char);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned char);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned short);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned int);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned long);
MAKE_TEMPLATE_INSTANCE_WITH(unsigned long long);
MAKE_TEMPLATE_INSTANCE_WITH(float);
MAKE_TEMPLATE_INSTANCE_WITH(double);
MAKE_TEMPLATE_INSTANCE_WITH(long double);
MAKE_TEMPLATE_INSTANCE_WITH(int8_t);
MAKE_TEMPLATE_INSTANCE_WITH(int16_t);
MAKE_TEMPLATE_INSTANCE_WITH(int32_t);
MAKE_TEMPLATE_INSTANCE_WITH(int64_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint8_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint16_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint32_t);
MAKE_TEMPLATE_INSTANCE_WITH(uint64_t);

}//namespace miso
