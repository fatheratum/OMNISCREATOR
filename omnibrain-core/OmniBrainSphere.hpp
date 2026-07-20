#pragma once

//==============================================================================
// OmniBrainSphere.hpp
// CENTRAL HOLOGRAPHIC OMNIBRAIN SPHERE
// Real-time pulsing golden sphere — multiple rotating holographic layers
// Inner core glow | Latitude/longitude sacred geometry rings
// Streaming golden light-code particles radiating outward
// Feeds vertex/instance data directly to Vulkan/DX12 command buffers
// C++17 | Header-only | OMNIBRAIN INFINITY Universe
//==============================================================================

#include <cstdint>
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include "OmniBrainReserveNexus.hpp"

namespace OmniBrain {

static constexpr float  PI       = 3.14159265358979323846f;
static constexpr float  TAU      = 6.28318530717958647692f;
static constexpr float  PHI      = 1.61803398874989484820f; // Golden ratio

//==============================================================================
// SPHERE VERTEX — fed to vertex buffer
//==============================================================================
struct SphereVertex {
    float x, y, z;        // Position
    float nx, ny, nz;     // Normal
    float u, v;           // UV
    float r, g, b, a;     // Light-code color (golden)
    float intensity;       // Glow intensity
    float layer;           // Which holographic layer [0..N]
};

//==============================================================================
// SPHERE RING — latitude/longitude sacred geometry ring
//==============================================================================
struct SphereRing {
    float     latitude;        // Radians from equator
    float     radius;          // Ring radius at this latitude
    float     rotationSpeed;   // Radians per second
    float     currentAngle;    // Current rotation angle
    float     glowIntensity;   // [0,1]
    uint32_t  segmentCount;
    bool      active;
    std::vector<SphereVertex> vertices;
};

//==============================================================================
// HOLOGRAPHIC LAYER — concentric transparent sphere shell
//==============================================================================
struct HolographicLayer {
    float     radius;
    float     opacity;
    float     rotationSpeedX;
    float     rotationSpeedY;
    float     rotationSpeedZ;
    float     angleX, angleY, angleZ;
    float     lightCodeDensity;
    uint32_t  latSegments;
    uint32_t  lonSegments;
    std::vector<SphereVertex> vertices;
    std::vector<uint32_t>     indices;
};

//==============================================================================
// LIGHT-CODE PARTICLE — radiating outward from sphere surface
//==============================================================================
struct LightCodeParticle {
    float x, y, z;            // World position
    float vx, vy, vz;         // Velocity
    float r, g, b, a;         // Color (golden to white)
    float size;                // Point size
    float life;                // [0,1] remaining life
    float maxLife;
    float speed;
    uint32_t type;             // 0=stream 1=burst 2=sacred 3=void
    bool  active;
};

//==============================================================================
// SPHERE INSTANCE DATA — for GPU instancing
//==============================================================================
struct SphereInstanceData {
    float worldMatrix[16];     // 4x4 row-major
    float color[4];
    float intensity;
    float pad[3];
};

//==============================================================================
// OMNIBRAIN SPHERE — Central holographic intelligence
//==============================================================================
class OmniBrainSphere {
public:
    //--------------------------------------------------------------------------
    // Config
    //--------------------------------------------------------------------------
    struct Config {
        float    coreRadius         = 0.55f;
        uint32_t layerCount         = 5;
        uint32_t ringsPerLayer      = 12;
        uint32_t sacredRingCount    = 9;   // Latitude rings
        uint32_t maxParticles       = 8192;
        float    outerRadius        = 3.8f;
        float    particleEmitRate   = 480.0f;  // Per second
        float    baseGlowR          = 1.00f;
        float    baseGlowG          = 0.78f;
        float    baseGlowB          = 0.10f;
        float    pulseSpeed         = 1.2f;
        bool     sacredGeometry     = true;
        bool     lightCodeStream    = true;
    };

    explicit OmniBrainSphere(const Config& cfg = Config{}) : m_cfg(cfg) {
        m_rng.seed(0xDEAD'BEEF);
        _build_core();
        _build_holographic_layers();
        _build_sacred_rings();
        _init_particles();
    }

    //--------------------------------------------------------------------------
    // tick() — Advance sphere state, called every frame
    //--------------------------------------------------------------------------
    void tick(double deltaTime, const ReserveMetrics& metrics) {
        float dt = static_cast<float>(deltaTime);
        m_simTime += dt;

        // Pulse core based on solvency index
        double si         = metrics.solvencyIndex.load();
        m_coreGlow        = 0.5f + 0.5f * std::sin(m_simTime * m_cfg.pulseSpeed);
        m_coreGlow       *= static_cast<float>(std::min(si, 2.0)) * 0.7f;

        // Scale core radius with liquid assets (normalized)
        double la         = metrics.liquidAssets.load();
        m_coreScale       = 1.0f + 0.25f * static_cast<float>(
            std::tanh((la - 100000.0) / 80000.0)
        );

        // Rotate holographic layers
        for (size_t i = 0; i < m_layers.size(); ++i) {
            auto& L      = m_layers[i];
            float speed  = 0.15f + i * 0.07f;
            float dir    = (i % 2 == 0) ? 1.0f : -1.0f;
            L.angleX    += dt * speed * dir * 0.4f;
            L.angleY    += dt * speed * dir;
            L.angleZ    += dt * speed * dir * 0.6f;
            // Update opacity with SI
            L.opacity    = std::clamp(
                0.08f + static_cast<float>(si) * 0.05f - i * 0.01f, 0.02f, 0.3f
            );
        }

        // Rotate sacred rings
        for (size_t i = 0; i < m_sacredRings.size(); ++i) {
            auto& ring        = m_sacredRings[i];
            float ringSpeed   = ring.rotationSpeed
                * (1.0f + static_cast<float>(metrics.yieldRate.load()) * 2.0f);
            ring.currentAngle += dt * ringSpeed;
            ring.glowIntensity = 0.4f + 0.6f * std::abs(
                std::sin(m_simTime * 0.7f + i * 0.4f)
            );
        }

        // Emit and update light-code particles
        if (m_cfg.lightCodeStream) {
            _emit_particles(dt, metrics);
            _update_particles(dt);
        }
    }

    //--------------------------------------------------------------------------
    // trigger_burst() — Called on domain acquire / organism circulate
    //--------------------------------------------------------------------------
    void trigger_burst(float originX, float originY, float originZ,
                       uint32_t count, float r, float g, float b) {
        uint32_t spawned = 0;
        for (auto& p : m_particles) {
            if (!p.active && spawned < count) {
                _spawn_particle(p, originX, originY, originZ, r, g, b,
                                LightCodeType::Burst);
                ++spawned;
            }
        }
    }

    //--------------------------------------------------------------------------
    // Accessors — return data ready for GPU upload
    //--------------------------------------------------------------------------
    const std::vector<HolographicLayer>&  layers()      const { return m_layers; }
    const std::vector<SphereRing>&        sacredRings() const { return m_sacredRings; }
    const std::vector<LightCodeParticle>& particles()   const { return m_particles; }
    const std::vector<SphereVertex>&      coreVerts()   const { return m_coreVerts; }
    const std::vector<uint32_t>&          coreIndices() const { return m_coreIndices; }

    float coreGlow()  const { return m_coreGlow; }
    float coreScale() const { return m_coreScale; }
    float simTime()   const { return m_simTime; }

    // Golden color helper
    static void golden_color(float t, float& r, float& g, float& b) {
        // t=[0,1]: inner core white-gold → outer deep-gold → edge orange
        r = 1.0f;
        g = 0.72f + 0.18f * (1.0f - t);
        b = 0.05f + 0.15f * (1.0f - t) * (1.0f - t);
    }

private:
    enum class LightCodeType : uint8_t {
        Stream = 0, Burst = 1, Sacred = 2, Void = 3
    };

    //--------------------------------------------------------------------------
    // Build core sphere mesh (icosphere subdivision)
    //--------------------------------------------------------------------------
    void _build_core() {
        // UV sphere with lat/lon
        uint32_t stacks = 32, slices = 64;
        float R = m_cfg.coreRadius;

        for (uint32_t st = 0; st <= stacks; ++st) {
            float phi_v = PI * st / stacks;         // 0 → PI
            float y     = R * std::cos(phi_v);
            float r_xy  = R * std::sin(phi_v);

            for (uint32_t sl = 0; sl <= slices; ++sl) {
                float theta = TAU * sl / slices;
                float x = r_xy * std::cos(theta);
                float z = r_xy * std::sin(theta);

                float cr, cg, cb;
                golden_color(static_cast<float>(st) / stacks, cr, cg, cb);

                SphereVertex v;
                v.x = x; v.y = y; v.z = z;
                float len = std::sqrt(x*x + y*y + z*z);
                v.nx = x/len; v.ny = y/len; v.nz = z/len;
                v.u  = static_cast<float>(sl) / slices;
                v.v  = static_cast<float>(st) / stacks;
                v.r  = cr; v.g = cg; v.b = cb; v.a = 1.0f;
                v.intensity = 1.0f - static_cast<float>(st) / stacks * 0.3f;
                v.layer = 0.0f;
                m_coreVerts.push_back(v);
            }
        }

        // Indices
        for (uint32_t st = 0; st < stacks; ++st) {
            for (uint32_t sl = 0; sl < slices; ++sl) {
                uint32_t a = st * (slices + 1) + sl;
                uint32_t b = a + slices + 1;
                m_coreIndices.push_back(a);
                m_coreIndices.push_back(b);
                m_coreIndices.push_back(a + 1);
                m_coreIndices.push_back(b);
                m_coreIndices.push_back(b + 1);
                m_coreIndices.push_back(a + 1);
            }
        }
    }

    //--------------------------------------------------------------------------
    // Build holographic concentric shells
    //--------------------------------------------------------------------------
    void _build_holographic_layers() {
        m_layers.resize(m_cfg.layerCount);
        for (uint32_t i = 0; i < m_cfg.layerCount; ++i) {
            auto& L     = m_layers[i];
            L.radius    = m_cfg.coreRadius * (1.5f + i * 0.6f);
            L.opacity   = 0.12f - i * 0.015f;
            float speed = 0.12f + i * 0.06f;
            float dir   = (i % 2 == 0) ? 1.0f : -1.0f;
            L.rotationSpeedX = speed * 0.4f * dir;
            L.rotationSpeedY = speed * dir;
            L.rotationSpeedZ = speed * 0.6f * dir;
            L.angleX = L.angleY = L.angleZ = 0.0f;
            L.lightCodeDensity = 0.5f + i * 0.1f;
            L.latSegments = 16 + i * 4;
            L.lonSegments = 32 + i * 8;

            _build_shell_mesh(L);
        }
    }

    void _build_shell_mesh(HolographicLayer& L) {
        uint32_t stk = L.latSegments, slc = L.lonSegments;
        float R = L.radius;

        for (uint32_t st = 0; st <= stk; ++st) {
            float phi_v = PI * st / stk;
            float y     = R * std::cos(phi_v);
            float r_xy  = R * std::sin(phi_v);

            for (uint32_t sl = 0; sl <= slc; ++sl) {
                float theta = TAU * sl / slc;
                float x = r_xy * std::cos(theta);
                float z = r_xy * std::sin(theta);

                SphereVertex v;
                v.x = x; v.y = y; v.z = z;
                float len = std::max(1e-6f, std::sqrt(x*x+y*y+z*z));
                v.nx = x/len; v.ny = y/len; v.nz = z/len;
                v.u  = static_cast<float>(sl)/slc;
                v.v  = static_cast<float>(st)/stk;
                // Golden glow falls off by layer
                float cr, cg, cb;
                golden_color(static_cast<float>(st)/stk, cr, cg, cb);
                v.r = cr; v.g = cg; v.b = cb;
                v.a = L.opacity;
                v.intensity = L.lightCodeDensity;
                v.layer     = static_cast<float>(
                    &L - m_layers.data()
                );
                L.vertices.push_back(v);
            }
        }
        for (uint32_t st = 0; st < stk; ++st) {
            for (uint32_t sl = 0; sl < slc; ++sl) {
                uint32_t a = st * (slc+1) + sl;
                uint32_t b = a + slc + 1;
                L.indices.push_back(a);   L.indices.push_back(b);
                L.indices.push_back(a+1); L.indices.push_back(b);
                L.indices.push_back(b+1); L.indices.push_back(a+1);
            }
        }
    }

    //--------------------------------------------------------------------------
    // Build sacred geometry latitude/longitude rings
    //--------------------------------------------------------------------------
    void _build_sacred_rings() {
        m_sacredRings.resize(m_cfg.sacredRingCount);
        for (uint32_t i = 0; i < m_cfg.sacredRingCount; ++i) {
            auto& ring = m_sacredRings[i];
            // Spread across latitudes including equator and phi-spaced
            float t          = static_cast<float>(i) / (m_cfg.sacredRingCount - 1);
            ring.latitude    = (t - 0.5f) * PI;
            ring.radius      = m_cfg.coreRadius * 1.02f
                             * std::cos(ring.latitude);
            ring.rotationSpeed = 0.3f + i * 0.07f * ((i % 2) ? 1.f : -1.f);
            ring.currentAngle  = 0.0f;
            ring.glowIntensity = 0.7f;
            ring.segmentCount  = 128;
            ring.active        = true;
            _build_ring_verts(ring, i);
        }
    }

    void _build_ring_verts(SphereRing& ring, uint32_t idx) {
        float y = m_cfg.coreRadius * 1.02f * std::sin(ring.latitude);
        ring.vertices.clear();
        ring.vertices.reserve(ring.segmentCount + 1);

        for (uint32_t s = 0; s <= ring.segmentCount; ++s) {
            float theta = TAU * s / ring.segmentCount;
            SphereVertex v;
            v.x = ring.radius * std::cos(theta);
            v.y = y;
            v.z = ring.radius * std::sin(theta);
            v.nx = v.x / std::max(1e-6f, ring.radius);
            v.ny = 0.0f;
            v.nz = v.z / std::max(1e-6f, ring.radius);
            v.u  = static_cast<float>(s) / ring.segmentCount;
            v.v  = static_cast<float>(idx) / m_cfg.sacredRingCount;
            // Sacred rings are brighter gold
            v.r = 1.0f; v.g = 0.88f; v.b = 0.3f; v.a = 0.9f;
            v.intensity = ring.glowIntensity;
            v.layer     = static_cast<float>(idx);
            ring.vertices.push_back(v);
        }
    }

    //--------------------------------------------------------------------------
    // Particle system
    //--------------------------------------------------------------------------
    void _init_particles() {
        m_particles.resize(m_cfg.maxParticles);
        for (auto& p : m_particles) p.active = false;
    }

    void _emit_particles(float dt, const ReserveMetrics& metrics) {
        m_emitAccum += m_cfg.particleEmitRate
            * static_cast<float>(metrics.yieldRate.load() * 4.0)
            * dt;

        while (m_emitAccum >= 1.0f) {
            m_emitAccum -= 1.0f;
            // Find a free slot
            for (auto& p : m_particles) {
                if (!p.active) {
                    // Random point on sphere surface
                    float phi_v = _rand_float() * TAU;
                    float theta = std::acos(2.0f * _rand_float() - 1.0f);
                    float R     = m_cfg.coreRadius;
                    float sx    = R * std::sin(theta) * std::cos(phi_v);
                    float sy    = R * std::cos(theta);
                    float sz    = R * std::sin(theta) * std::sin(phi_v);
                    _spawn_particle(p, sx, sy, sz,
                        m_cfg.baseGlowR, m_cfg.baseGlowG, m_cfg.baseGlowB,
                        LightCodeType::Stream);
                    break;
                }
            }
        }
    }

    void _spawn_particle(LightCodeParticle& p,
                         float ox, float oy, float oz,
                         float r, float g, float b,
                         LightCodeType type) {
        // Radiate outward from origin
        float len = std::sqrt(ox*ox + oy*oy + oz*oz);
        if (len < 1e-6f) { ox = 0; oy = 1; oz = 0; len = 1.0f; }

        float speed = 0.5f + _rand_float() * 2.5f;
        p.x  = ox; p.y = oy; p.z = oz;
        p.vx = (ox/len) * speed + (_rand_float()-0.5f)*0.3f;
        p.vy = (oy/len) * speed + (_rand_float()-0.5f)*0.3f;
        p.vz = (oz/len) * speed + (_rand_float()-0.5f)*0.3f;
        p.r  = r; p.g = g; p.b = b; p.a = 0.9f;
        p.size    = 0.02f + _rand_float() * 0.06f;
        p.maxLife = 0.8f + _rand_float() * 2.2f;
        p.life    = p.maxLife;
        p.speed   = speed;
        p.type    = static_cast<uint32_t>(type);
        p.active  = true;
    }

    void _update_particles(float dt) {
        for (auto& p : m_particles) {
            if (!p.active) continue;
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;

            // Fade and shrink
            p.life -= dt;
            float t = p.life / p.maxLife;
            p.a    = t * 0.9f;
            p.size = p.size * (0.98f + t * 0.02f);

            // Color shift: gold → white → transparent
            p.g = std::max(0.0f, p.g - dt * 0.05f);
            p.b = std::min(1.0f, p.b + dt * 0.1f);

            if (p.life <= 0.0f || p.a < 0.01f) p.active = false;
        }
    }

    float _rand_float() {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(m_rng);
    }

    //--------------------------------------------------------------------------
    // Member data
    //--------------------------------------------------------------------------
    Config                           m_cfg;
    std::vector<SphereVertex>        m_coreVerts;
    std::vector<uint32_t>            m_coreIndices;
    std::vector<HolographicLayer>    m_layers;
    std::vector<SphereRing>          m_sacredRings;
    std::vector<LightCodeParticle>   m_particles;
    float                            m_coreGlow   = 1.0f;
    float                            m_coreScale  = 1.0f;
    float                            m_simTime    = 0.0f;
    float                            m_emitAccum  = 0.0f;
    std::mt19937                     m_rng;
};

} // namespace OmniBrain
