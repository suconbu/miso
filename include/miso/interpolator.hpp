#ifndef MISO_INTERPOLATOR_HPP_
#define MISO_INTERPOLATOR_HPP_

#include "miso/common.h"

#include <array>
#include <string>

namespace miso {

class Interpolator {
public:
    static const Interpolator& GetInterpolator(const char* name);
    static const Interpolator& GetInterpolator(const std::string& name) { return GetInterpolator(name.c_str()); }

    Interpolator() = default;
    explicit Interpolator(float x1, float y1, float x2, float y2);

    bool IsValid() const { return function_ != None; }
    float Interpolate(float start, float end, float progress) const { return function_(*this, progress, start, end - start); }

private:
    using Function = float(*)(const Interpolator&, float, float, float);

    static constexpr int kSampleCount = 11;
    static constexpr float kSampleStep = 1.0f / (kSampleCount - 1);
    static constexpr int kNewtonRaphsonMaxIterations = 4;
    static constexpr float kNewtonMinSlope = 0.001f;
    static constexpr float kSubdivisionPrecision = 0.0000001f;
    static constexpr int kSubdivisionMaxIterations = 10;
    static constexpr float kPi = 3.1415926535f;

    explicit Interpolator(Function function) : function_(function) {}

    static float None(const Interpolator& self, float t, float s, float d) { return StepStart(self, t, s, d); }
    static float Bezier(const Interpolator& self, float t, float s, float d);
    static float StepStart(const Interpolator& self, float t, float s, float d) { (void)self;  return (t <= 0.0f) ? s : (s + d); }
    static float StepEnd(const Interpolator& self, float t, float s, float d) { (void)self; return (t < 1.0f) ? s : (s + d); }
    static float EaseInElastic(const Interpolator& self, float t, float s, float d);
    static float EaseOutElastic(const Interpolator& self, float t, float s, float d);
    static float EaseInOutElastic(const Interpolator& self, float t, float s, float d);
    static float EaseInBounce(const Interpolator& self, float t, float s, float d);
    static float EaseOutBounce(const Interpolator& self, float t, float s, float d);
    static float EaseInOutBounce(const Interpolator& self, float t, float s, float d);

    static float CalculateBezier(float t, float a1, float a2) { return ((GetA(a1, a2) * t + GetB(a1, a2)) * t + GetC(a1)) * t; }
    static float CalculateSlope(float t, float a1, float a2) { return (3.0f * GetA(a1, a2) * t + 2.0f * GetB(a1, a2)) * t + GetC(a1); }
    static float GetA(float a1, float a2) { return 1.0f - 3.0f * a2 + 3.0f * a1; }
    static float GetB(float a1, float a2) { return 3.0f * a2 - 6.0f * a1; }
    static float GetC(float a1) { return 3.0f * a1; }
    static float GetT(const Interpolator& self, float x);
    static float NewtonRaphsonIterate(float x, float guess_t, float x1, float x2);
    static float BinarySubdivide(float x, float a, float b, float x1, float x2);

    Function function_ = None;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
    float x2_ = 0.0f;
    float y2_ = 0.0f;
    std::array<float, kSampleCount> samples_ = {};
};

} // namespace miso

#ifdef MISO_HEADER_ONLY
#include "interpolator.cpp"
#endif // MISO_HEADER_ONLY

#endif // MISO_INTERPOLATOR_HPP_
