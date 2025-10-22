/*!
 * Leia Splat Viewer
 * Orbit camera system for interactive viewing
 */

#pragma once

#include "leia_math.h"

class OrbitCamera
{
public:
    OrbitCamera();
    ~OrbitCamera();

    // Initialize camera with default parameters
    void initialize(float distance = 1000.0f, float azimuth = 0.0f, float elevation = 0.0f);

    // Update camera (call each frame)
    void update();

    // Mouse input handling
    void onMouseDrag(int deltaX, int deltaY);
    void onMouseWheel(int delta);

    // Keyboard input
    void reset();

    // Get camera matrices
    mat4f getViewMatrix() const;
    mat4f getProjectionMatrix(float fovY, float aspect, float znear, float zfar) const;
    mat4f getViewProjectionMatrix(float fovY, float aspect, float znear, float zfar) const;

    // Get camera position and target
    vec3f getPosition() const { return m_position; }
    vec3f getTarget() const { return m_target; }

    // Set target point (default is origin)
    void setTarget(const vec3f& target) { m_target = target; }

    // Get spherical coordinates
    float getAzimuth() const { return m_azimuth; }
    float getElevation() const { return m_elevation; }
    float getDistance() const { return m_distance; }

    // Set spherical coordinates
    void setAzimuth(float azimuth) { m_azimuth = azimuth; m_dirty = true; }
    void setElevation(float elevation) { m_elevation = Clamp(elevation, -89.0f * (float)M_PI / 180.0f, 89.0f * (float)M_PI / 180.0f); m_dirty = true; }
    void setDistance(float distance) { m_distance = Clamp(distance, m_minDistance, m_maxDistance); m_dirty = true; }

    // Set distance constraints
    void setDistanceConstraints(float minDist, float maxDist) { m_minDistance = minDist; m_maxDistance = maxDist; }

    // Set sensitivity
    void setRotationSensitivity(float sensitivity) { m_rotationSensitivity = sensitivity; }
    void setZoomSensitivity(float sensitivity) { m_zoomSensitivity = sensitivity; }

private:
    // Update camera position from spherical coordinates
    void updatePosition();

    // Spherical coordinates
    float m_azimuth;     // Horizontal rotation (radians)
    float m_elevation;   // Vertical rotation (radians)
    float m_distance;    // Distance from target

    // Target point (orbit center)
    vec3f m_target;

    // Computed camera position
    vec3f m_position;

    // Constraints
    float m_minDistance;
    float m_maxDistance;

    // Input sensitivity
    float m_rotationSensitivity;
    float m_zoomSensitivity;

    // Default values (for reset)
    float m_defaultAzimuth;
    float m_defaultElevation;
    float m_defaultDistance;
    vec3f m_defaultTarget;

    // Dirty flag
    bool m_dirty;
};
