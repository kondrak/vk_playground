#ifndef UBO_INCLUDED
#define UBO_INCLUDED

#include "Math.hpp"
#include <stdint.h>

// UBO used by the main shader
struct UniformBufferObject
{
    Math::Matrix4f ModelViewProjectionMatrix;
};

// GLSL attribute IDs for both the main and font shaders
enum Attributes : uint32_t
{
    inVertex = 0,
    inTexCoord = 1,
    inColor = 2,
};

#endif