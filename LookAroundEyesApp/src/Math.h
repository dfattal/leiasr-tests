/*!
 * LookAround Eyes Comparison App
 * Math utilities for 3D transformations
 */

#pragma once

// Include C math header first to ensure C functions are declared in global namespace
// This works around conflicts with the Leia SR SDK headers
#include <math.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// The Leia SR SDK pollutes both global namespace and std namespace,
// breaking math functions. Define safe wrapper functions here that use
// the global C functions directly before any SDK headers can break them.
namespace SafeMath {
    inline float Sqrt(float x) { return ::sqrtf(x); }
    inline float Cos(float x) { return ::cosf(x); }
    inline float Sin(float x) { return ::sinf(x); }
    inline float Atan2(float y, float x) { return ::atan2f(y, x); }
}

#pragma pack(push, 1)

// 3D Vector
struct vec3f
{
    union
    {
        struct { float x, y, z; };
        struct { float e[3]; };
    };

    // Constructors
    vec3f() = default;
    explicit vec3f(float xyz) : x(xyz), y(xyz), z(xyz) {}
    explicit vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // Binary operators
    friend vec3f operator+(const vec3f& lhs, float rhs) { return vec3f(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs); }
    friend vec3f operator-(const vec3f& lhs, float rhs) { return vec3f(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs); }
    friend vec3f operator*(const vec3f& lhs, float rhs) { return vec3f(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs); }
    friend vec3f operator/(const vec3f& lhs, float rhs) { const float inv = 1.0f / rhs; return vec3f(lhs.x * inv, lhs.y * inv, lhs.z * inv); }
    friend vec3f operator+(float lhs, const vec3f& rhs) { return vec3f(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
    friend vec3f operator-(float lhs, const vec3f& rhs) { return vec3f(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z); }
    friend vec3f operator*(float lhs, const vec3f& rhs) { return vec3f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
    friend vec3f operator+(const vec3f& lhs, const vec3f& rhs) { return vec3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
    friend vec3f operator-(const vec3f& lhs, const vec3f& rhs) { return vec3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }
    friend vec3f operator*(const vec3f& lhs, const vec3f& rhs) { return vec3f(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z); }
    friend vec3f operator/(const vec3f& lhs, const vec3f& rhs) { return vec3f(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z); }

    // Unary operators
    vec3f operator-() const { return vec3f(-x, -y, -z); }

    // Compound assignment
    vec3f& operator+=(const vec3f& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    vec3f& operator-=(const vec3f& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    vec3f& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }

    // Static methods
    static vec3f cross(const vec3f& lhs, const vec3f& rhs) {
        return vec3f(lhs.y * rhs.z - lhs.z * rhs.y,
                     lhs.z * rhs.x - lhs.x * rhs.z,
                     lhs.x * rhs.y - lhs.y * rhs.x);
    }
    static float dot(const vec3f& lhs, const vec3f& rhs) {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }
    static float length(const vec3f& v) {
        return SafeMath::Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    static vec3f normalize(const vec3f& v) {
        float len = length(v);
        return (len > 0.0f) ? v / len : vec3f(0.0f, 0.0f, 0.0f);
    }
};

// 4D Vector
struct vec4f
{
    union
    {
        struct { float x, y, z, w; };
        struct { float e[4]; };
    };

    // Constructors
    vec4f() = default;
    explicit vec4f(float xyzw) : x(xyzw), y(xyzw), z(xyzw), w(xyzw) {}
    explicit vec4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    explicit vec4f(const vec3f& xyz, float _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {}
};

// 4x4 Matrix (row-major for DirectX)
struct mat4f
{
    float m[4][4];

    // Constructors
    mat4f() = default;

    // Identity matrix
    static mat4f identity() {
        mat4f result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        return result;
    }

    // Translation matrix
    static mat4f translation(const vec3f& t) {
        mat4f result = identity();
        result.m[3][0] = t.x;
        result.m[3][1] = t.y;
        result.m[3][2] = t.z;
        return result;
    }

    static mat4f translation(float x, float y, float z) {
        return translation(vec3f(x, y, z));
    }

    // Scaling matrix
    static mat4f scaling(float sx, float sy, float sz) {
        mat4f result = identity();
        result.m[0][0] = sx;
        result.m[1][1] = sy;
        result.m[2][2] = sz;
        return result;
    }

    static mat4f scaling(float s) {
        return scaling(s, s, s);
    }

    // Rotation around Y axis
    static mat4f rotationY(float angle) {
        mat4f result = identity();
        float c = SafeMath::Cos(angle);
        float s = SafeMath::Sin(angle);
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    // Rotation around X axis
    static mat4f rotationX(float angle) {
        mat4f result = identity();
        float c = SafeMath::Cos(angle);
        float s = SafeMath::Sin(angle);
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    // Rotation around Z axis
    static mat4f rotationZ(float angle) {
        mat4f result = identity();
        float c = SafeMath::Cos(angle);
        float s = SafeMath::Sin(angle);
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    // Asymmetric perspective frustum (for off-axis projection)
    static mat4f perspective(float l, float r, float b, float t, float n, float f) {
        mat4f result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = 0.0f;

        result.m[0][0] = 2.0f * n / (r - l);
        result.m[1][1] = 2.0f * n / (t - b);
        result.m[2][0] = (r + l) / (r - l);
        result.m[2][1] = (t + b) / (t - b);
        result.m[2][2] = -(f + n) / (f - n);
        result.m[2][3] = -1.0f;
        result.m[3][2] = -2.0f * f * n / (f - n);

        return result;
    }

    // Matrix multiplication
    friend mat4f operator*(const mat4f& lhs, const mat4f& rhs) {
        mat4f result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += lhs.m[i][k] * rhs.m[k][j];
                }
            }
        }
        return result;
    }

    // Transform vec4
    friend vec4f operator*(const mat4f& m, const vec4f& v) {
        return vec4f(
            m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
            m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
            m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
            m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w
        );
    }
};

#pragma pack(pop)

// Generalized perspective projection (Kooima 2009) with virtual screen positioning
inline mat4f CalculateViewProjectionMatrix(const vec3f& eyePosition,
                                            float screenWidthMM,
                                            float screenHeightMM,
                                            float virtualScreenDepthMM)
{
    const float znear = 0.1f;
    const float zfar = 10000.0f;

    // CRITICAL: Virtual screen is positioned at virtualScreenDepthMM (typically 400mm)
    // This places the screen plane in the middle of our scene
    vec3f pa(-screenWidthMM / 2.0f, screenHeightMM / 2.0f, virtualScreenDepthMM);  // Top-left
    vec3f pb(screenWidthMM / 2.0f, screenHeightMM / 2.0f, virtualScreenDepthMM);   // Top-right
    vec3f pc(-screenWidthMM / 2.0f, -screenHeightMM / 2.0f, virtualScreenDepthMM); // Bottom-left

    // Screen basis vectors
    vec3f vr(1.0f, 0.0f, 0.0f);  // Right
    vec3f vu(0.0f, 1.0f, 0.0f);  // Up
    vec3f vn(0.0f, 0.0f, 1.0f);  // Normal (toward viewer)

    // Vectors from eye to screen corners
    vec3f va = pa - eyePosition;
    vec3f vb = pb - eyePosition;
    vec3f vc = pc - eyePosition;

    // Distance from eye to screen plane
    float distance = -vec3f::dot(va, vn);

    // Frustum extents at near plane
    float l = vec3f::dot(vr, va) * znear / distance;
    float r = vec3f::dot(vr, vb) * znear / distance;
    float b = vec3f::dot(vu, vc) * znear / distance;
    float t = vec3f::dot(vu, va) * znear / distance;

    // Asymmetric frustum matrix
    mat4f frustum = mat4f::perspective(l, r, b, t, znear, zfar);

    // Translation to move eye to origin
    mat4f translate = mat4f::translation(-eyePosition);

    return frustum * translate;
}
