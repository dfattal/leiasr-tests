/*!
 * Leia Splat Viewer
 * Math utilities for 3D transformations, quaternions, and covariance matrices
 */

#pragma once

// Include Windows math header FIRST before any other headers
// The Leia SR SDK completely breaks all math functions in all namespaces
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declare C math functions with extern "C" linkage to bypass namespace pollution
extern "C" {
    float sqrtf(float);
    float cosf(float);
    float sinf(float);
    float atan2f(float, float);
}

// Safe wrapper namespace that directly calls C functions
namespace SafeMath {
    inline float Sqrt(float x) { return sqrtf(x); }
    inline float Cos(float x) { return cosf(x); }
    inline float Sin(float x) { return sinf(x); }
    inline float Atan2(float y, float x) { return atan2f(y, x); }
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

    // Quaternion operations
    static vec4f normalize(const vec4f& q) {
        float len = SafeMath::Sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        return (len > 0.0f) ? vec4f(q.x / len, q.y / len, q.z / len, q.w / len) : vec4f(0, 0, 0, 1);
    }
};

// 3x3 Matrix (for covariance matrices and rotations)
struct mat3f
{
    float m[3][3];

    // Constructors
    mat3f() = default;

    // Identity matrix
    static mat3f identity() {
        mat3f result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        return result;
    }

    // Construct from quaternion (rotation matrix)
    static mat3f fromQuaternion(const vec4f& q) {
        // Quaternion is (x, y, z, w) where w is the real part
        float x = q.x, y = q.y, z = q.z, w = q.w;

        mat3f result;
        result.m[0][0] = 1.0f - 2.0f * (y*y + z*z);
        result.m[0][1] = 2.0f * (x*y - w*z);
        result.m[0][2] = 2.0f * (x*z + w*y);

        result.m[1][0] = 2.0f * (x*y + w*z);
        result.m[1][1] = 1.0f - 2.0f * (x*x + z*z);
        result.m[1][2] = 2.0f * (y*z - w*x);

        result.m[2][0] = 2.0f * (x*z - w*y);
        result.m[2][1] = 2.0f * (y*z + w*x);
        result.m[2][2] = 1.0f - 2.0f * (x*x + y*y);

        return result;
    }

    // Construct diagonal matrix from scale vector
    static mat3f fromScale(const vec3f& scale) {
        mat3f result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i][j] = 0.0f;
            }
        }
        result.m[0][0] = scale.x;
        result.m[1][1] = scale.y;
        result.m[2][2] = scale.z;
        return result;
    }

    // Matrix multiplication
    friend mat3f operator*(const mat3f& lhs, const mat3f& rhs) {
        mat3f result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 3; k++) {
                    result.m[i][j] += lhs.m[i][k] * rhs.m[k][j];
                }
            }
        }
        return result;
    }

    // Transform vec3
    friend vec3f operator*(const mat3f& m, const vec3f& v) {
        return vec3f(
            m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z,
            m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z,
            m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z
        );
    }

    // Transpose
    mat3f transpose() const {
        mat3f result;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result.m[i][j] = m[j][i];
            }
        }
        return result;
    }

    // Compute 3D covariance matrix from scale and rotation
    // Σ = R * S * S^T * R^T
    static mat3f computeCovariance3D(const vec3f& scale, const vec4f& rotation) {
        mat3f R = fromQuaternion(rotation);
        mat3f S = fromScale(scale);

        // Σ = R * S * S^T * R^T
        mat3f SS = S * S.transpose();
        mat3f RSST = R * SS;
        mat3f covariance = RSST * R.transpose();

        return covariance;
    }
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

    // Look-at view matrix
    static mat4f lookAt(const vec3f& eye, const vec3f& target, const vec3f& up) {
        vec3f zaxis = vec3f::normalize(eye - target); // Forward
        vec3f xaxis = vec3f::normalize(vec3f::cross(up, zaxis)); // Right
        vec3f yaxis = vec3f::cross(zaxis, xaxis); // Up

        mat4f result = identity();
        result.m[0][0] = xaxis.x;
        result.m[1][0] = xaxis.y;
        result.m[2][0] = xaxis.z;
        result.m[3][0] = -vec3f::dot(xaxis, eye);

        result.m[0][1] = yaxis.x;
        result.m[1][1] = yaxis.y;
        result.m[2][1] = yaxis.z;
        result.m[3][1] = -vec3f::dot(yaxis, eye);

        result.m[0][2] = zaxis.x;
        result.m[1][2] = zaxis.y;
        result.m[2][2] = zaxis.z;
        result.m[3][2] = -vec3f::dot(zaxis, eye);

        return result;
    }

    // Symmetric perspective projection
    static mat4f perspectiveFOV(float fovY, float aspect, float znear, float zfar) {
        float tanHalfFovY = tanf(fovY / 2.0f);

        mat4f result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = 0.0f;

        result.m[0][0] = 1.0f / (aspect * tanHalfFovY);
        result.m[1][1] = 1.0f / tanHalfFovY;
        result.m[2][2] = -(zfar + znear) / (zfar - znear);
        result.m[2][3] = -1.0f;
        result.m[3][2] = -(2.0f * zfar * znear) / (zfar - znear);

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

// Helper: Convert spherical coordinates to Cartesian
inline vec3f SphericalToCartesian(float azimuth, float elevation, float distance) {
    float x = distance * SafeMath::Cos(elevation) * SafeMath::Sin(azimuth);
    float y = distance * SafeMath::Sin(elevation);
    float z = distance * SafeMath::Cos(elevation) * SafeMath::Cos(azimuth);
    return vec3f(x, y, z);
}

// Helper: Clamp value to range
inline float Clamp(float value, float minVal, float maxVal) {
    return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
}

// Helper: Linear interpolation
inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Helper: Sigmoid function (for opacity conversion from logit space)
inline float Sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

// Generalized perspective projection (Kooima 2009) with virtual screen positioning
inline mat4f CalculateViewProjectionMatrix(const vec3f& eyePosition,
                                            float screenWidthMM,
                                            float screenHeightMM,
                                            float virtualScreenDepthMM)
{
    const float znear = 0.1f;
    const float zfar = 10000.0f;

    // Virtual screen positioned at virtualScreenDepthMM
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
