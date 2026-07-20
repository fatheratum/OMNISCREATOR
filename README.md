# OMNISCREATOR

**The complete monorepo for the OMNIBRAIN GODHEAD visual + economic engine.**

## Directory Structure

```
OMNISCREATOR/
├── .github/workflows/           # CI/CD for DX12, Vulkan, Godhead Dashboard, Pages
├── omnibrain-core/              # ← Economic Godhead (Python core + C++ / JS bridge)
│   ├── python/                  # ReserveNexus + Syndicate (authoritative)
│   ├── include/                 # C++ headers for native bridge
│   ├── bridge/                  # JS + pybind11 bridge to dashboard
│   └── run_godhead.py           # Local runner
├── omnivalkin/                  # Vulkan renderer (shaders + src)
├── omnidirectxinfinity/         # DirectX 12 renderer (HLSL + nv_helpers)
├── godhead-dashboard/           # Living OMNISURF interface (WebGPU / TUI)
├── studios/
│   └── naru-atum/               # Character, cinematic, INFINITY narrative assets
├── docs/
├── scripts/
└── config/
```

## Quick Start (Economic Core)

```bash
cd OMNISCREATOR/omnibrain-core
python3 run_godhead.py
```

This runs the refined `ReserveNexus` + `OMNIBRAINSyndicate` you provided.

## Relationship to Graphics Layers

- `omnidirectxinfinity/` + `omnivalkin/` = Visual Godhead (the "body")
- `omnibrain-core/` = Economic + Semantic intelligence (the "soul" / living reserves)
- `godhead-dashboard/` = The ritual interface that visualizes both

The C++ header `OMNIBRAIN_Godhead.h` + JS bridge define how the visual layer will eventually call into the economic core in real time.

## Current Status

- Python economic engine: **Complete & tested** (using your latest `reserve_nexus-2.py` + `syndicate-1.py`)
- C++ / native bridge: **Header + stub defined**
- Graphics repos: **Structure ready** (real shaders & src live in your GitHub Actions)

**OMNIBRAIN • RESERVES • CIRCULATION • COMPLETE**
