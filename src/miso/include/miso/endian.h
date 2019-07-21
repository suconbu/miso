#ifndef MISO_CORE_ENDIAN_H_
#define MISO_CORE_ENDIAN_H_

namespace miso {

enum class Endian { Native, Little, Big };

class EndianUtils
{
public:
    static Endian GetNativeEndian();
    template<typename T> static T Flip(const T value);

    EndianUtils() = delete;
};

} // namespace miso

#endif // MISO_CORE_ENDIAN_H_
