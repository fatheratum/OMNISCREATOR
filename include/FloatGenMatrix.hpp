#pragma once

//==============================================================================
// FloatGenMatrix.hpp
// FLOAT-GEN ARRAY MATRIX — Interactive 3D multidimensional exchange grid
// Golden exchange paths | Mouse-drag rotate | Mouse-wheel zoom
// Click node → inspect force vector + state
// Animated value-flow particles travel along active paths
// C++17 | Header-only | Vulkan/DX12 vertex buffer ready
// OMNIBRAIN INFINITY Universe
//==============================================================================

#include "OmniBrainReserveNexus.hpp"
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

namespace OmniBrain {

//==============================================================================
// MATRIX CAMERA — view/projection for the 3D grid
//==============================================================================
struct MatrixCamera {
    // Spherical orbit
    float   orbitTheta   =  0.4f;  // Horizontal angle (radians)
    float   orbitPhi     =  0.35f; // Vertical angle (radians)
    float   orbitRadius  =  18.0f; // Distance from origin
    float   fovY         =  45.0f; // Degrees
    float   nearZ        =  0.1f;
    float   farZ         = 200.0f;

    // Drag state
    bool    dragging     = false;
    float   lastMouseX   = 0.0f;
    float   lastMouseY   = 0.0f;

    // Derived — computed each frame
    float   eyeX, eyeY, eyeZ;
    float   viewMat[16];
    float   projMat[16];

    //-- Mouse input handlers --------------------------------------------------
    void on_mouse_down(float mx, float my) {
        dragging   = true;
        lastMouseX = mx;
        lastMouseY = my;
    }
    void on_mouse_up() {
        dragging = false;
    }
    void on_mouse_move(float mx, float my) {
        if (!dragging) return;
        float dx = mx - lastMouseX;
        float dy = my - lastMouseY;
        orbitTheta -= dx * 0.008f;
        orbitPhi    = std::clamp(orbitPhi + dy * 0.006f, -1.45f, 1.45f);
        lastMouseX  = mx;
        lastMouseY  = my;
    }
    void on_mouse_wheel(float delta) {
        orbitRadius = std::clamp(orbitRadius - delta * 1.2f, 4.0f, 60.0f);
    }

    //-- Compute eye position and matrices each frame --------------------------
    void update(float aspectRatio) {
        eyeX = orbitRadius * std::cos(orbitPhi) * std::sin(orbitTheta);
        eyeY = orbitRadius * std::sin(orbitPhi);
        eyeZ = orbitRadius * std::cos(orbitPhi) * std::cos(orbitTheta);
        _build_look_at(eyeX, eyeY, eyeZ,  0,0,0,  0,1,0, viewMat);
        _build_perspective(fovY, aspectRatio, nearZ, farZ, projMat);
    }

private:
    static void _build_look_at(float ex,float ey,float ez,
                                float cx,float cy,float cz,
                                float ux,float uy,float uz,
                                float* m) {
        float fx=cx-ex, fy=cy-ey, fz=cz-ez;
        float fl=std::sqrt(fx*fx+fy*fy+fz*fz); fx/=fl; fy/=fl; fz/=fl;
        float sx=fy*uz-fz*uy, sy=fz*ux-fx*uz, sz=fx*uy-fy*ux;
        float sl=std::sqrt(sx*sx+sy*sy+sz*sz); sx/=sl; sy/=sl; sz/=sl;
        float ulx=sy*fz-sz*fy, uly=sz*fx-sx*fz, ulz=sx*fy-sy*fx;
        m[ 0]=sx;  m[ 1]=ulx; m[ 2]=-fx; m[ 3]=0;
        m[ 4]=sy;  m[ 5]=uly; m[ 6]=-fy; m[ 7]=0;
        m[ 8]=sz;  m[ 9]=ulz; m[10]=-fz; m[11]=0;
        m[12]=-(sx*ex+sy*ey+sz*ez);
        m[13]=-(ulx*ex+uly*ey+ulz*ez);
        m[14]= (fx*ex+fy*ey+fz*ez);
        m[15]=1;
    }
    static void _build_perspective(float fovDeg,float aspect,
                                   float zn,float zf,float* m) {
        float f=1.0f/std::tan(fovDeg*0.5f*3.14159265f/180.0f);
        m[ 0]=f/aspect; m[ 1]=0; m[ 2]=0;              m[ 3]=0;
        m[ 4]=0;        m[ 5]=f; m[ 6]=0;              m[ 7]=0;
        m[ 8]=0;        m[ 9]=0; m[10]=(zf+zn)/(zn-zf);m[11]=-1;
        m[12]=0;        m[13]=0; m[14]=2*zf*zn/(zn-zf);m[15]=0;
    }
};

//==============================================================================
// FLOW PARTICLE — travels along a path from nodeA to nodeB
//==============================================================================
struct FlowParticle {
    uint32_t pathIndex;
    float    t;           // [0,1] along path
    float    speed;
    float    r, g, b, a;
    float    size;
    bool     active;
};

//==============================================================================
// NODE INSPECTION RESULT — returned when user clicks a node
//==============================================================================
struct NodeInspection {
    uint32_t    nodeId;
    std::string label;
    float       x, y, z;
    float       forceX, forceY, forceZ;
    double      value;
    double      flow;
    uint32_t    state;          // 0=idle 1=flowing 2=burst 3=saturated
    uint32_t    connectedPaths;
    std::string stateLabel;
    bool        valid = false;
};

//==============================================================================
// MATRIX LINE VERTEX — for rendering exchange path lines
//==============================================================================
struct MatrixLineVertex {
    float x, y, z;
    float r, g, b, a;
    float intensity;
};

//==============================================================================
// FLOAT-GEN ARRAY MATRIX
//==============================================================================
class FloatGenMatrix {
public:
    //--------------------------------------------------------------------------
    // Constructor — initialises from the reserve nexus node/path data
    //--------------------------------------------------------------------------
    explicit FloatGenMatrix(OmniBrainReserveNexus& nexus)
        : m_nexus(nexus)
    {
        m_rng.seed(0xF10A'7600u);
        _rebuild_line_cache();
        _init_flow_particles();
    }

    //--------------------------------------------------------------------------
    // tick() — advance particles, update node states
    //--------------------------------------------------------------------------
    void tick(double deltaTime, float aspectRatio) {
        float dt = static_cast<float>(deltaTime);
        m_simTime += dt;

        // Advance nexus (decays, random path activations)
        m_nexus.tick(deltaTime);

        // Sync node visual states from nexus
        auto& nodes = m_nexus.nodes();
        auto& paths = m_nexus.paths();

        for (auto& node : nodes) {
            // Pulse value
            node.value += std::sin(m_simTime * 0.5f + node.id * 0.3f) * 10.0;
        }

        // Emit flow particles on active paths
        _emit_flow_particles(dt);
        _update_flow_particles(dt);

        // Rebuild line cache (colours change based on flow)
        _rebuild_line_cache();

        m_camera.update(aspectRatio);
    }

    //--------------------------------------------------------------------------
    // on_node_click() — raycast-style click → inspect node
    //--------------------------------------------------------------------------
    NodeInspection on_node_click(uint32_t nodeId) {
        auto& nodes = m_nexus.nodes();
        NodeInspection result;
        if (nodeId >= nodes.size()) return result;

        auto& node = nodes[nodeId];
        result.valid       = true;
        result.nodeId      = node.id;
        result.label       = node.label;
        result.x           = node.x;
        result.y           = node.y;
        result.z           = node.z;
        result.forceX      = node.forceVectorX;
        result.forceY      = node.forceVectorY;
        result.forceZ      = node.forceVectorZ;
        result.value       = node.value;
        result.flow        = node.flow;
        result.state       = node.state;

        // Count connected paths
        uint32_t conn = 0;
        for (auto& path : m_nexus.paths()) {
            if (path.nodeA == nodeId || path.nodeB == nodeId) ++conn;
        }
        result.connectedPaths = conn;

        static const char* stateLabels[] = {
            "IDLE", "FLOWING", "BURST", "SATURATED"
        };
        result.stateLabel = stateLabels[std::min(node.state, 3u)];

        // Visually select it
        for (auto& n : nodes) n.selected = (n.id == nodeId);

        // Trigger a burst on selected node
        node.state = 2;
        _burst_from_node(nodeId, 64);

        return result;
    }

    //--------------------------------------------------------------------------
    // format_inspection() — human-readable string for UI overlay
    //--------------------------------------------------------------------------
    std::string format_inspection(const NodeInspection& insp) const {
        if (!insp.valid) return "";
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "NODE " << insp.label << "\n"
            << "POS    [" << insp.x << ", " << insp.y << ", " << insp.z << "]\n"
            << "FORCE  [" << insp.forceX << ", "
                          << insp.forceY << ", "
                          << insp.forceZ << "]\n"
            << "VALUE  " << insp.value << "\n"
            << "FLOW   " << insp.flow  << "\n"
            << "STATE  " << insp.stateLabel << "\n"
            << "PATHS  " << insp.connectedPaths;
        return oss.str();
    }

    //--------------------------------------------------------------------------
    // Mouse / camera delegates
    //--------------------------------------------------------------------------
    void on_mouse_down(float mx, float my)    { m_camera.on_mouse_down(mx,my); }
    void on_mouse_up()                         { m_camera.on_mouse_up(); }
    void on_mouse_move(float mx, float my)    { m_camera.on_mouse_move(mx,my); }
    void on_mouse_wheel(float delta)           { m_camera.on_mouse_wheel(delta); }

    //--------------------------------------------------------------------------
    // Accessors
    //--------------------------------------------------------------------------
    const MatrixCamera&              camera()       const { return m_camera; }
    MatrixCamera&                    camera()             { return m_camera; }
    const std::vector<MatrixLineVertex>& lineCache() const { return m_lineCache; }
    const std::vector<FlowParticle>& flowParticles()const { return m_flowParticles; }
    OmniBrainReserveNexus&           nexus()              { return m_nexus; }

    // Node screen positions (for click picking)
    const std::vector<std::array<float,3>>& nodePositions() const {
        return m_nodePositions;
    }

private:
    //--------------------------------------------------------------------------
    // Build line vertex cache from current node/path state
    //--------------------------------------------------------------------------
    void _rebuild_line_cache() {
        m_lineCache.clear();
        m_nodePositions.clear();

        auto& nodes = m_nexus.nodes();
        auto& paths = m_nexus.paths();

        // Store node positions for picking
        for (auto& node : nodes) {
            m_nodePositions.push_back({ node.x, node.y, node.z });
        }

        // Build one line (2 verts) per path
        for (auto& path : paths) {
            if (path.nodeA >= nodes.size() || path.nodeB >= nodes.size()) continue;
            auto& A = nodes[path.nodeA];
            auto& B = nodes[path.nodeB];

            float flow_t = static_cast<float>(
                std::min(path.currentFlow / 10000.0, 1.0)
            );
            // Golden exchange path colour — brighter when active
            float r = 0.9f + flow_t * 0.1f;
            float g = 0.65f * flow_t + 0.2f * (1.0f - flow_t);
            float b = 0.05f * (1.0f - flow_t);
            float a = path.active ? (0.7f + flow_t * 0.3f) : 0.15f;

            MatrixLineVertex vA;
            vA.x = A.x; vA.y = A.y; vA.z = A.z;
            vA.r = r; vA.g = g; vA.b = b; vA.a = a;
            vA.intensity = path.active ? 1.0f : 0.3f;

            MatrixLineVertex vB;
            vB.x = B.x; vB.y = B.y; vB.z = B.z;
            vB.r = r; vB.g = g; vB.b = b; vB.a = a;
            vB.intensity = path.active ? 1.0f : 0.3f;

            // Selected node → brighter
            if (A.selected || B.selected) {
                vA.r = vB.r = 1.0f;
                vA.g = vB.g = 1.0f;
                vA.b = vB.b = 0.4f;
                vA.a = vB.a = 1.0f;
            }

            m_lineCache.push_back(vA);
            m_lineCache.push_back(vB);
        }
    }

    //--------------------------------------------------------------------------
    // Flow particles
    //--------------------------------------------------------------------------
    void _init_flow_particles() {
        m_flowParticles.resize(2048);
        for (auto& fp : m_flowParticles) fp.active = false;
    }

    void _emit_flow_particles(float dt) {
        auto& paths = m_nexus.paths();
        m_emitAccum += 60.0f * dt;

        while (m_emitAccum >= 1.0f) {
            m_emitAccum -= 1.0f;

            // Find an active path
            std::uniform_int_distribution<size_t> pi(0, paths.size()-1);
            size_t idx = pi(m_rng);
            if (!paths[idx].active) continue;

            for (auto& fp : m_flowParticles) {
                if (fp.active) continue;
                fp.pathIndex = static_cast<uint32_t>(idx);
                fp.t         = 0.0f;
                fp.speed     = 0.3f + _rand() * 0.5f;
                fp.r = 1.0f; fp.g = 0.82f; fp.b = 0.1f; fp.a = 1.0f;
                fp.size      = 0.08f + _rand() * 0.12f;
                fp.active    = true;
                break;
            }
        }
    }

    void _update_flow_particles(float dt) {
        auto& paths = m_nexus.paths();
        for (auto& fp : m_flowParticles) {
            if (!fp.active) continue;
            if (fp.pathIndex >= paths.size()) { fp.active = false; continue; }
            fp.t += fp.speed * dt;
            if (fp.t >= 1.0f) fp.active = false;
            // Fade near endpoints
            float edge = std::min(fp.t, 1.0f - fp.t) * 4.0f;
            fp.a = std::min(1.0f, edge);
        }
    }

    void _burst_from_node(uint32_t nodeId, uint32_t count) {
        auto& paths = m_nexus.paths();
        uint32_t spawned = 0;
        for (auto& fp : m_flowParticles) {
            if (fp.active || spawned >= count) continue;
            // Find a path connected to this node
            for (size_t pi = 0; pi < paths.size(); ++pi) {
                if (paths[pi].nodeA == nodeId || paths[pi].nodeB == nodeId) {
                    fp.pathIndex = static_cast<uint32_t>(pi);
                    fp.t         = (paths[pi].nodeA == nodeId) ? 0.0f : 1.0f;
                    fp.speed     = 0.8f + _rand() * 0.8f;
                    fp.r = 1.0f; fp.g = 1.0f; fp.b = 0.5f; fp.a = 1.0f;
                    fp.size = 0.15f;
                    fp.active = true;
                    ++spawned;
                    break;
                }
            }
        }
    }

    float _rand() {
        std::uniform_real_distribution<float> d(0.0f, 1.0f);
        return d(m_rng);
    }

    //--------------------------------------------------------------------------
    // Members
    //--------------------------------------------------------------------------
    OmniBrainReserveNexus&             m_nexus;
    MatrixCamera                       m_camera;
    std::vector<MatrixLineVertex>      m_lineCache;
    std::vector<FlowParticle>          m_flowParticles;
    std::vector<std::array<float,3>>   m_nodePositions;
    float                              m_simTime   = 0.0f;
    float                              m_emitAccum = 0.0f;
    std::mt19937                       m_rng;
};

} // namespace OmniBrain
