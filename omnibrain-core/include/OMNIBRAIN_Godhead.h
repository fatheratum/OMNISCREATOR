#pragma once
/*
 * OMNIBRAIN GODHEAD — C++ Header (Native Bridge)
 * This header declares the interface between the visual Godhead (DX12/Vulkan)
 * and the Python economic core (ReserveNexus + Syndicate).
 *
 * Current implementation: Calls into Python via pybind11 / subprocess / shared memory.
 * Future: Full native C++ port of the solvency + matrix engine.
 */

#include <string>
#include <vector>
#include <cstdint>

namespace omnibrain {
namespace godhead {

struct SolvencyReport {
    double solvency_index;
    double reserves;
    double liquid_assets;
    double outstanding_obligations;
    double float_volume;
};

class ReserveNexusBridge {
public:
    virtual ~ReserveNexusBridge() = default;

    virtual bool acquire_deposit(const std::string& organism_id, double value) = 0;
    virtual bool tender_for_solvency(double amount, const std::string& target = "") = 0;
    virtual SolvencyReport get_status() const = 0;
};

class SyndicateBridge {
public:
    virtual ~SyndicateBridge() = default;

    virtual bool initiate_organism(const std::string& organism_id, double initial_contribution) = 0;
    virtual bool circulate_collective(const std::string& path, double value) = 0;
    virtual double distribute_yield() = 0;
    virtual void enforce_governance() = 0;
};

// Factory functions (implemented in bridge/ via pybind11 or IPC)
ReserveNexusBridge* create_reserve_nexus_bridge();
SyndicateBridge*    create_syndicate_bridge();

} // namespace godhead
} // namespace omnibrain
