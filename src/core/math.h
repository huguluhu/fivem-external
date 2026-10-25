#pragma once
#include <cmath>
#include <algorithm>

struct Vec2 {
    float x = 0.f, y = 0.f;
    Vec2() {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
    Vec2 operator+(const Vec2& o) const { return { x + o.x, y + o.y }; }
    Vec2 operator-(const Vec2& o) const { return { x - o.x, y - o.y }; }
};

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
    Vec3() {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s) const { return { x * s, y * s, z * s }; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
};

inline float Dist(const Vec3& a, const Vec3& b) { return (a - b).Length(); }

struct Matrix4 { float m[16] = {}; };

struct RGBA {
    float r = 1.f, g = 1.f, b = 1.f, a = 1.f;
    constexpr RGBA() {}
    constexpr RGBA(float _r, float _g, float _b, float _a = 1.f) : r(_r), g(_g), b(_b), a(_a) {}
};

inline float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float Lerpf(float a, float b, float t) { return a + (b - a) * t; }

namespace palette {
    constexpr RGBA Accent      { 1.000f, 0.243f, 0.588f, 1.00f };
    constexpr RGBA AccentBright{ 1.000f, 0.478f, 0.722f, 1.00f };
    constexpr RGBA AccentDim   { 0.702f, 0.165f, 0.412f, 1.00f };
    constexpr RGBA Bg          { 0.039f, 0.039f, 0.051f, 0.96f };
    constexpr RGBA Panel       { 0.071f, 0.071f, 0.094f, 1.00f };
    constexpr RGBA Widget      { 0.102f, 0.102f, 0.133f, 1.00f };
    constexpr RGBA Border      { 0.149f, 0.149f, 0.196f, 1.00f };
    constexpr RGBA Text        { 0.929f, 0.929f, 0.949f, 1.00f };
    constexpr RGBA TextDim     { 0.545f, 0.545f, 0.600f, 1.00f };
    constexpr RGBA Good        { 0.290f, 0.870f, 0.502f, 1.00f };
    constexpr RGBA Bad         { 1.000f, 0.290f, 0.352f, 1.00f };
}