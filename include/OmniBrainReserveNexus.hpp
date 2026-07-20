#pragma once

//==============================================================================
// OmniBrainReserveNexus.hpp
// THE LIVING ECONOMIC GODHEAD — Central Reserve Intelligence
// Equations: R = F + D | SI = (LA + YP) / OO
// Full C++17 | Header-only | DX12/Vulkan compute-shader ready
// OMNIBRAIN • INFINITY Universe • Naru Atum Protocol
//==============================================================================

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>
#include <atomic>
#include <mutex>

namespace OmniBrain {

//==============================================================================
// CONSTANTS — Sacred Geometry of the Reserve
//==============================================================================
static constexpr double SOLVENCY_TARGET        = 1.0;
static constexpr double MIN_YIELD_RATE         = 0.001;
static constexpr double MAX_YIELD_RATE         = 0.25;
static constexpr double FLOAT_DECAY_RATE       = 0.0003;
static constexpr double DEPOSIT_AMPLIFIER      = 1.618033988749;  // Golden ratio
static constexpr uint32_t MAX_ORGANISMS        = 64;
static constexpr uint32_t MAX_DOMAINS          = 16;
static constexpr uint32_t MAX_PAYMENT_LOG      = 512;
static constexpr uint32_t PARTICLE_BURST_COUNT = 8192;

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
struct Domain;
struct AtomicPayment;
class  EconomicOrganism;

//==============================================================================
// DOMAIN — Private Wallet Domain
//==============================================================================
struct Domain {
    std::string  id;
    std::string  label;
    double       balance        = 0.0;
    double       pendingYield   = 0.0;
    bool         active         = true;

    // Light-code burst data (fed to particle system each acquire)
    float        burstOriginX   = 0.0f;
    float        burstOriginY   = 0.0f;
    float        burstOriginZ   = 0.0f;
    uint32_t     burstParticles = 512;
};

//==============================================================================
// ATOMIC PAYMENT — Settlement confirmation struct
//==============================================================================
struct AtomicPayment {
    std::string  paymentId;
    std::string  fromDomain;
    std::string  toDomain;
    double       amount         = 0.0;
    double       yieldApplied   = 0.0;
    uint64_t     timestamp      = 0;
    bool         unencumbered   = true;
    bool         complete       = false;
    std::string  status         = "PENDING";  // PENDING | SETTLED | UNENCUMBERED·PRIVATE·COMPLETE
};

//==============================================================================
// FLOAT-GEN NODE — Single node in the 3D multidimensional exchange grid
//==============================================================================
struct FloatGenNode {
    uint32_t     id;
    float        x, y, z;                    // World-space position in grid
    float        forceVectorX  = 0.0f;
    float        forceVectorY  = 0.0f;
    float        forceVectorZ  = 0.0f;
    double       value         = 0.0;
    double       flow          = 0.0;        // Active flow along paths
    bool         active        = true;
    bool         selected      = false;      // Clicked / inspected
    uint32_t     state         = 0;          // 0=idle 1=flowing 2=burst 3=saturated
    std::string  label;
};

//==============================================================================
// FLOAT-GEN PATH — Exchange path between two nodes
//==============================================================================
struct FloatGenPath {
    uint32_t     nodeA;
    uint32_t     nodeB;
    double       bandwidth     = 1.0;
    double       currentFlow   = 0.0;
    bool         active        = false;
    float        particleT     = 0.0f;      // Particle position [0,1] along path
    float        particleSpeed = 0.6f;
};

//==============================================================================
// RESERVE NEXUS — TOP BAR METRICS (atomic, live-updating)
//==============================================================================
struct ReserveMetrics {
    std::atomic<double>   solvencyIndex   { 1.0 };    // SI = (LA + YP) / OO
    std::atomic<double>   liquidAssets    { 100000.0 };
    std::atomic<double>   yieldPool       { 8500.0 };
    std::atomic<double>   floatVolume     { 42000.0 };
    std::atomic<uint32_t> organismsOnline { 4 };
    std::atomic<double>   reserveTotal    { 0.0 };    // R = F + D
    std::atomic<double>   depositTotal    { 0.0 };
    std::atomic<double>   yieldRate       { 0.07 };
};

//==============================================================================
// OMNIBRAIN RESERVE NEXUS — THE LIVING ECONOMIC ENGINE
//==============================================================================
class OmniBrainReserveNexus {
public:
    //--------------------------------------------------------------------------
    // Construction
    //--------------------------------------------------------------------------
    OmniBrainReserveNexus() {
        m_rng.seed(static_cast<uint32_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        ));
        _init_domains();
        _init_float_gen_matrix();
        recalculate_metrics();
    }

    //--------------------------------------------------------------------------
    // CORE EQUATION: R = F + D
    // Reserve = Float + Deposits
    //--------------------------------------------------------------------------
    double calculate_reserve() const {
        return m_metrics.floatVolume.load() + m_metrics.depositTotal.load();
    }

    //--------------------------------------------------------------------------
    // CORE EQUATION: SI = (LA + YP) / OO
    // Solvency Index = (Liquid Assets + Yield Pool) / Organisms Online
    //--------------------------------------------------------------------------
    double calculate_solvency_index() const {
        double oo = static_cast<double>(m_metrics.organismsOnline.load());
        if (oo <= 0.0) return 0.0;
        return (m_metrics.liquidAssets.load() + m_metrics.yieldPool.load()) / oo;
    }

    //--------------------------------------------------------------------------
    // CORE EQUATION: YP = LA * YieldRate
    //--------------------------------------------------------------------------
    double calculate_yield(double principal, double rate = -1.0) const {
        double r = (rate < 0.0) ? m_metrics.yieldRate.load() : rate;
        r = std::clamp(r, MIN_YIELD_RATE, MAX_YIELD_RATE);
        return principal * r;
    }

    //--------------------------------------------------------------------------
    // acquire_deposit() — Domain acquires a deposit, triggers light-code burst
    //--------------------------------------------------------------------------
    double acquire_deposit(uint32_t domainIndex, double amount = -1.0) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (domainIndex >= m_domains.size()) return 0.0;

        // Auto-generate deposit amount if not specified
        if (amount < 0.0) {
            std::uniform_real_distribution<double> dist(500.0, 25000.0);
            amount = dist(m_rng) * DEPOSIT_AMPLIFIER;
        }

        Domain& domain     = m_domains[domainIndex];
        double  yieldEarned = calculate_yield(amount);
        domain.balance     += amount + yieldEarned;
        domain.pendingYield = yieldEarned;

        // Update global metrics
        double prev = m_metrics.depositTotal.load();
        m_metrics.depositTotal.store(prev + amount);
        m_metrics.yieldPool.store(m_metrics.yieldPool.load() + yieldEarned);
        m_metrics.liquidAssets.store(m_metrics.liquidAssets.load() + amount * 0.72);

        // Update Float-Gen matrix — propagate flow from domain's nearest node
        _propagate_flow(domainIndex, amount);

        // Trigger burst particle data
        domain.burstParticles = static_cast<uint32_t>(
            512 + (amount / 1000.0) * 128
        );

        recalculate_metrics();

        // Log atomic payment
        _log_payment("DOMAIN-" + std::to_string(domainIndex),
                     "RESERVE-NEXUS", amount, yieldEarned);

        return amount + yieldEarned;
    }

    //--------------------------------------------------------------------------
    // tender_for_solvency() — SPACE key: restores SI >= 1.0
    //--------------------------------------------------------------------------
    void tender_for_solvency() {
        std::lock_guard<std::mutex> lock(m_mutex);

        double si = calculate_solvency_index();
        if (si >= SOLVENCY_TARGET) return;

        // Infuse liquid assets to restore solvency
        double deficit = SOLVENCY_TARGET - si;
        double infusion = deficit
            * static_cast<double>(m_metrics.organismsOnline.load())
            * 1.1;

        m_metrics.liquidAssets.store(m_metrics.liquidAssets.load() + infusion);
        m_metrics.floatVolume.store(m_metrics.floatVolume.load() + infusion * 0.5);

        // Activate ALL paths in Float-Gen matrix
        for (auto& path : m_paths) {
            path.active        = true;
            path.currentFlow   = infusion / m_paths.size();
            path.particleSpeed = 1.2f;
        }

        recalculate_metrics();
        _log_payment("TENDER-SOLVENCY", "ALL-DOMAINS",
                     infusion, calculate_yield(infusion));
    }

    //--------------------------------------------------------------------------
    // establish_payment() — E key: random atomic settlement
    //--------------------------------------------------------------------------
    AtomicPayment establish_payment(
        const std::string& from = "",
        const std::string& to   = "",
        double amount           = -1.0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::uniform_real_distribution<double> amtDist(100.0, 9999.0);
        std::uniform_int_distribution<uint32_t> domDist(0,
            static_cast<uint32_t>(m_domains.size() - 1));

        AtomicPayment pmt;
        pmt.paymentId   = _gen_payment_id();
        pmt.fromDomain  = from.empty()
            ? ("DOMAIN-" + std::to_string(domDist(m_rng))) : from;
        pmt.toDomain    = to.empty()
            ? ("DOMAIN-" + std::to_string(domDist(m_rng))) : to;
        pmt.amount      = (amount < 0.0) ? amtDist(m_rng) : amount;
        pmt.yieldApplied = calculate_yield(pmt.amount);
        pmt.timestamp   = _now_ns();
        pmt.unencumbered = true;
        pmt.complete    = true;
        pmt.status      = "UNENCUMBERED \xE2\x80\xA2 PRIVATE \xE2\x80\xA2 COMPLETE";

        m_metrics.floatVolume.store(
            m_metrics.floatVolume.load() + pmt.amount * 0.1
        );
        recalculate_metrics();

        // Activate a random path for the visual flow
        if (!m_paths.empty()) {
            std::uniform_int_distribution<size_t> pi(0, m_paths.size() - 1);
            m_paths[pi(m_rng)].active = true;
        }

        m_paymentLog.push_back(pmt);
        if (m_paymentLog.size() > MAX_PAYMENT_LOG)
            m_paymentLog.erase(m_paymentLog.begin());

        return pmt;
    }

    //--------------------------------------------------------------------------
    // recalculate_metrics() — recomputes SI and R, called after every mutation
    //--------------------------------------------------------------------------
    void recalculate_metrics() {
        double si = calculate_solvency_index();
        m_metrics.solvencyIndex.store(si);
        m_metrics.reserveTotal.store(calculate_reserve());
    }

    //--------------------------------------------------------------------------
    // tick() — Called every frame/update cycle to evolve metrics organically
    //--------------------------------------------------------------------------
    void tick(double deltaTime) {
        // Float volume decays slightly each tick (natural drain)
        double fv = m_metrics.floatVolume.load();
        fv *= (1.0 - FLOAT_DECAY_RATE * deltaTime);
        m_metrics.floatVolume.store(fv);

        // Yield rate oscillates
        double yr = m_metrics.yieldRate.load();
        yr += std::sin(m_simTime * 0.3) * 0.0001 * deltaTime;
        yr  = std::clamp(yr, MIN_YIELD_RATE, MAX_YIELD_RATE);
        m_metrics.yieldRate.store(yr);

        // Liquid assets drift
        double la = m_metrics.liquidAssets.load();
        la += (fv * yr * deltaTime * 0.01);
        m_metrics.liquidAssets.store(la);

        // Animate Float-Gen path particles
        for (auto& path : m_paths) {
            if (path.active) {
                path.particleT += path.particleSpeed * static_cast<float>(deltaTime);
                if (path.particleT >= 1.0f) {
                    path.particleT   = 0.0f;
                    path.currentFlow *= 0.95;
                    if (path.currentFlow < 0.001) path.active = false;
                }
            }
        }

        // Randomly activate a path occasionally
        {
            std::uniform_real_distribution<double> chance(0.0, 1.0);
            if (chance(m_rng) < 0.02 * deltaTime && !m_paths.empty()) {
                std::uniform_int_distribution<size_t> pi(0, m_paths.size() - 1);
                auto& p     = m_paths[pi(m_rng)];
                p.active    = true;
                p.currentFlow = 500.0 + chance(m_rng) * 5000.0;
                p.particleT = 0.0f;
            }
        }

        m_simTime += deltaTime;
        recalculate_metrics();
    }

    //--------------------------------------------------------------------------
    // Accessors
    //--------------------------------------------------------------------------
    const ReserveMetrics&            metrics()     const { return m_metrics; }
    std::vector<Domain>&             domains()           { return m_domains; }
    const std::vector<Domain>&       domains()     const { return m_domains; }
    std::vector<FloatGenNode>&       nodes()             { return m_nodes; }
    const std::vector<FloatGenNode>& nodes()       const { return m_nodes; }
    std::vector<FloatGenPath>&       paths()             { return m_paths; }
    const std::vector<FloatGenPath>& paths()       const { return m_paths; }
    const std::vector<AtomicPayment>& paymentLog() const { return m_paymentLog; }
    double simTime() const { return m_simTime; }

private:
    //--------------------------------------------------------------------------
    // Private helpers
    //--------------------------------------------------------------------------
    void _init_domains() {
        m_domains = {
            { "DOM-0001", "NARU-CORE",        50000.0, 0.0, true },
            { "DOM-0002", "ATUM-RESERVE",     38000.0, 0.0, true },
            { "DOM-0003", "SYNDICATE-ALPHA",  22000.0, 0.0, true },
            { "DOM-0004", "LIGHT-CODE-VAULT", 17500.0, 0.0, true },
            { "DOM-0005", "INFINITY-FUND",    61000.0, 0.0, true },
            { "DOM-0006", "GENESIS-POOL",     9800.0,  0.0, true },
            { "DOM-0007", "MATRIX-SINK",      14200.0, 0.0, true },
            { "DOM-0008", "VOID-BRIDGE",      5500.0,  0.0, true },
        };
        // Assign 3D burst origins (fed to particle system)
        for (size_t i = 0; i < m_domains.size(); ++i) {
            m_domains[i].burstOriginX = -2.0f + static_cast<float>(i) * 0.57f;
            m_domains[i].burstOriginY =  1.5f - static_cast<float>(i) * 0.2f;
            m_domains[i].burstOriginZ =  0.0f;
        }
    }

    void _init_float_gen_matrix() {
        // 5x5x2 = 50 node 3D grid
        uint32_t id = 0;
        for (int z = 0; z < 2; ++z)
        for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 5; ++x) {
            FloatGenNode node;
            node.id   = id++;
            node.x    = (x - 2) * 1.8f;
            node.y    = (y - 2) * 1.8f;
            node.z    = (z - 0) * 3.0f;
            node.value = 1000.0 + (x + y * 5 + z * 25) * 200.0;
            node.label = "N-" + std::to_string(node.id);
            node.forceVectorX = static_cast<float>(x - 2) * 0.3f;
            node.forceVectorY = static_cast<float>(y - 2) * 0.3f;
            node.forceVectorZ = static_cast<float>(z)     * 0.2f;
            m_nodes.push_back(node);
        }

        // Build exchange paths (golden paths connecting the grid)
        _build_paths();
    }

    void _build_paths() {
        // Connect each node to its neighbors (X/Y/Z adjacent)
        uint32_t W = 5, H = 5, D = 2;
        for (uint32_t z = 0; z < D; ++z)
        for (uint32_t y = 0; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x) {
            uint32_t idx = z * W * H + y * W + x;
            if (x + 1 < W) m_paths.push_back({ idx, idx + 1,   1.0, 0.0, false, 0.0f, 0.5f });
            if (y + 1 < H) m_paths.push_back({ idx, idx + W,   1.0, 0.0, false, 0.0f, 0.5f });
            if (z + 1 < D) m_paths.push_back({ idx, idx + W*H, 1.2, 0.0, false, 0.0f, 0.7f });
        }
        // Add diagonal golden paths
        for (uint32_t i = 0; i < m_nodes.size(); i += 7) {
            uint32_t j = (i + 13) % static_cast<uint32_t>(m_nodes.size());
            m_paths.push_back({ i, j, 1.618f, 0.0, false, 0.0f, 0.8f });
        }
    }

    void _propagate_flow(uint32_t domainIndex, double amount) {
        // Map domain to nearest grid node and activate outgoing paths
        uint32_t rootNode = (domainIndex * 6) % static_cast<uint32_t>(m_nodes.size());
        m_nodes[rootNode].state = 2; // burst
        m_nodes[rootNode].flow  = amount;

        for (auto& path : m_paths) {
            if (path.nodeA == rootNode || path.nodeB == rootNode) {
                path.active      = true;
                path.currentFlow = amount * 0.33;
                path.particleT   = 0.0f;
                path.particleSpeed = 0.6f + static_cast<float>(amount / 50000.0);
            }
        }
    }

    void _log_payment(const std::string& from, const std::string& to,
                      double amount, double yield) {
        AtomicPayment pmt;
        pmt.paymentId    = _gen_payment_id();
        pmt.fromDomain   = from;
        pmt.toDomain     = to;
        pmt.amount       = amount;
        pmt.yieldApplied = yield;
        pmt.timestamp    = _now_ns();
        pmt.unencumbered = true;
        pmt.complete     = true;
        pmt.status       = "UNENCUMBERED \xE2\x80\xA2 PRIVATE \xE2\x80\xA2 COMPLETE";

        m_paymentLog.push_back(pmt);
        if (m_paymentLog.size() > MAX_PAYMENT_LOG)
            m_paymentLog.erase(m_paymentLog.begin());
    }

    std::string _gen_payment_id() {
        std::uniform_int_distribution<uint32_t> dist(0x10000, 0xFFFFF);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "PMT-%05X", dist(m_rng));
        return std::string(buf);
    }

    uint64_t _now_ns() {
        return static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
    }

    //--------------------------------------------------------------------------
    // Member state
    //--------------------------------------------------------------------------
    ReserveMetrics            m_metrics;
    std::vector<Domain>       m_domains;
    std::vector<FloatGenNode> m_nodes;
    std::vector<FloatGenPath> m_paths;
    std::vector<AtomicPayment> m_paymentLog;
    std::mutex                m_mutex;
    std::mt19937              m_rng;
    double                    m_simTime = 0.0;
};

} // namespace OmniBrain
