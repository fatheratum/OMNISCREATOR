# OMNISCREATOR — BUILD INSTRUCTIONS

## Prerequisites

| Requirement | Version | Notes |
|---|---|---|
| Windows | 10/11 (1903+) | DXR requires 1809+ |
| Visual Studio | 2022 (v143) | Desktop C++ workload |
| Windows SDK | 10.0.22621+ | Included with VS2022 |
| CMake | 3.20+ | cmake.org |
| GPU | RTX 20xx+ / RDNA2+ / Arc | DXR Tier 1.0 minimum |
| Vulkan SDK | 1.3+ (optional) | vulkan.lunarg.com |
| Dear ImGui | latest | github.com/ocornut/imgui |

---

## Step 1 — Get Dear ImGui

```powershell
# In the parent directory of OMNISCREATOR
git clone https://github.com/ocornut/imgui.git
# Result: ../imgui (relative to OMNISCREATOR/)
```

---

## Step 2 — Get NVIDIA DXR Helpers (already included as simplified stubs)

The `include/nv_helpers_dx12/` stubs from the uploaded OMNIDIRECTXINFINITY system
are already present. For full production fidelity, replace with NVIDIA's originals:

```powershell
# From NVIDIA's DX12 Raytracing tutorial repo
git clone https://github.com/NVIDIAGameWorks/DX12-Raytracing-Tutorials.git
# Copy Common/nv_helpers_dx12/* into OMNISCREATOR/include/nv_helpers_dx12/
```

---

## Step 3 — Configure (DX12 — OMNIDIRECTXINFINITY)

```powershell
cd OMNISCREATOR
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 `
    -DIMGUI_DIR="../imgui"
```

---

## Step 4 — Build

```powershell
cmake --build build --config Release --target OMNIDIRECTXINFINITY -j 8
```

Or open `build/OMNISCREATOR.sln` in Visual Studio 2022 and build from there.

---

## Step 5 — Run

```powershell
.\build\Release\OMNIDIRECTXINFINITY.exe
```

---

## Step 6 — OMNIVALKIN (Vulkan path, optional)

Install Vulkan SDK, then:

```powershell
cmake -B build_vk -S . -G "Visual Studio 17 2022" -A x64 `
    -DIMGUI_DIR="../imgui" `
    -DVulkan_SDK="C:/VulkanSDK/1.3.xxx.0"

cmake --build build_vk --config Release --target OMNIVALKIN -j 8
.\build_vk\Release\OMNIVALKIN.exe
```

---

## Keyboard Controls (at runtime)

| Key | Action |
|---|---|
| `SPACE` | `tender_for_solvency()` — restores SI ≥ 1.0, particle burst |
| `E` | `establish_payment()` — random atomic settlement |
| `C` | Circulate NARU-ATUM organism |
| `N` | Inspect node 0 of Float-Gen matrix |
| `R` | Recalculate reserve metrics |
| `ESC` | Exit |

**Mouse (Float-Gen Matrix):**
- Left-drag → rotate 3D view
- Scroll wheel → zoom
- Right-click → inspect nearest node (force vector + state)

---

## Panel Layout

```
┌──────────────────────────────────────────────────────────────┐
│  TOP: RESERVE NEXUS BAR  SI | LA | YR | FV | OO | RESERVE   │
├────────────┬─────────────────────────────┬───────────────────┤
│   LEFT     │                             │     RIGHT         │
│  PRIVATE   │   CENTRAL 3D CANVAS         │   ORGANISMS       │
│  WALLET    │   • OMNIBRAIN SPHERE        │   • NARU-ATUM     │
│  DOMAINS   │   • FLOAT-GEN MATRIX        │   • VOID-WALKER   │
│  (click)   │   (drag/scroll/r-click)     │   • MATRIX-SYN    │
│            │                             │   • GENESIS-PRIME │
├────────────┴─────────────────────────────┴───────────────────┤
│  BOTTOM: PAYMENT FEED — scrolling atomic settlements          │
├──────────────────────────────────────────────────────────────┤
│  OMNIBRAIN CONSOLE (collapsible, bottom-left overlay)         │
└──────────────────────────────────────────────────────────────┘
```

---

## Shader Compilation

HLSL shaders compile automatically via DXC during the build if the Windows SDK
DXC binary is on PATH. GLSL shaders compile to SPIR-V via `glslc` (Vulkan SDK).

Manual HLSL compile:

```powershell
dxc -T lib_6_5 -Fo shaders/OmniBrainSphere.cso shaders/hlsl/OmniBrainSphere.hlsl
dxc -T cs_6_5 -E CS_EmitParticles -Fo shaders/LightCodeEmit.cso shaders/hlsl/LightCodeCompute.hlsl
dxc -T cs_6_5 -E CS_UpdateParticles -Fo shaders/LightCodeUpdate.cso shaders/hlsl/LightCodeCompute.hlsl
```

Manual GLSL compile:

```powershell
glslc shaders/glsl/omnibrain_sphere.vert -o shaders/omnibrain_sphere.vert.spv
glslc shaders/glsl/omnibrain_sphere.frag -o shaders/omnibrain_sphere.frag.spv
glslc shaders/glsl/lightcode_compute.comp -o shaders/lightcode_compute.comp.spv
```
