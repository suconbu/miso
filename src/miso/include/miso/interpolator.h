#ifndef MISO_INTERPOLATOR_H_
#define MISO_INTERPOLATOR_H_

#include "miso/string.h"

namespace miso {

class Interpolator {
public:
    using Function = float(*)(float, float, float);

    static Function GetFunction(const char* name);

    static float Linear(float s, float c, float t) { return s + c * t; }
};

inline Interpolator::Function
Interpolator::GetFunction(const char* name)
{
    return
        StringUtils::CompareIgnoreCase(name, "linear") == 0 ? Linear :
        Linear;
}

} // namespace miso

#endif // MISO_INTERPOLATOR_H_
