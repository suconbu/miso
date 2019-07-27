#ifndef MISO_CORE_ENDIAN_H_
#define MISO_CORE_ENDIAN_H_

#include <memory>

namespace miso {

enum class Endian { Native, Little, Big };

class EndianUtils
{
public:
    static Endian GetNativeEndian();
    template<typename T> static T Flip(const T value);

    EndianUtils() = delete;
};

inline Endian
EndianUtils::GetNativeEndian()
{
    const std::uint16_t s(1);
    return *(reinterpret_cast<const std::uint8_t*>(std::addressof(s))) != 0 ? Endian::Little : Endian::Big;
}

template<typename T> inline T
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

} // namespace miso

#endif // MISO_CORE_ENDIAN_H_
