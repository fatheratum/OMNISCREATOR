#!/usr/bin/env python3
"""
OMNIBRAIN GODHEAD — Local Runner (inside OMNISCREATOR/omnibrain-core/)
"""

import sys
from pathlib import Path

# Ensure we can import from python/
sys.path.insert(0, str(Path(__file__).parent / "python"))

from reserve_nexus import ReserveNexus
from syndicate import OMNIBRAINSyndicate, EconomicOrganism

def main():
    print("╔════════════════════════════════════════════════════════════╗")
    print("║          OMNIBRAIN GODHEAD — ECONOMIC CORE ACTIVATED       ║")
    print("╚════════════════════════════════════════════════════════════╝\n")

    nexus = ReserveNexus()
    nexus.acquire_deposit("naru_atum", 2_500_000.0)
    nexus.acquire_deposit("lumen", 1_800_000.0)

    syndicate = OMNIBRAINSyndicate(nexus=nexus)

    alpha = EconomicOrganism("naru_atum", "wallet_naru", inventory=1_200_000.0)
    beta  = EconomicOrganism("lumen", "wallet_lumen", inventory=950_000.0)

    syndicate.initiate_organism(alpha, initial_contribution=800_000.0)
    syndicate.initiate_organism(beta, initial_contribution=600_000.0)

    syndicate.circulate_collective("value_to_services", 420_000.0)
    syndicate.distribute_yield()
    syndicate.enforce_governance()

    print("\n[NEXUS STATUS]")
    print(nexus.status())

    print("\n[SYNDICATE STATUS]")
    print(syndicate.syndicate_status())

    print("\n✓ OMNIBRAIN GODHEAD economic core is live and unencumbered.")

if __name__ == "__main__":
    main()
