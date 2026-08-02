#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <bit>
#include <limits>

namespace Math {

// return -1 for negative numbers, 0 for 0 and +1 for positive numbers
template<typename T> requires (std::is_arithmetic_v<T>)
constexpr T sign(T x) {
    return (x > 0) - (x < 0);
}

constexpr float lerp(float a, float b, float alpha) {
    return a + (b - a) * alpha;
}

inline float exp2f(float exp) {
    float sign = 0.f;
    if ( exp < 0.f )
        sign = 1.f;
    const float clamped_exp = std::fmaxf(-126.f, exp);
    return std::bit_cast<float>((int)(float)((float)((float)((float)(27.728024f
                                                       / (float)(4.8425255f - (float)((float)(clamped_exp - std::round(clamped_exp)) + sign)))
                                               + (float)(clamped_exp + 121.27406f))
                                       - (float)((float)((float)(clamped_exp - std::round(clamped_exp)) + sign) * 1.4901291f))
                               * 8388608.0f));
}

inline float log2f(float v) {
    return (float)((float)((float)((float)((float)std::bit_cast<int32_t>(v)
                                                       * 0.00000011920929f)
                                               - 124.22552f)
                                       - (float)(std::bit_cast<float>((std::bit_cast<uint32_t>(v) & 0x7FFFFF) | 0x3F000000)
                                               * 1.4980303f))
                               - (float)(1.72588
                                       / (float)(std::bit_cast<float>((std::bit_cast<uint32_t>(v) & 0x7FFFFF) | 0x3F000000)
                                               + 0.35208872f)));
}

inline float cbrtf(float v) {
    return exp2f(log2f(v) * 0.33333334f);
}

inline float powf(float b, float e) {
    if (b == 0.f) {
        if (e == 0.f) return 1.f;
        if (e < 0.f) return INFINITY;
        return 0.f;
    }
    
    if (e == std::floor(e)) {
        int32_t ue = (int32_t)e;
        float result = 1.f;

        if (ue < 0) {
            ue = -ue;
            while (ue > 0) {
                if (ue & 1) result *= b;
                b *= b;
                ue >>= 1;
            }
            return 1.f / result;
        } else {
            while (ue > 0) {
                if (ue & 1) result *= b;
                b *= b;
                ue >>= 1;
            }
            return result;
        }
    }

    return exp2f(log2f(b) * e);
}

// from https://code.google.com/archive/p/fastapprox/
inline float fastlog2 (float x) {
    union { float f; uint32_t i; } vx = { x };
    union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;

    return y - 124.22551499f
             - 1.498030302f * mx.f
             - 1.72587999f / (0.3520887068f + mx.f);
}

// from https://code.google.com/archive/p/fastapprox/
inline float fastpow2 (float p) {
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = clipp;
    float z = clipp - w + offset;
    union { uint32_t i; float f; } v = { static_cast<uint32_t> ( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };

    return v.f;
}

static float fastpow(const float x, const float p) {
    return fastpow2 (p * fastlog2 (x));
}

inline float sin(float x) {
    constexpr double c_2500 = std::bit_cast<double>(0x3fd0000000000000); // 0.25
    constexpr double c_inv_2pi = std::bit_cast<double>(0x3fc45f306dc9c883); // 1/2pi
    constexpr double c76_56 = std::bit_cast<double>(0x405324687a27a35e); // 76.56887678023256
    constexpr double c81_60 = std::bit_cast<double>(0x405466876b29f494); // 81.60201529595571
    constexpr double c41_34 = std::bit_cast<double>(0x4044abbc02329376); // 41.34167506665737
    constexpr double c_2pi = std::bit_cast<double>(0x401921fb51bf1614); // 6.283185269630412
    constexpr double c39_96 = std::bit_cast<double>(0x4043d4243780214b); // 39.65735524898863

    double d = (double)x;
    double scaled_d = d * c_inv_2pi + -c_2500;
    scaled_d = c_2500 - std::abs(scaled_d - std::round(scaled_d));
    double scaled_d2 = scaled_d * scaled_d;
    double scaled_d4 = scaled_d2 * scaled_d2;
    return (float)((scaled_d4 * scaled_d4 * c39_96 +
            (scaled_d2 * -c76_56 + c81_60) * scaled_d4 + scaled_d2 * -c41_34 + c_2pi) * scaled_d);
}

inline float cos(float x) {
    constexpr double c_2500 = std::bit_cast<double>(0x3fd0000000000000); // 0.25
    constexpr double c_inv_2pi = std::bit_cast<double>(0x3fc45f306dc9c883); // 1/2pi
    constexpr double c76_56 = std::bit_cast<double>(0x405324687a27a35e); // 76.56887678023256
    constexpr double c81_60 = std::bit_cast<double>(0x405466876b29f494); // 81.60201529595571
    constexpr double c41_34 = std::bit_cast<double>(0x4044abbc02329376); // 41.34167506665737
    constexpr double c_2pi = std::bit_cast<double>(0x401921fb51bf1614); // 6.283185269630412
    constexpr double c39_96 = std::bit_cast<double>(0x4043d4243780214b); // 39.65735524898863

    double d = (double)x;
    double scaled_d = d * c_inv_2pi;
    scaled_d = c_2500 - std::abs(scaled_d - std::round(scaled_d));
    double scaled_d2 = scaled_d * scaled_d;
    double scaled_d4 = scaled_d2 * scaled_d2;
    return (float)((scaled_d4 * scaled_d4 * c39_96 +
            (scaled_d2 * -c76_56 + c81_60) * scaled_d4 + scaled_d2 * -c41_34 + c_2pi) * scaled_d);
}

constexpr float const_sqrt_implement(float x, float curr, float prev) {
    return curr == prev
        ? curr
        : const_sqrt_implement(x, 0.5f * (curr + x / curr), curr);
}

constexpr float const_sqrt(float x) {
    return x >= 0.f && x < std::numeric_limits<float>::infinity()
        ? const_sqrt_implement(x, x, 0.f)
        : std::numeric_limits<float>::quiet_NaN();
}

}
