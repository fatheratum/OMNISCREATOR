// ============================================================================
// omnibrain_sphere.vert
// OMNIVALKIN — Vulkan GLSL Vertex Shader
// Central Holographic OMNIBRAIN Sphere — Core + Shell passes
// OMNIBRAIN INFINITY Universe | Naru Atum Protocol
// GLSL 4.60 / Vulkan 1.2
// ============================================================================
#version 460
#extension GL_ARB_separate_shader_objects : enable

// ============================================================================
// LAYOUTS
// ============================================================================

layout(location = 0) in vec3  inPos;
layout(location = 1) in vec3  inNormal;
layout(location = 2) in vec2  inUV;
layout(location = 3) in vec4  inColor;
layout(location = 4) in float inIntensity;
layout(location = 5) in float inLayer;

layout(location = 0) out vec3  outWorldPos;
layout(location = 1) out vec3  outNormal;
layout(location = 2) out vec2  outUV;
layout(location = 3) out vec4  outColor;
layout(location = 4) out float outIntensity;
layout(location = 5) out float outLayer;

// ============================================================================
// PUSH CONSTANTS / UBO
// ============================================================================

layout(set = 0, binding = 0) uniform FrameUBO {
    mat4  view;
    mat4  proj;
    mat4  viewProj;
    vec3  eyePos;
    float simTime;
    float coreGlow;
    float coreScale;
    float solvencyIndex;
    float yieldRate;
    float lightCodeIntensity;
    float simWallOpacity;
    vec2  pad;
} frame;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4  world;
    vec4  color;
    float layer;
    float opacity;
    float intensity;
    float pad2;
} obj;

// ============================================================================
// MAIN
// ============================================================================
void main() {
    // Core scale from solvency/liquid assets
    float scale     = (inLayer < 0.5) ? frame.coreScale : 1.0;
    vec3  scaledPos = inPos * scale;

    vec4  worldPos  = obj.world * vec4(scaledPos, 1.0);
    outWorldPos     = worldPos.xyz;
    outNormal       = mat3(obj.world) * inNormal;
    outUV           = inUV;
    outColor        = inColor * obj.color;
    outIntensity    = inIntensity * frame.coreGlow * frame.lightCodeIntensity;
    outLayer        = inLayer;

    gl_Position     = frame.viewProj * worldPos;
}
