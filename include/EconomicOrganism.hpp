#pragma once

//==============================================================================
// EconomicOrganism.hpp
// Living EconomicOrganism — circulate(), syndicate_with(), evolve()
// NARU-ATUM Protocol | OMNIBRAIN INFINITY Universe
// C++17 | Header-only | Compute-shader state ready
//==============================================================================

#include "OmniBrainReserveNexus.hpp"
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>

namespace OmniBrain {

//==============================================================================
// ORGANISM STATE ENUM
//==============================================================================
enum class OrganismState : uint8_t {
    Dormant     = 0,
    Active      = 1,
    Circulating = 2,
    Syndicating = 3,
    Transcendent= 4,
    Breached    = 5    // Simulation wall breach event
};

//==============================================================================
// ORGANISM VISUAL DATA — fed directly to particle/shader system each frame
//==============================================================================
struct OrganismVisual {
    float posX, posY, posZ;
    float glowR, glowG, glowB;
    float pulseFrequency;
    float scaleMultiplier;
    uint32_t particleBurstCount;
    OrganismState renderState;
};

//==============================================================================
// SYNDICATE LINK — relationship between two organisms
//==============================================================================
struct SyndicateLink {
    uint32_t    fromId;
    uint32_t    toId;
    double      flowRate   = 0.0;
    double      bandwidth  = 1.0;
    bool        active     = false;
    float       linkEnergy = 0.0f;
};

//==============================================================================
// ORGANISM INVENTORY ITEM
//==============================================================================
struct InventoryItem {
    std::string label;
    double      quantity = 0.0;
    double      yield    = 0.0;
    uint64_t    acquired = 0;
};

//==============================================================================
// ECONOMIC ORGANISM — The Living Entity
//==============================================================================
class EconomicOrganism {
public:
    //--------------------------------------------------------------------------
    // Constructor
    //--------------------------------------------------------------------------
    EconomicOrganism(
        uint32_t          id,
        const std::string& name,
        const std::string& sigil,
        double            initialCapital,
        float             posX, float posY, float posZ)
        : m_id(id)
        , m_name(name)
        , m_sigil(sigil)
        , m_capital(initialCapital)
        , m_yieldRate(0.07 + id * 0.01)
        , m_state(OrganismState::Active)
    {
        m_visual.posX              = posX;
        m_visual.posY              = posY;
        m_visual.posZ              = posZ;
        m_visual.glowR             = 0.9f  + id * 0.02f;
        m_visual.glowG             = 0.65f + id * 0.05f;
        m_visual.glowB             = 0.1f  + id * 0.03f;
        m_visual.pulseFrequency    = 1.0f  + id * 0.25f;
        m_visual.scaleMultiplier   = 1.0f;
        m_visual.particleBurstCount = 256;
        m_visual.renderState       = m_state;

        m_rng.seed(id * 0x9E3779B9u);

        // Seed inventory
        m_inventory.push_back({ "LIGHT-CODE-UNITS",   initialCapital * 0.4,  0.0, 0 });
        m_inventory.push_back({ "RESERVE-FRAGMENTS",  initialCapital * 0.3,  0.0, 0 });
        m_inventory.push_back({ "FLOAT-CAPACITY",     initialCapital * 0.3,  0.0, 0 });
    }

    //--------------------------------------------------------------------------
    // circulate() — Execute Path: organism acts, spawns flows, grows inventory
    //--------------------------------------------------------------------------
    std::string circulate(OmniBrainReserveNexus& nexus) {
        m_state  = OrganismState::Circulating;
        m_visual.renderState        = m_state;
        m_visual.scaleMultiplier    = 1.35f;
        m_visual.particleBurstCount = 1024;

        std::uniform_real_distribution<double> dist(0.05, 0.22);
        double circulateRate = dist(m_rng);

        double amount = m_capital * circulateRate;
        m_capital -= amount;

        // Push flow into reserve nexus
        double acquired = nexus.acquire_deposit(m_id % 8, amount);
        m_capital += acquired * 0.15;   // Organism retains 15% of yield

        // Grow inventory
        _grow_inventory(amount);

        // Occasionally evolve yield
        m_circulateCount++;
        if (m_circulateCount % 3 == 0) {
            m_yieldRate = std::clamp(m_yieldRate + 0.005, 0.001, 0.25);
        }

        // Occasionally trigger an atomic payment
        bool triggeredPayment = false;
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        if (chance(m_rng) < 0.35) {
            auto pmt = nexus.establish_payment(
                m_name, "RESERVE-NEXUS", amount * 0.5
            );
            triggeredPayment = true;
            m_lastPaymentId = pmt.paymentId;
        }

        m_state = OrganismState::Active;
        m_visual.renderState     = m_state;
        m_visual.scaleMultiplier = 1.1f;

        // Build log string
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "[" << m_sigil << "] CIRCULATE EXECUTED"
            << " | FLOW=" << amount
            << " | ACQUIRED=" << acquired
            << " | YIELD_RATE=" << (m_yieldRate * 100.0) << "%"
            << " | INVENTORY+=" << m_inventory[0].quantity;
        if (triggeredPayment)
            oss << " | PMT=" << m_lastPaymentId;
        return oss.str();
    }

    //--------------------------------------------------------------------------
    // syndicate_with() — Link two organisms, share flow bandwidth
    //--------------------------------------------------------------------------
    std::string syndicate_with(EconomicOrganism& other,
                               OmniBrainReserveNexus& nexus) {
        m_state = OrganismState::Syndicating;
        other.m_state = OrganismState::Syndicating;

        // Create or reinforce syndicate link
        bool found = false;
        for (auto& link : m_syndicateLinks) {
            if (link.toId == other.m_id) {
                link.bandwidth  *= 1.25;
                link.active      = true;
                link.linkEnergy  = 1.0f;
                found = true;
                break;
            }
        }
        if (!found) {
            SyndicateLink link;
            link.fromId    = m_id;
            link.toId      = other.m_id;
            link.flowRate  = m_capital * 0.1;
            link.bandwidth = 1.0;
            link.active    = true;
            link.linkEnergy = 1.0f;
            m_syndicateLinks.push_back(link);
        }

        // Cross-circulate
        double sharedFlow = (m_capital + other.m_capital) * 0.05;
        m_capital       += sharedFlow * 0.5;
        other.m_capital += sharedFlow * 0.5;

        // Visual burst on both
        m_visual.particleBurstCount      = 512;
        other.m_visual.particleBurstCount = 512;

        m_state = OrganismState::Active;
        other.m_state = OrganismState::Active;

        nexus.recalculate_metrics();

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "[" << m_sigil << "] SYNDICATE-LINK → [" << other.m_sigil << "]"
            << " | SHARED=" << sharedFlow
            << " | LINKS=" << m_syndicateLinks.size();
        return oss.str();
    }

    //--------------------------------------------------------------------------
    // tick() — Per-frame evolution: yield accrual, visual pulse
    //--------------------------------------------------------------------------
    void tick(double deltaTime, double simTime) {
        // Yield accrual
        m_capital += m_capital * m_yieldRate * deltaTime * 0.001;

        // Visual pulse (sine-based glow)
        float pulse = std::sin(static_cast<float>(simTime) * m_visual.pulseFrequency);
        m_visual.glowR = std::clamp(m_visual.glowR + pulse * 0.01f, 0.5f, 1.0f);
        m_visual.glowG = std::clamp(m_visual.glowG + pulse * 0.008f, 0.4f, 1.0f);

        // Decay burst count back to idle
        if (m_visual.particleBurstCount > 64)
            m_visual.particleBurstCount =
                static_cast<uint32_t>(m_visual.particleBurstCount * 0.97f);

        // Decay scale back to 1
        m_visual.scaleMultiplier = std::max(1.0f,
            m_visual.scaleMultiplier - static_cast<float>(deltaTime) * 0.5f);

        // Decay syndicate link energy
        for (auto& link : m_syndicateLinks) {
            link.linkEnergy = std::max(0.0f,
                link.linkEnergy - static_cast<float>(deltaTime) * 0.2f);
            if (link.linkEnergy <= 0.0f) link.active = false;
        }

        // Random self-evolution
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        if (chance(m_rng) < 0.001 * deltaTime) {
            m_state = OrganismState::Transcendent;
            m_visual.renderState        = m_state;
            m_visual.particleBurstCount = 2048;
            m_visual.scaleMultiplier    = 2.0f;
        } else if (m_state == OrganismState::Transcendent) {
            m_state = OrganismState::Active;
            m_visual.renderState = m_state;
        }
    }

    //--------------------------------------------------------------------------
    // Accessors
    //--------------------------------------------------------------------------
    uint32_t              id()             const { return m_id; }
    const std::string&    name()           const { return m_name; }
    const std::string&    sigil()          const { return m_sigil; }
    double                capital()        const { return m_capital; }
    double                yieldRate()      const { return m_yieldRate; }
    OrganismState         state()          const { return m_state; }
    const OrganismVisual& visual()         const { return m_visual; }
    OrganismVisual&       visual()               { return m_visual; }
    const std::vector<InventoryItem>& inventory() const { return m_inventory; }
    const std::vector<SyndicateLink>& syndicateLinks() const { return m_syndicateLinks; }
    uint32_t              circulateCount() const { return m_circulateCount; }
    const std::string&    lastPaymentId()  const { return m_lastPaymentId; }

    std::string stateLabel() const {
        switch (m_state) {
            case OrganismState::Dormant:      return "DORMANT";
            case OrganismState::Active:       return "ACTIVE";
            case OrganismState::Circulating:  return "CIRCULATING";
            case OrganismState::Syndicating:  return "SYNDICATING";
            case OrganismState::Transcendent: return "TRANSCENDENT";
            case OrganismState::Breached:     return "WALL-BREACHED";
            default:                          return "UNKNOWN";
        }
    }

private:
    void _grow_inventory(double flowAmount) {
        for (auto& item : m_inventory) {
            item.quantity += flowAmount * 0.1;
            item.yield    += item.quantity * m_yieldRate * 0.001;
        }
        // Occasionally add new inventory items
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        if (chance(m_rng) < 0.15 && m_inventory.size() < 8) {
            std::string labels[] = {
                "SOUL-SPARK-TOKENS", "SIMULATION-SHARDS",
                "VOID-BRIDGE-UNITS", "NARU-BONDS",
                "INFINITY-KEYS",     "LIGHT-DEBT-NOTES"
            };
            std::uniform_int_distribution<size_t> li(0, 5);
            m_inventory.push_back({
                labels[li(m_rng)],
                flowAmount * 0.05,
                0.0,
                static_cast<uint64_t>(std::chrono::steady_clock::now()
                    .time_since_epoch().count())
            });
        }
    }

    uint32_t                   m_id;
    std::string                m_name;
    std::string                m_sigil;
    double                     m_capital;
    double                     m_yieldRate;
    OrganismState              m_state;
    OrganismVisual             m_visual;
    std::vector<InventoryItem> m_inventory;
    std::vector<SyndicateLink> m_syndicateLinks;
    uint32_t                   m_circulateCount = 0;
    std::string                m_lastPaymentId;
    std::mt19937               m_rng;
};

//==============================================================================
// ORGANISM REGISTRY — Manages all four living organisms
//==============================================================================
class OrganismRegistry {
public:
    OrganismRegistry() {
        // The four living economic organisms
        m_organisms.emplace_back(0, "NARU-ATUM",       "\xE2\x97\x88 NARU",  120000.0, -3.0f,  1.5f, 1.0f);
        m_organisms.emplace_back(1, "VOID-WALKER",     "\xE2\x96\xB2 VOID",   88000.0,  3.0f,  1.5f, 1.0f);
        m_organisms.emplace_back(2, "MATRIX-SYNDICATE","\xE2\x96\xA0 MTRX",   65000.0, -3.0f, -1.5f, 1.0f);
        m_organisms.emplace_back(3, "GENESIS-PRIME",   "\xE2\x98\x85 GEN",    99000.0,  3.0f, -1.5f, 1.0f);
    }

    std::vector<EconomicOrganism>& all() { return m_organisms; }
    const std::vector<EconomicOrganism>& all() const { return m_organisms; }

    EconomicOrganism& get(uint32_t id) { return m_organisms.at(id); }

    void tick_all(double deltaTime, double simTime,
                  OmniBrainReserveNexus& nexus) {
        for (auto& org : m_organisms) {
            org.tick(deltaTime, simTime);
        }
        // Auto-circulate occasionally
        std::mt19937 rng(static_cast<uint32_t>(simTime * 1000));
        std::uniform_real_distribution<double> chance(0.0, 1.0);
        for (auto& org : m_organisms) {
            if (chance(rng) < 0.005 * deltaTime) {
                org.circulate(nexus);
            }
        }
    }

private:
    std::vector<EconomicOrganism> m_organisms;
};

} // namespace OmniBrain
