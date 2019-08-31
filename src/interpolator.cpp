#include "miso/interpolator.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <map>

#include "miso/string_utils.hpp"

namespace miso {

MISO_INLINE const Interpolator&
Interpolator::GetInterpolator(const char* name)
{
    std::string name_str(name);
    auto name_str_end = std::remove_if(name_str.begin(), name_str.end(), [](char c) {
        return c == ' ' || c == '-' || c == '_';
    });
    name_str.erase(name_str_end, name_str.end());
    name_str = StringUtils::ToLower(name_str);
    static const auto ease = { std::string("easein"), std::string("easeout"), std::string("easeinout") };
    for (auto& e : ease) {
        if (StringUtils::EndsWith(name_str, e)) {
            name_str.erase(name_str.size() - e.size(), e.size());
            name_str = e + name_str;
            break;
        }
    }
    const char* n = name_str.c_str();

    static const Interpolator invalid;
    // https://developer.mozilla.org/ja/docs/Web/CSS/timing-function
    // https://on-ze.com/archives/6697
    static const std::map<std::string, Interpolator> interpolators = {
        // CSS compatible
        { "stepstart", Interpolator(StepStart) },
        { "stepend", Interpolator(StepEnd) },
        { "linear", Interpolator(0.0f, 0.0f, 1.0f, 1.0f) },
        { "ease", Interpolator(0.25f, 0.1f, 0.25f, 1.0f) },
        { "easein", Interpolator(0.42f, 0.0f, 1.0f, 1.0f) },
        { "easeinout", Interpolator(0.42f, 0.0f, 0.58f, 1.0f) },
        { "easeout", Interpolator(0.0f, 0.0f, 0.58f, 1.0f) },
        // Sine
        { "easeinsine", Interpolator(0.47f, 0.0f, 0.745f, 0.715f) },
        { "easeoutsine", Interpolator(0.39f, 0.575f, 0.565f, 1.0f) },
        { "easeinoutsine", Interpolator(0.445f, 0.05f, 0.55f, 0.95f) },
        // Quad
        { "quadeasein", Interpolator(0.55f, 0.085f, 0.68f, 0.53f) },
        { "quadeaseout", Interpolator(0.25f, 0.46f, 0.45f, 0.94f) },
        { "quadeaseinout", Interpolator(0.455f, 0.03f, 0.515f, 0.955f) },
        // Cubic
        { "easeincubic", Interpolator(0.55f, 0.055f, 0.675f, 0.19f) },
        { "easeoutcubic", Interpolator(0.215f, 0.61f, 0.355f, 1.0f) },
        { "easeinoutcubic", Interpolator(0.645f, 0.045f, 0.355f, 1.0f) },
        // Quartic
        { "easeinquart", Interpolator(0.895f, 0.03f, 0.685f, 0.22f) },
        { "easeoutquart", Interpolator(0.165f, 0.84f, 0.44f, 1.0f) },
        { "easeinoutquart", Interpolator(0.77f, 0.0f, 0.175f, 1.0f) },
        // Quintic
        { "easeinquint", Interpolator(0.755f, 0.05f, 0.855f, 0.06f) },
        { "easeoutquint", Interpolator(0.23f, 1.0f, 0.32f, 1.0f) },
        { "easeinoutquint", Interpolator(0.86f, 0.0f, 0.07f, 1.0f) },
        // Exponential
        { "easeinexpo", Interpolator(0.95f, 0.05f, 0.795f, 0.035f) },
        { "easeoutexpo", Interpolator(0.19f, 1.0f, 0.22f, 1.0f) },
        { "easeinoutexpo", Interpolator(1.0f, 0.0f, 0.0f, 1.0f) },
        // Circular
        { "aseincirce", Interpolator(0.6f, 0.04f, 0.98f, 0.335f) },
        { "aseoutcirce", Interpolator(0.075f, 0.82f, 0.165f, 1.0f) },
        { "aseinoutcirce", Interpolator(0.785f, 0.135f, 0.15f, 0.86f) },
        // Elastic
        { "easeinelastic", Interpolator(EaseInElastic) },
        { "easeoutelastic", Interpolator(EaseOutElastic) },
        { "easeinoutelastic", Interpolator(EaseInOutElastic) },
        // Bounce
        { "easeinbounce", Interpolator(EaseInBounce) },
        { "easeoutbounce", Interpolator(EaseOutBounce) },
        { "easeinoutbounce", Interpolator(EaseInOutBounce) },
    };
    auto found = interpolators.find(n);
    return (found != interpolators.end()) ? found->second : invalid;
}

MISO_INLINE
Interpolator::Interpolator(float x1, float y1, float x2, float y2)
{
    if (x1 < 0.0f || 1.0f < x2) return;

    function_ = Bezier;
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;
    for (int i = 0; i < kSampleCount; ++i) {
        samples_[i] = CalculateBezier(static_cast<float>(i) / (kSampleCount - 1), x1, x2);
    }
}

MISO_INLINE float
Interpolator::EaseInElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    float n1 = 0.3f;
    float n2 = n1 / 4.0f;
    return -(d * std::powf(2.0f, 10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPi * 2.0f / n1)) + s;
}
MISO_INLINE float
Interpolator::EaseOutElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    float n1 = 0.3f;
    float n2 = n1 / 4.0f;
    return d * std::powf(2.0f, -10.0f * t) * std::sinf((t - n2) * kPi * 2.0f / n1) + d + s;
}

MISO_INLINE float
Interpolator::EaseInOutElastic(const Interpolator& self, float t, float s, float d)
{
    if (1.0f <= t) return s + d;
    t /= 0.5f;
    float n1 = 0.45f;
    float n2 = n1 / 4.0f;
    return (t < 1.0f) ?
        -0.5f * (d * std::powf(2.0f, 10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPi * 2.0f / n1)) + s :
        d * std::powf(2.0f, -10.0f * (t - 1.0f)) * std::sinf((t - 1.0f - n2) * kPi * 2.0f / n1) * 0.5f + d + s;
}

MISO_INLINE float
Interpolator::EaseInBounce(const Interpolator& self, float t, float s, float d)
{
    return d - EaseOutBounce(self, 1.0f - t, 0.0f, d) + s;
}

MISO_INLINE float
Interpolator::EaseOutBounce(const Interpolator& self, float t, float s, float d)
{
    if (t < 0.364f) {
        return d * (7.5625f * t * t) + s;
    }
    if (t < 0.727f) {
        t -= 0.545f;
        return d * (7.563f * t * t + 0.75f) + s;
    }
    if (t < 0.909f) {
        t -= 0.818f;
        return d * (7.563f * t * t + 0.938f) + s;
    }
    t -= 0.955f;
    return d * (7.563f * t * t + 0.984f) + s;
}

MISO_INLINE float
Interpolator::EaseInOutBounce(const Interpolator& self, float t, float s, float d)
{
    return (t < 0.5f) ?
        EaseInBounce(self, t * 2.0f, 0.0f, d / 2.0f) * 0.5f + s :
        EaseOutBounce(self, t * 2.0f - 1.0f, 0.0f, d) * 0.5f + d * 0.5f + s;
}

MISO_INLINE float
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

    auto denom = samples[current_sample + 1] - samples[current_sample];
    denom = (denom == 0.0f) ? 0.0000001f : denom;
    auto dist = (x - samples[current_sample]) / denom;
    auto guess_t = interval_start + dist * kSampleStep;

    auto slope = CalculateSlope(guess_t, x1, x2);
    return
        (slope >= kNewtonMinSlope) ? NewtonRaphsonIterate(x, guess_t, x1, x2) :
        (slope == 0.0f) ? guess_t :
        BinarySubdivide(x, interval_start, interval_start + kSampleStep, x1, x2);
}

MISO_INLINE float
Interpolator::NewtonRaphsonIterate(float x, float t, float x1, float x2)
{
    for (int i = 0; i < kNewtonRaphsonMaxIterations; ++i) {
        auto slope = CalculateSlope(t, x1, x2);
        if (slope == 0.0f) break;
        auto cx = CalculateBezier(t, x1, x2) - x;
        if (cx == 0.0f) break;
        t -= cx / slope;
    }
    return t;
}

MISO_INLINE float
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

MISO_INLINE float
Interpolator::Bezier(const Interpolator& self, float t, float s, float d)
{
    return
        (t <= 0.0f) ? (s) :
        (1.0f <= t) ? (s + d) :
        (s + d * CalculateBezier(self.GetT(self, t), self.y1_, self.y2_));
}

} // namespace miso
