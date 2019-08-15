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
    static constexpr float kPI = 3.1415926535f;

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
    // Testing for (a == b + c) or (a == c + b) with ignore case
    // e.g.
    // a:"EaseInSine" b:"sine" c:"easein" -> true
    // a:"SineEaseIn" b:"sine" c:"easein" -> true
    // a:"EaseSineIn" b:"sine" c:"easein" -> false
    auto match = [](const char* a, const char* b, const char* c) {
        auto b_len = strlen(b);
        auto c_len = strlen(c);
        if (StringUtils::CompareN(a, b, b_len, true) == 0) {
            return StringUtils::Compare(a + b_len, c, true) == 0;
        } else if (StringUtils::CompareN(a, c, c_len, true) == 0) {
            return StringUtils::Compare(a + c_len, b, true) == 0;
        } else {
            return false;
        }
    };

    std::string name_str(name);
    auto name_str_end = std::remove_if(name_str.begin(), name_str.end(), [](char c) {
        return c == ' ' || c == '-' || c == '_';
    });
    name_str.erase(name_str_end, name_str.end());
    const char* n = name_str.c_str();

    // https://developer.mozilla.org/ja/docs/Web/CSS/timing-function
    // https://on-ze.com/archives/6697

    // CSS compatible easings
    if (StringUtils::Compare(n, "stepstart", true) == 0) function_ = StepStart;
    else if (StringUtils::Compare(n, "stepend", true) == 0) function_ = StepEnd;
    else if (StringUtils::Compare(n, "linear", true) == 0) InitializeBezier(0.0f, 0.0f, 1.0f, 1.0f);
    else if (StringUtils::Compare(n, "ease", true) == 0) InitializeBezier(0.25f, 0.1f, 0.25f, 1.0f);
    else if (StringUtils::Compare(n, "easein", true) == 0) InitializeBezier(0.42f, 0.0f, 1.0f, 1.0f);
    else if (StringUtils::Compare(n, "easeinout", true) == 0) InitializeBezier(0.42f, 0.0f, 0.58f, 1.0f);
    else if (StringUtils::Compare(n, "easeout", true) == 0) InitializeBezier(0.0f, 0.0f, 0.58f, 1.0f);
    // Sine
    else if (match(n, "sine", "easein")) InitializeBezier(0.47f, 0.0f, 0.745f, 0.715f);
    else if (match(n, "sine", "easeout")) InitializeBezier(0.39f, 0.575f, 0.565f, 1.0f);
    else if (match(n, "sine", "easeinout")) InitializeBezier(0.445f, 0.05f, 0.55f, 0.95f);
    // Quad
    else if (match(n, "quad", "easein")) InitializeBezier(0.55f, 0.085f, 0.68f, 0.53f);
    else if (match(n, "quad", "easeout")) InitializeBezier(0.25f, 0.46f, 0.45f, 0.94f);
    else if (match(n, "quad", "easeinout")) InitializeBezier(0.455f, 0.03f, 0.515f, 0.955f);
    // Cubic
    else if (match(n, "cubic", "easein")) InitializeBezier(0.55f, 0.055f, 0.675f, 0.19f);
    else if (match(n, "cubic", "easeout")) InitializeBezier(0.215f, 0.61f, 0.355f, 1.0f);
    else if (match(n, "cubic", "easeinout")) InitializeBezier(0.645f, 0.045f, 0.355f, 1.0f);
    // Quartic
    else if (match(n, "quart", "easein")) InitializeBezier(0.895f, 0.03f, 0.685f, 0.22f);
    else if (match(n, "quart", "easeout")) InitializeBezier(0.165f, 0.84f, 0.44f, 1.0f);
    else if (match(n, "quart", "easeinout")) InitializeBezier(0.77f, 0.0f, 0.175f, 1.0f);
    // Quintic
    else if (match(n, "quint", "easein")) InitializeBezier(0.755f, 0.05f, 0.855f, 0.06f);
    else if (match(n, "quint", "easeout")) InitializeBezier(0.23f, 1.0f, 0.32f, 1.0f);
    else if (match(n, "quint", "easeinout")) InitializeBezier(0.86f, 0.0f, 0.07f, 1.0f);
    // Exponential
    else if (match(n, "expo", "easein")) InitializeBezier(0.95f, 0.05f, 0.795f, 0.035f);
    else if (match(n, "expo", "easeout")) InitializeBezier(0.19f, 1.0f, 0.22f, 1.0f);
    else if (match(n, "expo", "easeinout")) InitializeBezier(1.0f, 0.0f, 0.0f, 1.0f);
    // Circular
    else if (match(n, "circ", "easein")) InitializeBezier(0.6f, 0.04f, 0.98f, 0.335f);
    else if (match(n, "circ", "easeout")) InitializeBezier(0.075f, 0.82f, 0.165f, 1.0f);
    else if (match(n, "circ", "easeinout")) InitializeBezier(0.785f, 0.135f, 0.15f, 0.86f);
    // Elastic
    else if (match(n, "elastic", "easein")) function_ = EaseInElastic;
    else if (match(n, "elastic", "easeout")) function_ = EaseOutElastic;
    else if (match(n, "elastic", "easeinout")) function_ = EaseInOutElastic;
    // Bounce
    else if (match(n, "bounce", "easein")) function_ = EaseInBounce;
    else if (match(n, "bounce", "easeout")) function_ = EaseOutBounce;
    else if (match(n, "bounce", "easeinout")) function_ = EaseInOutBounce;
    else function_ = None;
}

inline float
Interpolator::EaseInElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    float n1 = 0.3f;
    float n2 = n1 / 4.0f;
    return -(d * std::powf(2.0f, 10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPI * 2.0f / n1)) + s;
}
inline float
Interpolator::EaseOutElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    float n1 = 0.3f;
    float n2 = n1 / 4.0f;
    return d * std::powf(2.0f, -10.0f * t) * std::sinf((t - n2) * kPI * 2.0f / n1) + d + s;
}

inline float
Interpolator::EaseInOutElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    t /= 0.5f;
    float n1 = 0.45f;
    float n2 = n1 / 4.0f;
    return (t < 1.0f) ?
        -0.5f * (d * std::powf(2.0f, 10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPI * 2.0f / n1)) + s :
        d * std::powf(2.0f, -10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPI * 2.0f / n1) * 0.5f + d + s;
}

inline float
Interpolator::EaseInBounce(const Interpolator& self, float t, float s, float d)
{
    return d - EaseOutBounce(self, 1.0f - t, 0.0f, d) + s;
}

inline float
Interpolator::EaseOutBounce(const Interpolator& self, float t, float s, float d)
{
    if (t < 0.3636363636f) {
        return d * (7.5625f * t * t) + s;
    }
    if (t < 0.7272727272f) {
        t -= 0.5454545455f;
        return d * (7.5625f * t * t + 0.75f) + s;
    }
    if (t < 0.9090909091f) {
        t -= 0.8181818182f;
        return d * (7.5625f * t * t + 0.9375f) + s;
    }
    t -= 0.9545454545f;
    return d * (7.5625f * t * t + 0.984375f) + s;
}

inline float
Interpolator::EaseInOutBounce(const Interpolator& self, float t, float s, float d)
{
    return (t < 0.5f) ?
        EaseInBounce(self, t * 2.0f, 0.0f, d / 2.0f) * 0.5f + s :
        EaseOutBounce(self, t * 2.0f - 1.0f, 0.0f, d) * 0.5f + d * 0.5f + s;
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
        samples_[i] = CalculateBezier(static_cast<float>(i) / (kSampleCount - 1), x1, x2);
    }
}

inline float
Interpolator::GetT(const Interpolator& self, float x)
{
    int current_sample = 1;
    int last_sample = kSampleCount - 1;
    auto& samples = self.samples_;
    auto x1 = self.x1_;
    auto x2 = self.x2_;

    while (current_sample != last_sample && samples[current_sample] <= x) {
        ++current_sample;
    }
    --current_sample;

    auto interval_start = static_cast<float>(current_sample) / (kSampleCount - 1);

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
        if (slope == 0.0f) break;
        auto cx = CalculateBezier(guess_t, x1, x2) - x;
        if (cx == 0.0f) break;
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
