/*!
 * Leia Splat Viewer
 * PLY file loader implementation
 */

#include "SplatLoader.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

SplatLoader::SplatLoader()
    : m_vertexCount(0)
    , m_vertexSize(0)
    , m_loaded(false)
{
}

SplatLoader::~SplatLoader()
{
}

bool SplatLoader::load(const std::string& filePath)
{
    m_loaded = false;
    m_splats.clear();
    m_properties.clear();
    m_errorMessage.clear();

    // Open file
    FILE* file = nullptr;
    fopen_s(&file, filePath.c_str(), "rb");
    if (!file)
    {
        m_errorMessage = "Failed to open file: " + filePath;
        return false;
    }

    // Parse header
    if (!parseHeader(file))
    {
        fclose(file);
        return false;
    }

    // Parse binary data
    if (!parseBinaryData(file))
    {
        fclose(file);
        return false;
    }

    fclose(file);

    m_loaded = true;
    return true;
}

bool SplatLoader::parseHeader(FILE* file)
{
    char line[256];

    // Read "ply"
    if (!fgets(line, sizeof(line), file) || strncmp(line, "ply", 3) != 0)
    {
        m_errorMessage = "Invalid PLY file: missing 'ply' magic";
        return false;
    }

    // Read format
    if (!fgets(line, sizeof(line), file))
    {
        m_errorMessage = "Invalid PLY file: missing format line";
        return false;
    }
    if (strstr(line, "binary_little_endian") == nullptr)
    {
        m_errorMessage = "Only binary_little_endian PLY format is supported";
        return false;
    }

    // Parse properties until "end_header"
    bool inVertexElement = false;
    m_vertexSize = 0;

    while (fgets(line, sizeof(line), file))
    {
        // Remove newline
        line[strcspn(line, "\r\n")] = 0;

        if (strncmp(line, "end_header", 10) == 0)
        {
            break;
        }
        else if (strncmp(line, "element vertex", 14) == 0)
        {
            // Parse vertex count
            sscanf_s(line, "element vertex %zu", &m_vertexCount);
            inVertexElement = true;
        }
        else if (strncmp(line, "element", 7) == 0)
        {
            // Another element type (not vertex)
            inVertexElement = false;
        }
        else if (inVertexElement && strncmp(line, "property", 8) == 0)
        {
            // Parse property
            char typeStr[32];
            char nameStr[64];
            if (sscanf_s(line, "property %s %s", typeStr, (unsigned)sizeof(typeStr), nameStr, (unsigned)sizeof(nameStr)) == 2)
            {
                PLYProperty prop;
                prop.name = nameStr;
                prop.offset = m_vertexSize;

                // Determine type and size
                if (strcmp(typeStr, "float") == 0)
                {
                    prop.type = PLYProperty::FLOAT;
                    prop.size = sizeof(float);
                }
                else if (strcmp(typeStr, "double") == 0)
                {
                    prop.type = PLYProperty::DOUBLE;
                    prop.size = sizeof(double);
                }
                else if (strcmp(typeStr, "uchar") == 0)
                {
                    prop.type = PLYProperty::UCHAR;
                    prop.size = sizeof(unsigned char);
                }
                else if (strcmp(typeStr, "int") == 0)
                {
                    prop.type = PLYProperty::INT;
                    prop.size = sizeof(int);
                }
                else if (strcmp(typeStr, "uint") == 0)
                {
                    prop.type = PLYProperty::UINT;
                    prop.size = sizeof(unsigned int);
                }
                else
                {
                    m_errorMessage = "Unknown property type: " + std::string(typeStr);
                    return false;
                }

                m_vertexSize += prop.size;
                m_properties.push_back(prop);
            }
        }
    }

    if (m_vertexCount == 0)
    {
        m_errorMessage = "No vertices found in PLY file";
        return false;
    }

    return true;
}

bool SplatLoader::parseBinaryData(FILE* file)
{
    m_splats.reserve(m_vertexCount);

    // Temporary buffer for one vertex
    std::vector<unsigned char> vertexBuffer(m_vertexSize);

    for (size_t i = 0; i < m_vertexCount; i++)
    {
        // Read one vertex
        if (fread(vertexBuffer.data(), 1, m_vertexSize, file) != m_vertexSize)
        {
            m_errorMessage = "Failed to read vertex data";
            return false;
        }

        // Parse into SplatVertex
        SplatVertex splat;

        // Helper lambda to read property from buffer
        auto readProp = [&](const std::string& name, float& outValue) -> bool {
            const PLYProperty* prop = findProperty(name);
            if (!prop) return false;

            const unsigned char* dataPtr = vertexBuffer.data() + prop->offset;

            switch (prop->type)
            {
            case PLYProperty::FLOAT:
                outValue = *reinterpret_cast<const float*>(dataPtr);
                break;
            case PLYProperty::DOUBLE:
                outValue = static_cast<float>(*reinterpret_cast<const double*>(dataPtr));
                break;
            case PLYProperty::UCHAR:
                outValue = static_cast<float>(*dataPtr);
                break;
            case PLYProperty::INT:
                outValue = static_cast<float>(*reinterpret_cast<const int*>(dataPtr));
                break;
            case PLYProperty::UINT:
                outValue = static_cast<float>(*reinterpret_cast<const unsigned int*>(dataPtr));
                break;
            }

            return true;
        };

        // Position
        readProp("x", splat.position.x);
        readProp("y", splat.position.y);
        readProp("z", splat.position.z);

        // Scale (log space)
        readProp("scale_0", splat.scale.x);
        readProp("scale_1", splat.scale.y);
        readProp("scale_2", splat.scale.z);

        // Rotation (quaternion)
        readProp("rot_0", splat.rotation.x);
        readProp("rot_1", splat.rotation.y);
        readProp("rot_2", splat.rotation.z);
        readProp("rot_3", splat.rotation.w);

        // Normalize quaternion
        splat.rotation = vec4f::normalize(splat.rotation);

        // Opacity (logit space)
        readProp("opacity", splat.opacity);

        // Spherical harmonics DC component
        readProp("f_dc_0", splat.sh_dc.x);
        readProp("f_dc_1", splat.sh_dc.y);
        readProp("f_dc_2", splat.sh_dc.z);

        // Spherical harmonics rest (45 coefficients for 3 bands)
        for (int j = 0; j < 45; j++)
        {
            char propName[32];
            sprintf_s(propName, "f_rest_%d", j);
            readProp(propName, splat.sh_rest[j]);
        }

        // Compute color from spherical harmonics DC component
        // SH_C0 = sqrt(1/(4*pi)) = 0.28209479177387814
        const float SH_C0 = 0.28209479177387814f;

        // Check if we have SH DC properties
        bool hasSH = (findProperty("f_dc_0") != nullptr);

        float r, g, b;
        if (hasSH)
        {
            // Convert from SH to RGB (Gaussian splatting standard)
            r = 0.5f + SH_C0 * splat.sh_dc.x;
            g = 0.5f + SH_C0 * splat.sh_dc.y;
            b = 0.5f + SH_C0 * splat.sh_dc.z;
        }
        else
        {
            // Fallback: try to read RGB directly
            r = 0.5f; g = 0.5f; b = 0.5f;
            readProp("red", r);
            readProp("green", g);
            readProp("blue", b);

            // Convert color from [0, 255] to [0, 1] if stored as uchar
            const PLYProperty* redProp = findProperty("red");
            if (redProp && redProp->type == PLYProperty::UCHAR)
            {
                r /= 255.0f;
                g /= 255.0f;
                b /= 255.0f;
            }
        }

        // Clamp to valid range
        r = std::max(0.0f, std::min(1.0f, r));
        g = std::max(0.0f, std::min(1.0f, g));
        b = std::max(0.0f, std::min(1.0f, b));

        splat.color = vec3f(r, g, b);

        m_splats.push_back(splat);
    }

    return true;
}

const PLYProperty* SplatLoader::findProperty(const std::string& name) const
{
    for (const auto& prop : m_properties)
    {
        if (prop.name == name)
            return &prop;
    }
    return nullptr;
}

void SplatLoader::getBounds(vec3f& minBounds, vec3f& maxBounds) const
{
    if (m_splats.empty())
    {
        minBounds = vec3f(0, 0, 0);
        maxBounds = vec3f(0, 0, 0);
        return;
    }

    minBounds = m_splats[0].position;
    maxBounds = m_splats[0].position;

    for (const auto& splat : m_splats)
    {
        minBounds.x = std::min(minBounds.x, splat.position.x);
        minBounds.y = std::min(minBounds.y, splat.position.y);
        minBounds.z = std::min(minBounds.z, splat.position.z);

        maxBounds.x = std::max(maxBounds.x, splat.position.x);
        maxBounds.y = std::max(maxBounds.y, splat.position.y);
        maxBounds.z = std::max(maxBounds.z, splat.position.z);
    }
}
