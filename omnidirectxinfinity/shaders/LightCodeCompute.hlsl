// ============================================================================
// LightCodeCompute.hlsl
// OMNIDIRECTXINFINITY — Compute Shader
// Light-code particle emission, update, and sphere layer rotation
// DXR / DirectX 12 Compute Pipeline
// OMNIBRAIN INFINITY Universe | Naru Atum Protocol
// ============================================================================

#define MAX_PARTICLES   8192
#define PI              3.14159265358979f
#define TAU             6.28318530717959f
#define PHI             1.61803398874989f    // Golden ratio
#define GOLDEN_R        1.00f
#define GOLDEN_G        0.78f
#define GOLDEN_B        0.10f

// ============================================================================
// STRUCTS
// ============================================================================

struct LightCodeParticle {
    float3   position;
    float3   velocity;
    float4   color;         // rgba — golden stream
    float    size;
    float    life;
    float    maxLife;
    uint     type;          // 0=stream 1=burst 2=sacred 3=void
    uint     active;        // 0 or 1
    float    pad;
};

struct SphereState {
    float    simTime;
    float    coreGlow;
    float    coreScale;
    float    yieldRate;
    float    solvencyIndex;
    float    liquidAssets;
    float    particleEmitRate;
    uint     frameIndex;
};

struct EmitCommand {
    float3   origin;
    uint     count;
    float4   color;
    uint     type;
    float3   pad;
};

// ============================================================================
// REGISTERS
// ============================================================================

RWStructuredBuffer<LightCodeParticle>  gParticles   : register(u0);
RWBuffer<uint>                         gDeadList    : register(u1);  // free slots
RWBuffer<uint>                         gDeadCount   : register(u2);

ConstantBuffer<SphereState>            gState       : register(b0);
StructuredBuffer<EmitCommand>          gEmitCmds    : register(t0);
RWBuffer<uint>                         gEmitCount   : register(u3);

// ============================================================================
// UTILITY — PCG pseudo-random (fast, GPU-friendly)
// ============================================================================

uint pcg(uint state) {
    uint s = state * 747796405u + 2891336453u;
    uint w = ((s >> ((s >> 28u) + 4u)) ^ s) * 277803737u;
    return (w >> 22u) ^ w;
}

float rand_float(inout uint seed) {
    seed = pcg(seed);
    return float(seed) / float(0xFFFFFFFFu);
}

float3 rand_dir(inout uint seed) {
    float phi_v = rand_float(seed) * TAU;
    float cost  = rand_float(seed) * 2.0f - 1.0f;
    float sint  = sqrt(max(0.0f, 1.0f - cost * cost));
    return float3(sint * cos(phi_v), cost, sint * sin(phi_v));
}

float3 golden_color(float t) {
    // t=0 → white-gold core  t=1 → deep gold edge
    return float3(
        GOLDEN_R,
        GOLDEN_G + 0.18f * (1.0f - t),
        GOLDEN_B + 0.15f * (1.0f - t) * (1.0f - t)
    );
}

// ============================================================================
// CS_EmitParticles — spawns new light-code particles onto sphere surface
// Dispatch: [ceil(emitCount/64), 1, 1]
// ============================================================================

[numthreads(64, 1, 1)]
void CS_EmitParticles(uint3 DTid : SV_DispatchThreadID) {
    uint emitCount = gEmitCount[0];
    if (DTid.x >= emitCount) return;

    // Pull an emit command (round-robin if fewer commands than emits)
    uint cmdCount = 1;  // bound by gEmitCmds size
    EmitCommand cmd = gEmitCmds[DTid.x % cmdCount];

    // Claim a dead slot
    uint slot;
    InterlockedAdd(gDeadCount[0], -1, slot);
    if (slot == 0 || slot > MAX_PARTICLES) return;
    uint idx = gDeadList[slot - 1];

    // Seed with thread + frame for variety
    uint seed = pcg(DTid.x ^ (gState.frameIndex * 1234567u));

    float3 dir   = rand_dir(seed);
    float  speed = 0.5f + rand_float(seed) * 2.5f;

    float3 origin = cmd.origin;
    float  oLen   = length(origin);
    if (oLen < 1e-6f) origin = float3(0, 1, 0);

    // Outward radiate from sphere surface
    float3 vel  = normalize(origin) * speed;
    vel += (rand_dir(seed) * 0.3f);

    float t_color = rand_float(seed);
    float3 gc     = golden_color(t_color);

    LightCodeParticle p;
    p.position = origin;
    p.velocity = vel;
    p.color    = float4(gc * cmd.color.rgb, 0.9f);
    p.size     = 0.02f + rand_float(seed) * 0.06f;
    p.maxLife  = 0.8f + rand_float(seed) * 2.2f;
    p.life     = p.maxLife;
    p.type     = cmd.type;
    p.active   = 1u;
    p.pad      = 0.0f;

    gParticles[idx] = p;
}

// ============================================================================
// CS_UpdateParticles — advances positions, fades, reclaims dead
// Dispatch: [ceil(MAX_PARTICLES/64), 1, 1]
// ============================================================================

[numthreads(64, 1, 1)]
void CS_UpdateParticles(uint3 DTid : SV_DispatchThreadID) {
    uint idx = DTid.x;
    if (idx >= MAX_PARTICLES) return;

    LightCodeParticle p = gParticles[idx];
    if (p.active == 0u) return;

    // deltaTime baked into constant buffer as 1/60 approximation for now
    float dt = 1.0f / 60.0f;

    // Advance position
    p.position += p.velocity * dt;

    // Decay life
    p.life -= dt;
    float t = saturate(p.life / max(p.maxLife, 1e-6f));

    // Fade alpha
    p.color.a = t * 0.9f;

    // Colour shift: gold → cooler white as particle ages out
    p.color.g = max(0.0f, p.color.g - dt * 0.05f);
    p.color.b = min(1.0f, p.color.b + dt * 0.10f);

    // Shrink slightly
    p.size = max(0.005f, p.size - dt * 0.01f);

    // Gentle outward drift (slight deceleration)
    p.velocity *= (1.0f - 0.03f * dt);

    // Sacred geometry attractor: subtle spiral pull on type==2
    if (p.type == 2u) {
        float angle = gState.simTime * 0.5f + float(idx) * 0.01f;
        float3 spiral = float3(cos(angle), 0, sin(angle)) * 0.05f;
        p.velocity += spiral * dt;
    }

    // Kill if expired
    if (p.life <= 0.0f || p.color.a < 0.01f) {
        p.active = 0u;
        // Return to dead list
        uint slot;
        InterlockedAdd(gDeadCount[0], 1, slot);
        gDeadList[slot] = idx;
    }

    gParticles[idx] = p;
}

// ============================================================================
// CS_RotateSacredRings — rotates ring transform matrices on GPU
// Each ring is a 4x4 matrix in a structured buffer
// Dispatch: [numRings, 1, 1]
// ============================================================================

struct RingState {
    float4x4 transform;
    float    rotationSpeed;
    float    currentAngle;
    float    glowIntensity;
    float    latitude;
    float    radius;
    float    pad[3];
};

RWStructuredBuffer<RingState> gRings : register(u4);

[numthreads(16, 1, 1)]
void CS_RotateSacredRings(uint3 DTid : SV_DispatchThreadID) {
    uint i = DTid.x;
    // Ring count passed via emitCount[1] for simplicity
    RingState rs = gRings[i];

    // Advance angle
    float dt       = 1.0f / 60.0f;
    rs.currentAngle += rs.rotationSpeed * dt
                     * (1.0f + gState.yieldRate * 2.0f);

    // Build Y-rotation matrix for this ring
    float c = cos(rs.currentAngle);
    float s = sin(rs.currentAngle);

    // Rotate around Y axis (ring lies in XZ plane)
    rs.transform[0] = float4( c, 0, s, 0);
    rs.transform[1] = float4( 0, 1, 0, 0);
    rs.transform[2] = float4(-s, 0, c, 0);
    rs.transform[3] = float4( 0, rs.latitude * rs.radius, 0, 1);

    // Pulse glow with solvency
    rs.glowIntensity = 0.4f + 0.6f
        * abs(sin(gState.simTime * 0.7f + float(i) * 0.4f))
        * saturate(gState.solvencyIndex);

    gRings[i] = rs;
}

// ============================================================================
// CS_EvolveFloatGenNodes — per-frame evolution of Float-Gen matrix nodes
// Updates value, flow, and force vectors for GPU-side animation
// Dispatch: [ceil(nodeCount/64), 1, 1]
// ============================================================================

struct FloatGenNodeGPU {
    float3   position;
    float3   forceVector;
    float    value;
    float    flow;
    uint     state;     // 0=idle 1=flowing 2=burst 3=saturated
    float    pad[3];
};

struct FloatGenPathGPU {
    uint     nodeA;
    uint     nodeB;
    float    bandwidth;
    float    currentFlow;
    uint     active;
    float    particleT;
    float    particleSpeed;
    float    pad;
};

RWStructuredBuffer<FloatGenNodeGPU> gFGNodes : register(u5);
RWStructuredBuffer<FloatGenPathGPU> gFGPaths : register(u6);

[numthreads(64, 1, 1)]
void CS_EvolveFloatGenNodes(uint3 DTid : SV_DispatchThreadID) {
    uint idx = DTid.x;
    FloatGenNodeGPU node = gFGNodes[idx];

    float dt    = 1.0f / 60.0f;
    float t     = gState.simTime;

    // Pulse value
    node.value += sin(t * 0.5f + float(idx) * 0.3f) * 10.0f;

    // Force vector oscillation (sacred geometry attractor pattern)
    node.forceVector.x += cos(t * PHI + float(idx)) * 0.01f;
    node.forceVector.y += sin(t * 0.7f + float(idx) * 0.5f) * 0.01f;
    node.forceVector.z += cos(t * 1.1f + float(idx) * 0.7f) * 0.01f;

    // Clamp force magnitude
    float fLen = length(node.forceVector);
    if (fLen > 2.0f) node.forceVector = normalize(node.forceVector) * 2.0f;

    // State decay: burst → flowing → idle
    if (node.state == 2u) node.state = 1u;  // burst decays to flowing
    else if (node.state == 1u && node.flow < 0.01f) node.state = 0u;

    // Flow decay
    node.flow *= (1.0f - 0.01f * dt * 60.0f);

    gFGNodes[idx] = node;
}

[numthreads(64, 1, 1)]
void CS_EvolveFloatGenPaths(uint3 DTid : SV_DispatchThreadID) {
    uint idx = DTid.x;
    FloatGenPathGPU path = gFGPaths[idx];

    float dt = 1.0f / 60.0f;

    if (path.active == 1u) {
        path.particleT += path.particleSpeed * dt;
        if (path.particleT >= 1.0f) {
            path.particleT    = 0.0f;
            path.currentFlow *= 0.95f;
            if (path.currentFlow < 0.001f) path.active = 0u;
        }
    }

    // Random activation (seed per path + frame)
    uint seed = pcg(idx ^ gState.frameIndex);
    float r   = float(seed) / float(0xFFFFFFFFu);
    if (r < 0.0003f) {
        path.active       = 1u;
        path.currentFlow  = 500.0f + r * 5000.0f;
        path.particleT    = 0.0f;
    }

    gFGPaths[idx] = path;
}
