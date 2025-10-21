/*!
 * Leia Splat Viewer
 * PLY file loader for Gaussian Splat scenes
 */

#pragma once

#include "Math.h"
#include <vector>
#include <string>
#include <map>

// Splat vertex data structure (matches GPU layout)
struct SplatVertex
{
    vec3f position;       // x, y, z
    vec3f scale;          // scale_0, scale_1, scale_2 (log space)
    vec4f rotation;       // rot_0, rot_1, rot_2, rot_3 (quaternion, normalized)
    float opacity;        // opacity (logit space)
    vec3f sh_dc;          // f_dc_0, f_dc_1, f_dc_2 (DC component of SH)
    float sh_rest[45];    // f_rest_0..44 (remaining SH coefficients, 3 bands)
    vec3f color;          // Fallback color if no SH (r, g, b)

    SplatVertex()
        : position(0,0,0)
        , scale(0,0,0)
        , rotation(0,0,0,1)
        , opacity(0)
        , sh_dc(0,0,0)
        , color(1,1,1)
    {
        for (int i = 0; i < 45; i++) sh_rest[i] = 0.0f;
    }
};

// PLY property descriptor
struct PLYProperty
{
    enum Type {
        FLOAT,
        DOUBLE,
        UCHAR,
        INT,
        UINT
    };

    std::string name;
    Type type;
    size_t offset; // Offset in binary data
    size_t size;   // Size in bytes
};

// PLY loader class
class SplatLoader
{
public:
    SplatLoader();
    ~SplatLoader();

    // Load PLY file
    bool load(const std::string& filePath);

    // Get loaded splats
    const std::vector<SplatVertex>& getSplats() const { return m_splats; }
    size_t getSplatCount() const { return m_splats.size(); }

    // Get load status
    bool isLoaded() const { return m_loaded; }
    const std::string& getErrorMessage() const { return m_errorMessage; }

    // Get bounding box
    void getBounds(vec3f& minBounds, vec3f& maxBounds) const;

private:
    // Parse PLY header
    bool parseHeader(FILE* file);

    // Parse binary data
    bool parseBinaryData(FILE* file);

    // Helper: Read property value from binary stream
    bool readProperty(FILE* file, const PLYProperty& prop, float& outValue);

    // Helper: Find property by name
    const PLYProperty* findProperty(const std::string& name) const;

    // Member variables
    std::vector<SplatVertex> m_splats;
    std::vector<PLYProperty> m_properties;
    size_t m_vertexCount;
    size_t m_vertexSize; // Total size of one vertex in binary data
    bool m_loaded;
    std::string m_errorMessage;
};
