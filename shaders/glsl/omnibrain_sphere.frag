// ============================================================================
// omnibrain_sphere.frag
// OMNIVALKIN — Vulkan GLSL Fragment Shader
// Core glow | Holographic shells | Sacred ring emission
// GLSL 4.60 / Vulkan 1.2
// ============================================================================
#version 460
#extension GL_ARB_separate_shader_objects : enable

// ============================================================================
// INPUTS / OUTPUTS
// ============================================================================
layout(location = 0) in  vec3  inWorldPos;
layout(location = 1) in  vec3  inNormal;
layout(location = 2) in  vec2  inUV;
layout(location = 3) in  vec4  inColor;
layout(location = 4) in  float inIntensity;
layout(location = 5) in  float inLayer;

layout(location = 0) out vec4 outColor;

// ============================================================================
// UBO
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
// CONSTANTS
// ============================================================================
const float PI  = 3.14159265358979;
const float TAU = 6.28318530717959;
const float PHI = 1.61803398874989;

vec3 golden() { return vec3(1.0, 0.78, 0.10); }

// ============================================================================
// CORE SPHERE PASS  (layer == 0)
// ============================================================================
vec4 shade_core() {
    vec3  N     = normalize(inNormal);
    vec3  V     = normalize(frame.eyePos - inWorldPos);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    // Fresnel rim
    float rim   = pow(1.0 - NdotV, 3.5);
    vec3  gold  = golden();

    // Inner pulse
    float pulse = 0.5 + 0.5 * sin(frame.simTime * 1.2);
    float glow  = inIntensity * (0.6 + 0.4 * pulse);

    vec3  base  = inColor.rgb * glow;
    vec3  rimC  = gold * rim * frame.coreGlow * 2.5;

    // Core center emission
    float dist  = length(inWorldPos);
    float emit  = exp(-dist * 1.8) * frame.coreGlow * 3.0;

    vec3  finalC = (base + rimC + gold * emit) * frame.lightCodeIntensity;
    return vec4(finalC, inColor.a);
}

// ============================================================================
// HOLOGRAPHIC SHELL PASS  (layer > 0)
// ============================================================================
vec4 shade_shell() {
    vec3  N     = normalize(inNormal);
    vec3  V     = normalize(frame.eyePos - inWorldPos);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    float edge  = pow(1.0 - NdotV, 2.0);

    // Animated scan lines
    float scanV = fract(inUV.y * 20.0 - frame.simTime * 0.5 - inLayer * 0.3);
    float scan  = smoothstep(0.45, 0.5, scanV) * smoothstep(0.55, 0.5, scanV);

    float scanH = fract(inUV.x * 12.0 + frame.simTime * 0.3 + inLayer * 0.2);
    float col   = smoothstep(0.4, 0.5, scanH) * smoothstep(0.6, 0.5, scanH);

    float lc    = (scan + col) * 0.5;
    vec3  gold  = vec3(1.0, 0.78 + inLayer * 0.02, 0.10);
    vec3  color = gold * (edge * 0.8 + lc * 0.6) * frame.lightCodeIntensity;

    float alpha = obj.opacity * (edge * 0.7 + lc * 0.4 + 0.05)
                * frame.solvencyIndex;

    return vec4(color, clamp(alpha, 0.0, 1.0));
}

// ============================================================================
// SACRED RING PASS  (distinguished by obj.intensity > 1.5)
// ============================================================================
vec4 shade_ring() {
    float pulse = 0.6 + 0.4 * sin(frame.simTime * 1.5 + inUV.x * TAU);
    vec3  gold  = vec3(1.0, 0.92, 0.30);
    float glow  = inIntensity * pulse * frame.lightCodeIntensity;
    float alpha = inColor.a * inIntensity * (0.5 + 0.5 * pulse);
    return vec4(gold * glow, clamp(alpha, 0.0, 1.0));
}

// ============================================================================
// MAIN — dispatch by layer
// ============================================================================
void main() {
    if (obj.intensity > 1.5) {
        outColor = shade_ring();
    } else if (inLayer < 0.5) {
        outColor = shade_core();
    } else {
        outColor = shade_shell();
    }
}
