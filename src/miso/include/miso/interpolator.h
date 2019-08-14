#ifndef MISO_INTERPOLATOR_H_
#define MISO_INTERPOLATOR_H_

#include <array>
#include <cmath>

#include "miso/string_utils.h"

namespace miso {

// Bezier curve calculation was based on:
// https://github.com/thomasuster/cubic-bezier/blob/master/src/com/thomasuster/CubicBezier.hx

class Interpolator {
public:
    Interpolator() = delete;
    explicit Interpolator(const char* name);
    explicit Interpolator(const std::string& name) : Interpolator(name.c_str()) {}
    explicit Interpolator(float x1, float y1, float x2, float y2);

    bool IsValid() { return function_ != None; }
    float Interpolate(float start, float end, float progress) const { return function_(*this, progress, start, end - start); }

private:
    static constexpr int kSampleCount = 11;
    static constexpr float kSampleStep = 1.0f / (kSampleCount - 1);
    static constexpr int kNewtonIterations = 4;
    static constexpr float kNewtonMinSlope = 0.001f;
    static constexpr float kSubdivisionPrecision = 0.0000001f;
    static constexpr int kSubdivisionMaxIterations = 10;

    static float None(const Interpolator& self, float t, float s, float d) { return StepStart(self, t, s, d); }
    static float StepStart(const Interpolator& self, float t, float s, float d) { (void)self;  return (0.0f == t) ? s : (s + d); }
    static float StepEnd(const Interpolator& self, float t, float s, float d) { (void)self; return (t < 1.0f) ? s : (s + d); }
    //static float Linear(const Interpolator& self, float t, float s, float d) { (void)self; return s + d * t; }
    static float Bezier(const Interpolator& self, float t, float s, float d);

    static float CalculateBezier(float t, float a1, float a2) { return ((GetA(a1, a2) * t + GetB(a1, a2)) * t + GetC(a1)) * t; }
    static float CalculateSlope(float t, float a1, float a2) { return 3.0f * GetA(a1, a2) * t * t + 2.0f * GetB(a1, a2) * t + GetC(a1); }
    static float GetA(float a1, float a2) { return 1.0f - 3.0f * a2 + 3.0f * a1; }
    static float GetB(float a1, float a2) { return 3.0f * a2 - 6.0f * a1; }
    static float GetC(float a1) { return 3.0f * a1; }
    static float GetT(const Interpolator& self, float x);
    static float BinarySubdivide(float x, float a, float b, float x1, float x2);
    static float NewtonRaphsonIterate(float x, float guess_t, float x1, float x2);

    void InitializeBezier(float x1, float y1, float x2, float y2);

    float(*function_)(const Interpolator&, float, float, float) = None;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
    float x2_ = 0.0f;
    float y2_ = 0.0f;
    std::array<float, kSampleCount> samples_;
};

inline
Interpolator::Interpolator(const char* name)
{
    // https://developer.mozilla.org/ja/docs/Web/CSS/timing-function
    if (StringUtils::CompareIgnoreCase(name, "step-start") == 0) function_ = StepStart;
    else if (StringUtils::CompareIgnoreCase(name, "step-end") == 0) function_ = StepEnd;
    else if (StringUtils::CompareIgnoreCase(name, "linear") == 0) InitializeBezier(0.0f, 0.0f, 1.0f, 1.0f);
    else if (StringUtils::CompareIgnoreCase(name, "ease") == 0) InitializeBezier(0.25f, 0.1f, 0.25f, 1.0f);
    else if (StringUtils::CompareIgnoreCase(name, "ease-in") == 0) InitializeBezier(0.42f, 0.0f, 1.0f, 1.0f);
    else if (StringUtils::CompareIgnoreCase(name, "ease-in-out") == 0) InitializeBezier(0.42f, 0.0f, 0.58f, 1.0f);
    else if (StringUtils::CompareIgnoreCase(name, "ease-out") == 0) InitializeBezier(0.0f, 0.0f, 0.58f, 1.0f);
    else function_ = None;
}

inline
Interpolator::Interpolator(float x1, float y1, float x2, float y2)
{
    if (x1 < 0.0f || 1.0f < x2) return;
    InitializeBezier(x1, y1, x2, y2);
}

inline void
Interpolator::InitializeBezier(float x1, float y1, float x2, float y2)
{
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;
    function_ = Bezier;
    for (int i = 0; i < kSampleCount; ++i) {
        samples_[i] = CalculateBezier(i * kSampleStep, x1, x2);
    }
}

inline float
Interpolator::GetT(const Interpolator& self, float x)
{
    float interval_start = 0.0;
    int current_sample = 1;
    int last_sample = kSampleCount - 1;
    auto& samples = self.samples_;
    auto x1 = self.x1_;
    auto x2 = self.x2_;

    while (current_sample != last_sample && samples[current_sample] <= x) {
        interval_start += kSampleStep;
        ++current_sample;
    }
    --current_sample;

    // Interpolate to provide an initial guess for t
    auto dist = (x - samples[current_sample]) / (samples[current_sample + 1] - samples[current_sample]);
    auto guess_for_t = interval_start + dist * kSampleStep;

    auto initial_slope = CalculateSlope(guess_for_t, x1, x2);
    if (initial_slope >= kNewtonMinSlope) {
        return NewtonRaphsonIterate(x, guess_for_t, x1, x2);
    } else if (initial_slope == 0.0f) {
        return guess_for_t;
    } else {
        return BinarySubdivide(x, interval_start, interval_start + kSampleStep, x1, x2);
    }
}

inline float
Interpolator::BinarySubdivide(float x, float a, float b, float x1, float x2)
{
    float cx, ct;
    int i = 0;
    do {
        ct = a + (b - a) / 2.0f;
        cx = CalculateBezier(ct, x1, x2) - x;
        if (cx > 0.0f) {
            b = ct;
        } else {
            a = ct;
        }
    } while (std::abs(cx) > kSubdivisionPrecision && ++i < kSubdivisionMaxIterations);
    return ct;
}

inline float
Interpolator::NewtonRaphsonIterate(float x, float guess_t, float x1, float x2)
{
    for (int i = 0; i < kNewtonIterations; ++i) {
        auto slope = CalculateSlope(guess_t, x1, x2);
        if (slope == 0.0f) {
            return guess_t;
        }
        auto cx = CalculateBezier(guess_t, x1, x2) - x;
        guess_t -= cx / slope;
    }
    return guess_t;
}
inline float
Interpolator::Bezier(const Interpolator& self, float t, float s, float d)
{
    return
        (t <= 0.0f) ? (s) :
        (1.0f <= t) ? (s + d) :
        (s + d * CalculateBezier(self.GetT(self, t), self.y1_, self.y2_));
}

} // namespace miso

#endif // MISO_INTERPOLATOR_H_
