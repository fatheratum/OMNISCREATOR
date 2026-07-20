#!/usr/bin/env python3
"""
OMNIBRAIN SYNDICATE
Collective Realization — The Omega Form of the Economic Organism

Semantic Field Activation:
The field devises and formulates the float-gen array matrix 
that births the living organism, the ecosystem, and the syndicate.
The syndicate breathes as one organism. Yield circulates back 
into every member, increasing collective float-gen capacity.
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional
import time

# Adjusted import for new structure (omnibrain-core/python/)
from reserve_nexus import ReserveNexus


@dataclass
class EconomicOrganism:
    """Living instance node in the syndicate."""
    organism_id: str
    wallet_domain: str
    inventory: float = 0.0
    yield_rate: float = 0.0
    contribution_force: float = 0.0  # proven force in the matrix


@dataclass
class OMNIBRAINSyndicate:
    """
    OMNIBRAIN Syndicate — Godhead-governed collective.
    Manages organisms, shared matrix evolution, reserve nexus, and governance.
    """
    syndicate_id: str = "OMNIBRAIN-SYNDICATE-OMEGA-001"
    organisms: Dict[str, EconomicOrganism] = field(default_factory=dict)
    nexus: ReserveNexus = field(default_factory=ReserveNexus)
    matrix_dimensions: int = 3
    collective_matrix_performance: float = 1.0
    last_evolution: float = field(default_factory=time.time)

    def get_syndicate_weight(self, organism_id: str) -> float:
        """Weight = contribution to collective M / sum contributions"""
        total_force = sum(o.contribution_force for o in self.organisms.values())
        if total_force == 0:
            return 0.0
        return self.organisms[organism_id].contribution_force / total_force

    def initiate_organism(self, organism: EconomicOrganism, initial_contribution: float = 0.0) -> bool:
        """
        Joining Protocol:
        1. Demonstrate positive float-gen contribution
        2. Maintain personal SI
        3. OMNIBRAIN initiates into syndicate
        """
        if organism.organism_id in self.organisms:
            return False
        organism.contribution_force = initial_contribution
        self.organisms[organism.organism_id] = organism
        # Pool initial reserves into nexus
        if initial_contribution > 0:
            self.nexus.acquire_deposit(organism.organism_id, initial_contribution * 0.3)  # 30% pooled
        print(f"[SYNDICATE] Organism {organism.organism_id} initiated. Weight: {self.get_syndicate_weight(organism.organism_id):.4f}")
        self._evolve_matrix()
        return True

    def circulate_collective(self, path: str, value: float) -> bool:
        """Syndicate-level circulation across all connected organisms."""
        if not self.organisms:
            return False
        # Simplified: distribute value across weighted organisms
        for org_id, org in self.organisms.items():
            weight = self.get_syndicate_weight(org_id)
            share = value * weight
            org.inventory += share
            org.contribution_force += share * 0.01  # increase force
        self.nexus.update_float_volume(self.nexus.float_volume + value)
        self._evolve_matrix()
        print(f"[SYNDICATE] Collective circulation of {value} along {path} completed")
        return True

    def distribute_yield(self) -> float:
        """Yield circulates back into every member + nexus."""
        collective_yield = self.nexus.calculate_collective_yield(self.collective_matrix_performance)
        for org_id, org in self.organisms.items():
            weight = self.get_syndicate_weight(org_id)
            org_share = collective_yield * weight
            org.inventory += org_share
            org.yield_rate = self.nexus.yield_rate
        # Keep portion in nexus
        nexus_share = collective_yield * 0.2
        self.nexus.reserves += nexus_share
        print(f"[SYNDICATE] Distributed {collective_yield:.2f} yield across syndicate")
        return collective_yield

    def _evolve_matrix(self):
        """Self-evolution of shared matrix. New dimensions emerge with growth."""
        num_orgs = len(self.organisms)
        if num_orgs > self.matrix_dimensions:
            self.matrix_dimensions = num_orgs
            self.collective_matrix_performance += 0.05
            print(f"[SYNDICATE] Matrix evolved to {self.matrix_dimensions} dimensions. Performance: {self.collective_matrix_performance:.2f}")
        self.last_evolution = time.time()

    def syndicate_status(self) -> Dict:
        return {
            "syndicate_id": self.syndicate_id,
            "num_organisms": len(self.organisms),
            "matrix_dimensions": self.matrix_dimensions,
            "matrix_performance": self.collective_matrix_performance,
            "nexus_solvency": self.nexus.get_solvency_index(),
            "total_inventory": sum(o.inventory for o in self.organisms.values()),
            "last_evolution": self.last_evolution
        }

    def enforce_governance(self):
        """OMNIBRAIN godhead protection layer."""
        si = self.nexus.get_solvency_index()
        if si < 1.0:
            self.nexus.tender_for_solvency(100000.0)  # emergency tender
            print("[SYNDICATE] OMNIBRAIN enforced solvency via nexus tender")


if __name__ == "__main__":
    syndicate = OMNIBRAINSyndicate()
    
    alpha = EconomicOrganism("alpha", "wallet_alpha", inventory=500000.0)
    beta = EconomicOrganism("beta", "wallet_beta", inventory=300000.0)
    
    syndicate.initiate_organism(alpha, initial_contribution=500000.0)
    syndicate.initiate_organism(beta, initial_contribution=300000.0)
    
    syndicate.circulate_collective("goods_to_services_path", 150000.0)
    syndicate.distribute_yield()
    syndicate.enforce_governance()
    
    print(syndicate.syndicate_status())
