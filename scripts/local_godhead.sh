#!/bin/bash
# OMNISCREATOR — One-command Godhead activation

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║           OMNISCREATOR — GODHEAD LOCAL ACTIVATION              ║"
echo "╚════════════════════════════════════════════════════════════════╝"

cd "$(dirname "$0")/.."

echo ""
echo "[1] Running OMNIBRAIN Economic Core..."
cd omnibrain-core
python3 run_godhead.py

echo ""
echo "[2] (Optional) Launch Godhead Dashboard..."
# cd ../godhead-dashboard && npm run dev   # when implemented

echo ""
echo "✓ OMNISCREATOR Godhead is live."
echo "  Economic core + Visual layer ready for integration."
