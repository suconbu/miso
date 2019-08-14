#ifndef MISO_INTERPOLATOR_H_
#define MISO_INTERPOLATOR_H_

#include <array>

#include "miso/string_utils.h"

namespace miso {

// https://github.com/thomasuster/cubic-bezier/blob/master/src/com/thomasuster/CubicBezier.hx

class Interpolator {
public:
    Interpolator() = delete;
    explicit Interpolator(const char* name);
    //explicit Interpolator(float x1, float y1, float x2, float y2);

    float Interpolate(float start, float end, float progress) const { return function_(progress, start, end - start); }

private:
    //using Function = float(*)(float t, double b, double c);

    static constexpr size_t kSampleCount = 11;

    static float None(float t, float s, float d) { return StepStart(t, s, d); }
    static float StepStart(float t, float s, float d) { return s + d; }
    static float StepEnd(float t, float s, float d) { return (t < 1.0f) ? s : (s + d); }
    static float Linear(float t, float s, float d) { return s + d * t; }
    //static float Bezier(float t, float b, float c);

    float(*function_)(float, float, float);
    //float x1_ = 0.0f;
    //float y1_ = 0.0f;
    //float x2_ = 0.0f;
    //float y2_ = 0.0f;
    //std::array<float, kSampleCount> samples_;
};

inline
Interpolator::Interpolator(const char* name)
{
    function_ =
        StringUtils::CompareIgnoreCase(name, "step-start") == 0 ? StepStart :
        StringUtils::CompareIgnoreCase(name, "step-end") == 0 ? StepEnd :
        StringUtils::CompareIgnoreCase(name, "linear") == 0 ? Linear :
        None;
}

//inline
//Interpolator::Interpolator(float x1, float y1, float x2, float y2) :
//    function_(Bezier)
//{
//    ;
//}

//inline float
//Interpolator::Bezier(const Interpolator& self, float t, float b, float c)
//{
//    ;
//}

} // namespace miso

#endif // MISO_INTERPOLATOR_H_
