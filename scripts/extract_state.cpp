//==============================================================================
// extract_state.cpp
// Reads OmniBrainReserveNexus.hpp + EconomicOrganism.hpp
// Outputs omnibrain_state.json → fed to godhead-dashboard
// Run by GitHub Actions after engine compile
// C++17 | standalone | no dependencies
//==============================================================================

#include "../omnibrain-core/OmniBrainReserveNexus.hpp"
#include "../omnibrain-core/EconomicOrganism.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace OmniBrain;

int main() {
    // Boot living systems
    OmniBrainReserveNexus nexus;
    OrganismRegistry      registry;

    // Tick once to generate initial live state
    nexus.tick(0.016);
    registry.tick_all(0.016, 0.016, nexus);

    // Establish 4 boot payments
    for (int i = 0; i < 4; ++i) nexus.establish_payment();

    const auto& M = nexus.metrics();

    std::ostringstream json;
    json << std::fixed << std::setprecision(4);
    json << "{\n";
    json << "  \"godhead\": \"OMNIBRAIN RESERVE NEXUS\",\n";
    json << "  \"universe\": \"INFINITY / Naru Atum\",\n";
    json << "  \"version\": \"1.0.0\",\n";

    // Reserve metrics
    json << "  \"reserve\": {\n";
    json << "    \"solvencyIndex\": "   << M.solvencyIndex.load()           << ",\n";
    json << "    \"liquidAssets\": "    << M.liquidAssets.load()            << ",\n";
    json << "    \"yieldPool\": "       << M.yieldPool.load()               << ",\n";
    json << "    \"floatVolume\": "     << M.floatVolume.load()             << ",\n";
    json << "    \"organismsOnline\": " << M.organismsOnline.load()         << ",\n";
    json << "    \"reserveTotal\": "    << M.reserveTotal.load()            << ",\n";
    json << "    \"yieldRate\": "       << M.yieldRate.load()               << "\n";
    json << "  },\n";

    // Domains
    json << "  \"domains\": [\n";
    const auto& domains = nexus.domains();
    for (size_t i = 0; i < domains.size(); ++i) {
        const auto& d = domains[i];
        json << "    { \"id\": \"" << d.id << "\","
             << " \"label\": \""   << d.label << "\","
             << " \"balance\": "   << d.balance << ","
             << " \"yield\": "     << d.pendingYield << " }";
        if (i < domains.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";

    // Organisms
    json << "  \"organisms\": [\n";
    const auto& orgs = registry.all();
    for (size_t i = 0; i < orgs.size(); ++i) {
        const auto& o = orgs[i];
        json << "    { \"id\": "        << o.id()
             << ", \"name\": \""        << o.name()      << "\""
             << ", \"sigil\": \""       << o.sigil()     << "\""
             << ", \"capital\": "       << o.capital()
             << ", \"yieldRate\": "     << o.yieldRate()
             << ", \"state\": \""       << o.stateLabel() << "\""
             << ", \"circulations\": "  << o.circulateCount() << " }";
        if (i < orgs.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";

    // Payment log (last 8)
    json << "  \"payments\": [\n";
    const auto& log = nexus.paymentLog();
    size_t start = log.size() > 8 ? log.size() - 8 : 0;
    for (size_t i = start; i < log.size(); ++i) {
        const auto& p = log[i];
        json << "    { \"id\": \""     << p.paymentId   << "\""
             << ", \"from\": \""       << p.fromDomain  << "\""
             << ", \"to\": \""         << p.toDomain    << "\""
             << ", \"amount\": "       << p.amount
             << ", \"yield\": "        << p.yieldApplied
             << ", \"status\": \""     << p.status      << "\" }";
        if (i < log.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";

    // Config
    json << "  \"config\": {\n";
    json << "    \"maxParticles\": 8192,\n";
    json << "    \"lightCodeIntensity\": 1.0,\n";
    json << "    \"sacredRings\": 9,\n";
    json << "    \"holographicLayers\": 5,\n";
    json << "    \"autoEvolve\": true\n";
    json << "  }\n";
    json << "}\n";

    // Write to godhead-dashboard/
    std::ofstream out("godhead-dashboard/omnibrain_state.json");
    if (!out) {
        std::cerr << "ERROR: cannot write omnibrain_state.json\n";
        return 1;
    }
    out << json.str();
    out.close();

    std::cout << "omnibrain_state.json written — GODHEAD ONLINE\n";
    std::cout << "SI=" << M.solvencyIndex.load()
              << " LA=" << M.liquidAssets.load()
              << " FV=" << M.floatVolume.load() << "\n";
    return 0;
}
