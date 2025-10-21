/*!
 * Leia Splat Viewer
 * Orbit camera implementation
 */

#include "OrbitCamera.h"

OrbitCamera::OrbitCamera()
    : m_azimuth(0.0f)
    , m_elevation(0.0f)
    , m_distance(1000.0f)
    , m_target(0, 0, 0)
    , m_position(0, 0, 1000.0f)
    , m_minDistance(100.0f)
    , m_maxDistance(10000.0f)
    , m_rotationSensitivity(0.005f)
    , m_zoomSensitivity(0.1f)
    , m_defaultAzimuth(0.0f)
    , m_defaultElevation(0.0f)
    , m_defaultDistance(1000.0f)
    , m_defaultTarget(0, 0, 0)
    , m_dirty(true)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::initialize(float distance, float azimuth, float elevation)
{
    m_azimuth = azimuth;
    m_elevation = Clamp(elevation, -89.0f * (float)M_PI / 180.0f, 89.0f * (float)M_PI / 180.0f);
    m_distance = Clamp(distance, m_minDistance, m_maxDistance);
    m_target = vec3f(0, 0, 0);

    // Save defaults
    m_defaultAzimuth = m_azimuth;
    m_defaultElevation = m_elevation;
    m_defaultDistance = m_distance;
    m_defaultTarget = m_target;

    m_dirty = true;
    updatePosition();
}

void OrbitCamera::update()
{
    if (m_dirty)
    {
        updatePosition();
        m_dirty = false;
    }
}

void OrbitCamera::updatePosition()
{
    // Convert spherical coordinates to Cartesian
    vec3f offset = SphericalToCartesian(m_azimuth, m_elevation, m_distance);
    m_position = m_target + offset;
}

void OrbitCamera::onMouseDrag(int deltaX, int deltaY)
{
    // Update azimuth (horizontal rotation)
    m_azimuth += deltaX * m_rotationSensitivity;

    // Wrap azimuth to [0, 2π]
    while (m_azimuth < 0.0f) m_azimuth += 2.0f * (float)M_PI;
    while (m_azimuth > 2.0f * (float)M_PI) m_azimuth -= 2.0f * (float)M_PI;

    // Update elevation (vertical rotation)
    m_elevation += deltaY * m_rotationSensitivity;

    // Clamp elevation to avoid gimbal lock
    m_elevation = Clamp(m_elevation, -89.0f * (float)M_PI / 180.0f, 89.0f * (float)M_PI / 180.0f);

    m_dirty = true;
}

void OrbitCamera::onMouseWheel(int delta)
{
    // Zoom in/out
    float zoomFactor = 1.0f + (delta > 0 ? -m_zoomSensitivity : m_zoomSensitivity);
    m_distance *= zoomFactor;

    // Clamp distance
    m_distance = Clamp(m_distance, m_minDistance, m_maxDistance);

    m_dirty = true;
}

void OrbitCamera::reset()
{
    m_azimuth = m_defaultAzimuth;
    m_elevation = m_defaultElevation;
    m_distance = m_defaultDistance;
    m_target = m_defaultTarget;

    m_dirty = true;
}

mat4f OrbitCamera::getViewMatrix() const
{
    vec3f up(0, 1, 0);
    return mat4f::lookAt(m_position, m_target, up);
}

mat4f OrbitCamera::getProjectionMatrix(float fovY, float aspect, float znear, float zfar) const
{
    return mat4f::perspectiveFOV(fovY, aspect, znear, zfar);
}

mat4f OrbitCamera::getViewProjectionMatrix(float fovY, float aspect, float znear, float zfar) const
{
    mat4f projection = getProjectionMatrix(fovY, aspect, znear, zfar);
    mat4f view = getViewMatrix();
    return projection * view;
}
