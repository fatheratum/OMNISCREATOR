#!/usr/bin/env python3
"""
OMNIBRAIN GODHEAD — Production State Exporter
This is the real version used by the live dashboard.
"""

import sys
import argparse
from pathlib import Path
import json
from datetime import datetime

sys.path.insert(0, str(Path(__file__).parent / "python"))

from reserve_nexus import ReserveNexus
from syndicate import OMNIBRAINSyndicate, EconomicOrganism


def build_state(nexus: ReserveNexus, syndicate: OMNIBRAINSyndicate) -> dict:
    """Build the state dictionary expected by the dashboard."""

    return {
        "godhead": "OMNIBRAIN RESERVE NEXUS",
        "universe": "INFINITY / Naru Atum",
        "version": "1.0.0",
        "timestamp": datetime.now().isoformat(),
        "reserve": {
            "solvencyIndex": round(nexus.get_solvency_index(), 4),
            "liquidAssets": round(nexus.liquid_assets, 2),
            "yieldRate": round(nexus.yield_rate * 100, 3),
            "floatVolume": round(nexus.float_volume, 2),
            "organismsOnline": len(syndicate.organisms),
            "reserveTotal": round(nexus.reserves, 2)
        },
        "organisms": [
            {
                "name": org.organism_id,
                "capital": round(org.inventory, 2),
                "yieldRate": round(org.yield_rate * 100, 2),
                "state": "ACTIVE"
            }
            for org in syndicate.organisms.values()
        ],
        "domains": [
            {"label": k, "balance": v, "pendingYield": round(v * 0.07, 2)}
            for k, v in nexus.organisms_contributions.items()
        ],
        "payments": [],
        "engines": {
            "python_core": {"status": "ONLINE", "source": "omnibrain-core/python/"}
        },
        "build": {
            "timestamp": datetime.now().isoformat(),
            "source": "Python Core"
        }
    }


def export_state(output_path: str = str(Path(__file__).resolve().parent.parent / "godhead-dashboard" / "omnibrain_state.json")):
    """Main function to generate and save omnibrain_state.json."""

    nexus = ReserveNexus()
    nexus.acquire_deposit("naru_atum", 2500000)
    nexus.acquire_deposit("lumen", 1800000)

    syndicate = OMNIBRAINSyndicate(nexus=nexus)

    # Seed organisms
    for name, inv, contrib in [
        ("naru_atum", 1200000, 800000),
        ("void_walker", 880000, 600000),
        ("matrix_syndicate", 650000, 400000),
        ("genesis_prime", 990000, 700000),
    ]:
        org = EconomicOrganism(name, f"wallet_{name}", inventory=inv)
        syndicate.initiate_organism(org, initial_contribution=contrib)

    syndicate.circulate_collective("value_circulation", 420000)
    syndicate.distribute_yield()

    state = build_state(nexus, syndicate)

    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w") as f:
        json.dump(state, f, indent=2)

    print(f"✅ omnibrain_state.json generated → {output_path}")
    return state


def main():
    parser = argparse.ArgumentParser(description="OMNIBRAIN GODHEAD State Exporter")
    parser.add_argument("--export-only", action="store_true", help="Only export JSON (no demo output)")
    args = parser.parse_args()

    if args.export_only:
        export_state()
    else:
        print("╔════════════════════════════════════════════════════════════╗")
        print("║          OMNIBRAIN GODHEAD — PYTHON CORE (PRODUCTION)      ║")
        print("╚════════════════════════════════════════════════════════════╝\n")

        state = export_state()
        print("\n✓ Python Core state ready for live dashboard.")


if __name__ == "__main__":
    main()
