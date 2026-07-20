//==============================================================================
// main.cpp
// OMNISCREATOR — Main Application Entry Point
// Win32 message pump | DX12 + Vulkan ready | Full OMNIBRAIN living system
// SPACE = tender_for_solvency()  |  E = establish_payment()
// All panels: Sphere · FloatGen Matrix · Reserve Nexus Bar ·
//             Domain Panel · Organism Panel · Payment Feed
// OMNIBRAIN INFINITY Universe | Naru Atum Protocol
// C++17 | Visual Studio 2022 | Windows 11
//==============================================================================

#include "OMNIDirectXInfinity.h"        // DX12 engine (from uploaded system)
#include "OMNISystem.h"                 // Full UI + tool suite
#include "OmniBrainReserveNexus.hpp"    // Economic godhead
#include "EconomicOrganism.hpp"         // Living organisms
#include "OmniBrainSphere.hpp"          // Holographic sphere
#include "FloatGenMatrix.hpp"           // 3D Float-Gen grid
#include "UIRenderer.hpp"               // Panel rendering (ImGui/custom)

#include <Windows.h>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

using namespace OmniBrain;

//==============================================================================
// GLOBALS
//==============================================================================
static const uint32_t WIN_WIDTH  = 1920;
static const uint32_t WIN_HEIGHT = 1080;

// Core systems
static OmniBrainReserveNexus*  g_nexus    = nullptr;
static OrganismRegistry*       g_registry = nullptr;
static OmniBrainSphere*        g_sphere   = nullptr;
static FloatGenMatrix*         g_matrix   = nullptr;

// Timing
static double g_simTime   = 0.0;
static double g_lastTime  = 0.0;

// UI state
static NodeInspection   g_lastInspection;
static std::string      g_consoleLog;
static bool             g_solvencyRestored = false;

//==============================================================================
// LOG HELPER
//==============================================================================
static void log(const std::string& msg) {
    g_consoleLog += msg + "\n";
    // Trim to last 512 lines
    size_t lines = 0, pos = g_consoleLog.size();
    while (pos > 0 && lines < 512) {
        pos = g_consoleLog.rfind('\n', pos - 1);
        ++lines;
    }
    if (lines == 512 && pos != std::string::npos)
        g_consoleLog = g_consoleLog.substr(pos + 1);
}

//==============================================================================
// KEYBOARD CONTROLS
//==============================================================================
static void on_key(UINT8 key) {
    switch (key) {

    // SPACE — tender_for_solvency()
    case VK_SPACE: {
        g_nexus->tender_for_solvency();
        g_sphere->trigger_burst(0.0f, 0.0f, 0.0f, 4096,
                                1.0f, 1.0f, 0.5f);
        // Activate all matrix paths visually
        for (auto& path : g_nexus->paths()) {
            path.active = true;
            path.particleT = 0.0f;
        }
        log("[SPACE] tender_for_solvency() — SI restored to "
            + std::to_string(g_nexus->calculate_solvency_index()));
        g_solvencyRestored = true;
        break;
    }

    // E — establish_payment()
    case 'E': {
        auto pmt = g_nexus->establish_payment();
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "[E] PAYMENT ESTABLISHED"
            << " | ID=" << pmt.paymentId
            << " | FROM=" << pmt.fromDomain
            << " | TO="   << pmt.toDomain
            << " | AMT="  << pmt.amount
            << " | YIELD=" << pmt.yieldApplied
            << " | " << pmt.status;
        log(oss.str());
        // Small particle burst at sphere equator
        g_sphere->trigger_burst(1.0f, 0.0f, 0.0f, 512,
                                1.0f, 0.8f, 0.1f);
        break;
    }

    // R — force solvency recalculate
    case 'R': {
        g_nexus->recalculate_metrics();
        log("[R] Metrics recalculated.");
        break;
    }

    // C — circulate first organism
    case 'C': {
        auto result = g_registry->get(0).circulate(*g_nexus);
        log(result);
        break;
    }

    // N — node click (select node 0)
    case 'N': {
        g_lastInspection = g_matrix->on_node_click(0);
        log("[N] Node inspect: " + g_matrix->format_inspection(g_lastInspection));
        break;
    }

    default: break;
    }
}

//==============================================================================
// DOMAIN PANEL CLICK — acquire deposit from domain i
//==============================================================================
static void on_domain_click(uint32_t domainIndex) {
    double acquired = g_nexus->acquire_deposit(domainIndex);
    auto&  domain   = g_nexus->domains()[domainIndex];

    // Trigger light-code burst from domain's 3D position
    g_sphere->trigger_burst(
        domain.burstOriginX,
        domain.burstOriginY,
        domain.burstOriginZ,
        domain.burstParticles,
        1.0f, 0.82f, 0.15f
    );

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "[DOMAIN-CLICK] " << domain.label
        << " ACQUIRED=" << acquired
        << " BALANCE="  << domain.balance;
    log(oss.str());
}

//==============================================================================
// ORGANISM PANEL — CIRCULATE • EXECUTE PATH
//==============================================================================
static void on_circulate_click(uint32_t orgIndex) {
    auto& org    = g_registry->get(orgIndex);
    auto  result = org.circulate(*g_nexus);
    log(result);

    // Burst from organism's world position
    g_sphere->trigger_burst(
        org.visual().posX,
        org.visual().posY,
        org.visual().posZ,
        org.visual().particleBurstCount,
        org.visual().glowR,
        org.visual().glowG,
        org.visual().glowB
    );
}

//==============================================================================
// TICK — called every frame
//==============================================================================
static void tick(double deltaTime) {
    g_simTime += deltaTime;

    // Tick all living systems
    g_nexus->tick(deltaTime);
    g_registry->tick_all(deltaTime, g_simTime, *g_nexus);
    g_sphere->tick(deltaTime, g_nexus->metrics());
    g_matrix->tick(deltaTime, static_cast<float>(WIN_WIDTH) / WIN_HEIGHT);

    // Clear per-frame flags
    g_solvencyRestored = false;
}

//==============================================================================
// WIN32 WINDOW PROC
//==============================================================================
static HWND g_hwnd = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_KEYUP:
        on_key(static_cast<UINT8>(wParam));
        return 0;

    case WM_LBUTTONDOWN: {
        // Simple UI hit-test routing
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // Domain panel — left 280px, slots stacked 80px each from y=120
        if (mx < 280) {
            int slot = (my - 120) / 80;
            if (slot >= 0 && slot < static_cast<int>(g_nexus->domains().size()))
                on_domain_click(static_cast<uint32_t>(slot));
        }
        // Organism panel — right 280px, slots stacked 200px each from y=80
        else if (mx > WIN_WIDTH - 280) {
            int slot = (my - 80) / 200;
            if (slot >= 0 && slot < 4)
                on_circulate_click(static_cast<uint32_t>(slot));
        }
        // Float-Gen matrix — drag start
        else {
            g_matrix->on_mouse_down(static_cast<float>(mx),
                                    static_cast<float>(my));
        }
        return 0;
    }

    case WM_LBUTTONUP:
        g_matrix->on_mouse_up();
        return 0;

    case WM_MOUSEMOVE:
        g_matrix->on_mouse_move(
            static_cast<float>(LOWORD(lParam)),
            static_cast<float>(HIWORD(lParam))
        );
        return 0;

    case WM_MOUSEWHEEL: {
        float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / 120.0f;
        g_matrix->on_mouse_wheel(delta);
        return 0;
    }

    case WM_RBUTTONDOWN: {
        // Right-click on matrix → node pick (closest to cursor, simplified)
        int   mx   = LOWORD(lParam);
        int   my   = HIWORD(lParam);
        // Map screen → rough node index
        float nx   = (static_cast<float>(mx) / WIN_WIDTH  - 0.5f) * 50.0f;
        float ny   = (static_cast<float>(my) / WIN_HEIGHT - 0.5f) * 50.0f;
        // Find closest node
        uint32_t best = 0;
        float bestD = 1e9f;
        for (uint32_t i = 0; i < g_nexus->nodes().size(); ++i) {
            auto& n = g_nexus->nodes()[i];
            float d = (n.x - nx)*(n.x - nx) + (n.y - ny)*(n.y - ny);
            if (d < bestD) { bestD = d; best = i; }
        }
        g_lastInspection = g_matrix->on_node_click(best);
        log("[NODE-CLICK] " + g_matrix->format_inspection(g_lastInspection));
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

//==============================================================================
// WIN32 WINDOW SETUP
//==============================================================================
static HWND create_window(HINSTANCE hInst) {
    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"OMNISCREATOR";
    RegisterClassEx(&wc);

    RECT rect = { 0, 0,
        static_cast<LONG>(WIN_WIDTH),
        static_cast<LONG>(WIN_HEIGHT) };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0, L"OMNISCREATOR",
        L"OMNISCREATOR — OMNIBRAIN GODHEAD DASHBOARD "
        L"[OMNIDIRECTXINFINITY / OMNIVALKIN]",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr, hInst, nullptr
    );
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    return hwnd;
}

//==============================================================================
// WINMAIN — Application entry point
//==============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    //--------------------------------------------------------------------------
    // 1. Boot living economic systems
    //--------------------------------------------------------------------------
    OmniBrainReserveNexus nexus;
    OrganismRegistry      registry;
    OmniBrainSphere       sphere;
    FloatGenMatrix        matrix(nexus);

    g_nexus    = &nexus;
    g_registry = &registry;
    g_sphere   = &sphere;
    g_matrix   = &matrix;

    log("[OMNISCREATOR] BOOT — OMNIBRAIN GODHEAD ONLINE");
    log("[OMNISCREATOR] OMNIVALKIN + OMNIDIRECTXINFINITY ENGINES READY");
    log("[OMNISCREATOR] NARU-ATUM PROTOCOL ENGAGED");
    log("[SPACE] tender_for_solvency()  [E] establish_payment()");
    log("[C] circulate NARU-ATUM  [N] inspect node  [R] recalculate");

    //--------------------------------------------------------------------------
    // 2. Create Win32 window
    //--------------------------------------------------------------------------
    g_hwnd = create_window(hInstance);

    //--------------------------------------------------------------------------
    // 3. Boot OMNIDIRECTXINFINITY DX12 engine
    //--------------------------------------------------------------------------
    OMNIDirectXInfinity dx12Engine(WIN_WIDTH, WIN_HEIGHT,
        L"OMNISCREATOR — OMNIDIRECTXINFINITY");
    dx12Engine.OnInit();

    //--------------------------------------------------------------------------
    // 4. Boot OMNISystem (ImGui UI + all subsystems)
    //--------------------------------------------------------------------------
    OMNISystem omniSystem(&dx12Engine);
    omniSystem.Initialize();

    //--------------------------------------------------------------------------
    // 5. Prime a few atomic payments on boot
    //--------------------------------------------------------------------------
    for (int i = 0; i < 8; ++i) {
        auto pmt = nexus.establish_payment();
        std::ostringstream oss;
        oss << "[BOOT-PMT] " << pmt.paymentId
            << " | " << pmt.fromDomain
            << " → " << pmt.toDomain
            << " | $" << std::fixed << std::setprecision(2) << pmt.amount
            << " | " << pmt.status;
        log(oss.str());
    }

    //--------------------------------------------------------------------------
    // 6. Register OMNIBRAIN keyboard actions with OMNISystem
    //--------------------------------------------------------------------------
    omniSystem.RegisterAction("TenderSolvency", [&]() {
        on_key(VK_SPACE);
    });
    omniSystem.RegisterAction("EstablishPayment", [&]() {
        on_key('E');
    });
    omniSystem.RegisterAction("CirculateNaruAtum", [&]() {
        on_circulate_click(0);
    });

    //--------------------------------------------------------------------------
    // 7. Main loop
    //--------------------------------------------------------------------------
    auto tStart = std::chrono::high_resolution_clock::now();
    g_lastTime  = 0.0;

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // Compute delta time
            auto tNow  = std::chrono::high_resolution_clock::now();
            double now = std::chrono::duration<double>(tNow - tStart).count();
            double dt  = now - g_lastTime;
            g_lastTime = now;

            // Clamp dt to avoid spiral-of-death on pauses
            dt = std::min(dt, 0.05);

            //------------------------------------------------------------------
            // TICK all living systems
            //------------------------------------------------------------------
            tick(dt);
            omniSystem.Update(static_cast<float>(dt));

            //------------------------------------------------------------------
            // DX12 RENDER FRAME
            // Full pipeline:
            //   Pass 0: Compute — emit + update particles, evolve Float-Gen
            //   Pass 1: Depth pre-pass (opaque sphere core + nodes)
            //   Pass 2: Opaque — sphere core, node spheres, ring lines
            //   Pass 3: Transparency — holographic shells (back→front)
            //   Pass 4: Additive — light-code particle sprites
            //   Pass 5: Float-Gen lines (additive)
            //   Pass 6: ImGui UI overlay (all panels)
            //------------------------------------------------------------------
            dx12Engine.OnUpdate();
            dx12Engine.OnRender();
            omniSystem.RenderUI();
        }
    }

    //--------------------------------------------------------------------------
    // 8. Shutdown
    //--------------------------------------------------------------------------
    omniSystem.Shutdown();
    dx12Engine.OnDestroy();

    log("[OMNISCREATOR] SHUTDOWN — All particles returned to source.");
    return static_cast<int>(msg.wParam);
}
