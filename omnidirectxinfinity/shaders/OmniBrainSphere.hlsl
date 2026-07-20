// ============================================================================
// OmniBrainSphere.hlsl
// OMNIDIRECTXINFINITY — Vertex + Pixel Shaders
// Central Holographic OMNIBRAIN Sphere rendering
//   • Core glow pass
//   • Holographic layer shells (additive blend)
//   • Sacred geometry rings (line list)
//   • Light-code particle sprites (point → quad expansion in GS)
// OMNIBRAIN INFINITY Universe | Naru Atum Protocol
// ============================================================================

// ============================================================================
// CONSTANT BUFFERS
// ============================================================================

cbuffer FrameConstants : register(b0) {
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    float3   gEyePos;
    float    gSimTime;
    float    gCoreGlow;
    float    gCoreScale;
    float    gSolvencyIndex;
    float    gYieldRate;
    float    gLightCodeIntensity;
    float    gSimWallOpacity;
    float2   gPad;
};

cbuffer ObjectConstants : register(b1) {
    float4x4 gWorld;
    float4   gColor;
    float    gLayer;        // Which holographic layer index
    float    gOpacity;
    float    gIntensity;
    float    gPad2;
};

// ============================================================================
// CORE SPHERE — VS + PS
// ============================================================================

struct CoreVSIn {
    float3 pos       : POSITION;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

struct CoreVSOut {
    float4 clipPos   : SV_POSITION;
    float3 worldPos  : WORLDPOS;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

CoreVSOut VS_Core(CoreVSIn input) {
    CoreVSOut output;

    // Apply core scale from solvency / liquid assets
    float3 scaledPos = input.pos * gCoreScale;

    float4 worldPos  = mul(gWorld, float4(scaledPos, 1.0f));
    output.worldPos  = worldPos.xyz;
    output.clipPos   = mul(gViewProj, worldPos);
    output.normal    = mul((float3x3)gWorld, input.normal);
    output.uv        = input.uv;
    output.color     = input.color;
    output.intensity = input.intensity * gCoreGlow * gLightCodeIntensity;
    return output;
}

float4 PS_Core(CoreVSOut input) : SV_TARGET {
    float3 N     = normalize(input.normal);
    float3 V     = normalize(gEyePos - input.worldPos);
    float  NdotV = saturate(dot(N, V));

    // Fresnel rim — golden glow brightens at edges
    float  rim   = pow(1.0f - NdotV, 3.5f);
    float3 gold  = float3(1.0f, 0.78f, 0.10f);

    // Inner core pulse
    float pulse  = 0.5f + 0.5f * sin(gSimTime * 1.2f);
    float glow   = input.intensity * (0.6f + 0.4f * pulse);

    float3 baseColor = input.color.rgb * glow;
    float3 rimColor  = gold * rim * gCoreGlow * 2.5f;
    float3 finalColor = baseColor + rimColor;

    // Emission from core center (distance-based)
    float  distFromCenter = length(input.worldPos);
    float  coreEmit  = exp(-distFromCenter * 1.8f) * gCoreGlow * 3.0f;
    finalColor += gold * coreEmit;

    return float4(finalColor * gLightCodeIntensity, input.color.a);
}

// ============================================================================
// HOLOGRAPHIC SHELL — additive blended transparent layers
// ============================================================================

struct ShellVSIn {
    float3 pos       : POSITION;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
    float  layer     : LAYER;
};

struct ShellVSOut {
    float4 clipPos   : SV_POSITION;
    float3 worldPos  : WORLDPOS;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD;
    float4 color     : COLOR;
    float  layer     : LAYER;
};

ShellVSOut VS_Shell(ShellVSIn input) {
    ShellVSOut output;
    float4 worldPos = mul(gWorld, float4(input.pos, 1.0f));
    output.worldPos = worldPos.xyz;
    output.clipPos  = mul(gViewProj, worldPos);
    output.normal   = mul((float3x3)gWorld, input.normal);
    output.uv       = input.uv;
    output.color    = input.color;
    output.layer    = input.layer;
    return output;
}

float4 PS_Shell(ShellVSOut input) : SV_TARGET {
    float3 N     = normalize(input.normal);
    float3 V     = normalize(gEyePos - input.worldPos);
    float  NdotV = saturate(dot(N, V));

    // Edge glow — holographic shell brightest at silhouette
    float  edge  = pow(1.0f - NdotV, 2.0f);
    float3 gold  = float3(1.0f, 0.78f + input.layer * 0.02f, 0.1f);

    // Animated light-code scan lines
    float  scanV = frac(input.uv.y * 20.0f - gSimTime * 0.5f
                        - input.layer * 0.3f);
    float  scan  = smoothstep(0.45f, 0.5f, scanV)
                 * smoothstep(0.55f, 0.5f, scanV);

    // Animated light-code columns
    float  scanH = frac(input.uv.x * 12.0f + gSimTime * 0.3f
                        + input.layer * 0.2f);
    float  col   = smoothstep(0.4f, 0.5f, scanH)
                 * smoothstep(0.6f, 0.5f, scanH);

    float  lightCode = (scan + col) * 0.5f;
    float3 color     = gold * (edge * 0.8f + lightCode * 0.6f)
                     * gLightCodeIntensity;

    float  alpha     = gOpacity * (edge * 0.7f + lightCode * 0.4f + 0.05f);
    alpha           *= gSolvencyIndex;

    return float4(color, saturate(alpha));
}

// ============================================================================
// SACRED GEOMETRY RINGS — line list, bright gold
// ============================================================================

struct RingVSIn {
    float3 pos       : POSITION;
    float3 normal    : NORMAL;
    float2 uv        : TEXCOORD;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

struct RingVSOut {
    float4 clipPos   : SV_POSITION;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

RingVSOut VS_Ring(RingVSIn input) {
    RingVSOut output;
    float4 worldPos = mul(gWorld, float4(input.pos, 1.0f));
    output.clipPos  = mul(gViewProj, worldPos);
    output.color    = input.color;
    output.intensity= input.intensity;
    return output;
}

float4 PS_Ring(RingVSOut input) : SV_TARGET {
    // Pulse glow on rings — bright sacred gold
    float  pulse = 0.6f + 0.4f * sin(gSimTime * 1.5f + input.uv.x * 6.28f);
    float3 gold  = float3(1.0f, 0.92f, 0.3f);
    float3 color = gold * input.intensity * pulse * gLightCodeIntensity;
    float  alpha = input.color.a * input.intensity * (0.5f + 0.5f * pulse);
    return float4(color, saturate(alpha));
}

// ============================================================================
// LIGHT-CODE PARTICLE SPRITES
// Geometry shader expands each point to a camera-facing quad
// ============================================================================

struct ParticleVSIn {
    float3 pos   : POSITION;
    float4 color : COLOR;
    float  size  : SIZE;
    uint   type  : TYPE;
};

struct ParticleGSIn {
    float4 worldPos : WORLDPOS;
    float4 color    : COLOR;
    float  size     : SIZE;
    uint   type     : TYPE;
};

struct ParticlePSIn {
    float4 clipPos  : SV_POSITION;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD;
    uint   type     : TYPE;
};

ParticleGSIn VS_Particle(ParticleVSIn input) {
    ParticleGSIn output;
    output.worldPos = mul(gWorld, float4(input.pos, 1.0f));
    output.color    = input.color;
    output.size     = input.size;
    output.type     = input.type;
    return output;
}

[maxvertexcount(4)]
void GS_Particle(point ParticleGSIn input[1],
                 inout TriangleStream<ParticlePSIn> outStream)
{
    float3 center = input[0].worldPos.xyz;
    float  s      = input[0].size;

    // Camera-facing basis
    float3 toEye  = normalize(gEyePos - center);
    float3 up     = float3(0, 1, 0);
    float3 right  = normalize(cross(up, toEye));
    up            = cross(toEye, right);

    float2 uvs[4] = {
        float2(0,1), float2(1,1), float2(0,0), float2(1,0)
    };
    float3 offsets[4] = {
        -right*s - up*s,
         right*s - up*s,
        -right*s + up*s,
         right*s + up*s
    };

    [unroll]
    for (int i = 0; i < 4; ++i) {
        ParticlePSIn v;
        v.clipPos = mul(gViewProj, float4(center + offsets[i], 1.0f));
        v.color   = input[0].color;
        v.uv      = uvs[i];
        v.type    = input[0].type;
        outStream.Append(v);
    }
}

float4 PS_Particle(ParticlePSIn input) : SV_TARGET {
    // Soft circular sprite
    float2 uvc   = input.uv * 2.0f - 1.0f;
    float  dist  = length(uvc);
    if (dist > 1.0f) discard;

    float  soft  = 1.0f - smoothstep(0.5f, 1.0f, dist);
    float3 color = input.color.rgb;

    // Type-specific glow
    if (input.type == 1u) {
        // Burst — brighter, wider core
        soft = pow(soft, 0.5f);
        color *= 1.8f;
    } else if (input.type == 2u) {
        // Sacred — star-shaped cross flare
        float star = max(abs(uvc.x), abs(uvc.y));
        soft = (1.0f - smoothstep(0.3f, 1.0f, dist))
             + (1.0f - smoothstep(0.0f, 0.15f, star)) * 0.5f;
        color = float3(1.0f, 0.95f, 0.5f);
    }

    float alpha = soft * input.color.a * gLightCodeIntensity;
    return float4(color * soft, saturate(alpha));
}

// ============================================================================
// FLOAT-GEN MATRIX — line and node shaders
// ============================================================================

struct MatrixLineVSIn {
    float3 pos       : POSITION;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

struct MatrixLineVSOut {
    float4 clipPos   : SV_POSITION;
    float4 color     : COLOR;
    float  intensity : INTENSITY;
};

MatrixLineVSOut VS_MatrixLine(MatrixLineVSIn input) {
    MatrixLineVSOut output;
    float4 worldPos = mul(gWorld, float4(input.pos, 1.0f));
    output.clipPos  = mul(gViewProj, worldPos);
    output.color    = input.color;
    output.intensity= input.intensity;
    return output;
}

float4 PS_MatrixLine(MatrixLineVSOut input) : SV_TARGET {
    // Golden exchange path — pulse with sim time
    float pulse = 0.7f + 0.3f * sin(gSimTime * 2.0f);
    return float4(
        input.color.rgb * input.intensity * pulse * gLightCodeIntensity,
        input.color.a
    );
}

// ============================================================================
// NODE SPHERE — instanced small spheres at each grid node
// ============================================================================

struct NodeVSIn {
    float3   pos      : POSITION;
    float3   normal   : NORMAL;
    // Per-instance
    float4x4 instWorld: INSTWORLD;
    float4   instColor: INSTCOLOR;
    float    instGlow : INSTGLOW;
};

struct NodeVSOut {
    float4 clipPos  : SV_POSITION;
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float  glow     : GLOW;
};

NodeVSOut VS_Node(NodeVSIn input) {
    NodeVSOut output;
    float4 worldPos = mul(input.instWorld, float4(input.pos, 1.0f));
    output.clipPos  = mul(gViewProj, worldPos);
    output.normal   = mul((float3x3)input.instWorld, input.normal);
    output.color    = input.instColor;
    output.glow     = input.instGlow;
    return output;
}

float4 PS_Node(NodeVSOut input) : SV_TARGET {
    float3 N     = normalize(input.normal);
    float3 V     = normalize(gEyePos - float3(0,0,0));
    float  NdotV = saturate(dot(N, V));
    float  rim   = pow(1.0f - NdotV, 2.5f);

    float  pulse = 0.5f + 0.5f * sin(gSimTime * 3.0f + input.glow * 10.0f);
    float3 gold  = float3(1.0f, 0.82f, 0.15f);
    float3 color = input.color.rgb * (0.4f + 0.6f * pulse)
                 + gold * rim * input.glow * gLightCodeIntensity;

    return float4(color, input.color.a);
}
