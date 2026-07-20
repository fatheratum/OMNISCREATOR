#pragma once

//==============================================================================
// UIRenderer.hpp
// OMNISCREATOR — Full ImGui Panel Renderer
// All six panels, cyberpunk gold theme, atomic live-update
//
// PANELS:
//   1. Top Reserve Nexus Bar    — live SI, LA, YR, FV, OO
//   2. Left Domain Panel        — 8 private wallet domains, click-to-acquire
//   3. Central Sphere Overlay   — sphere state readout over 3D canvas
//   4. Right Organism Panel     — 4 organism cards incl. NARU-ATUM
//   5. Bottom Payment Feed      — scrolling atomic settlement log
//   6. Node Inspector Popup     — force vector, state, flow on node click
//
// Requires: Dear ImGui (imgui.h / imgui_impl_dx12.h / imgui_impl_win32.h)
// C++17 | OMNIBRAIN INFINITY Universe
//==============================================================================

#include "imgui.h"
#include "OmniBrainReserveNexus.hpp"
#include "EconomicOrganism.hpp"
#include "OmniBrainSphere.hpp"
#include "FloatGenMatrix.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <cmath>

namespace OmniBrain {

//==============================================================================
// COLOUR PALETTE (matches cyberpunk ImGui theme)
//==============================================================================
namespace Colors {
    static const ImVec4 Gold        = ImVec4(1.00f, 0.78f, 0.10f, 1.0f);
    static const ImVec4 GoldDim     = ImVec4(0.80f, 0.60f, 0.06f, 1.0f);
    static const ImVec4 GoldBright  = ImVec4(1.00f, 0.92f, 0.45f, 1.0f);
    static const ImVec4 Cyan        = ImVec4(0.00f, 0.95f, 0.85f, 1.0f);
    static const ImVec4 Pink        = ImVec4(1.00f, 0.40f, 0.80f, 1.0f);
    static const ImVec4 Green       = ImVec4(0.00f, 1.00f, 0.60f, 1.0f);
    static const ImVec4 Red         = ImVec4(1.00f, 0.25f, 0.25f, 1.0f);
    static const ImVec4 White       = ImVec4(0.90f, 0.93f, 0.97f, 1.0f);
    static const ImVec4 DarkBg      = ImVec4(0.04f, 0.02f, 0.07f, 0.96f);
    static const ImVec4 PanelBg     = ImVec4(0.06f, 0.03f, 0.10f, 0.94f);
    static const ImVec4 OrganismBg  = ImVec4(0.08f, 0.04f, 0.14f, 0.97f);
    static const ImVec4 Transparent = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
}

//==============================================================================
// FORMAT HELPERS
//==============================================================================
static std::string fmt_money(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << "$" << v;
    return oss.str();
}
static std::string fmt_pct(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << (v * 100.0) << "%";
    return oss.str();
}
static std::string fmt_si(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << v;
    return oss.str();
}

//==============================================================================
// APPLY CYBERPUNK GOLD THEME
//==============================================================================
static void apply_theme() {
    ImGuiStyle& s    = ImGui::GetStyle();
    ImVec4*     c    = s.Colors;

    c[ImGuiCol_WindowBg]          = Colors::DarkBg;
    c[ImGuiCol_ChildBg]           = Colors::PanelBg;
    c[ImGuiCol_PopupBg]           = Colors::DarkBg;
    c[ImGuiCol_Border]            = ImVec4(1.00f, 0.78f, 0.10f, 0.25f);
    c[ImGuiCol_TitleBg]           = ImVec4(0.08f, 0.03f, 0.12f, 1.00f);
    c[ImGuiCol_TitleBgActive]     = ImVec4(0.14f, 0.06f, 0.20f, 1.00f);
    c[ImGuiCol_FrameBg]           = ImVec4(0.10f, 0.04f, 0.16f, 1.00f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.18f, 0.08f, 0.26f, 1.00f);
    c[ImGuiCol_Button]            = ImVec4(0.20f, 0.12f, 0.05f, 1.00f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.40f, 0.28f, 0.04f, 1.00f);
    c[ImGuiCol_ButtonActive]      = ImVec4(1.00f, 0.78f, 0.10f, 1.00f);
    c[ImGuiCol_Header]            = ImVec4(0.20f, 0.10f, 0.04f, 1.00f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.38f, 0.22f, 0.04f, 1.00f);
    c[ImGuiCol_Separator]         = ImVec4(1.00f, 0.78f, 0.10f, 0.35f);
    c[ImGuiCol_ScrollbarBg]       = ImVec4(0.04f, 0.02f, 0.07f, 1.00f);
    c[ImGuiCol_ScrollbarGrab]     = ImVec4(1.00f, 0.78f, 0.10f, 0.60f);
    c[ImGuiCol_SliderGrab]        = ImVec4(1.00f, 0.78f, 0.10f, 0.90f);
    c[ImGuiCol_CheckMark]         = Colors::Gold;
    c[ImGuiCol_Text]              = Colors::White;
    c[ImGuiCol_TextDisabled]      = ImVec4(0.45f, 0.40f, 0.30f, 1.00f);
    c[ImGuiCol_PlotLines]         = Colors::Gold;
    c[ImGuiCol_PlotHistogram]     = Colors::Gold;

    s.WindowRounding    = 6.0f;
    s.FrameRounding     = 4.0f;
    s.GrabRounding      = 3.0f;
    s.ScrollbarRounding = 4.0f;
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 0.0f;
    s.WindowPadding     = ImVec2(10, 8);
    s.ItemSpacing       = ImVec2(8, 5);
}

//==============================================================================
// UI RENDERER CLASS
//==============================================================================
class UIRenderer {
public:
    using DomainClickFn   = std::function<void(uint32_t)>;
    using CirculateFn     = std::function<void(uint32_t)>;
    using SolventFn       = std::function<void()>;
    using PaymentFn       = std::function<void()>;

    UIRenderer(OmniBrainReserveNexus& nexus,
               OrganismRegistry&      registry,
               OmniBrainSphere&       sphere,
               FloatGenMatrix&        matrix)
        : m_nexus(nexus)
        , m_registry(registry)
        , m_sphere(sphere)
        , m_matrix(matrix)
    {
        apply_theme();
    }

    // Callbacks set from main
    DomainClickFn  onDomainClick;
    CirculateFn    onCirculate;
    SolventFn      onTenderSolvency;
    PaymentFn      onEstablishPayment;

    //--------------------------------------------------------------------------
    // render_all() — call each ImGui frame after ImGui::NewFrame()
    //--------------------------------------------------------------------------
    void render_all(float displayW, float displayH,
                    const NodeInspection& nodeInspection,
                    const std::string&    consoleLog) {
        m_displayW = displayW;
        m_displayH = displayH;

        _panel_top_reserve_bar();
        _panel_left_domains();
        _panel_right_organisms();
        _panel_bottom_payment_feed();
        _panel_sphere_overlay();
        _panel_node_inspector(nodeInspection);
        _panel_omnibrain_console(consoleLog);
    }

private:
    float m_displayW = 1920.0f;
    float m_displayH = 1080.0f;

    OmniBrainReserveNexus& m_nexus;
    OrganismRegistry&      m_registry;
    OmniBrainSphere&       m_sphere;
    FloatGenMatrix&        m_matrix;

    // Payment feed scroll
    float m_paymentScroll = 0.0f;

    //==========================================================================
    // PANEL 1 — TOP RESERVE NEXUS BAR
    //==========================================================================
    void _panel_top_reserve_bar() {
        const auto& M = m_nexus.metrics();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(m_displayW, 54), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.93f);
        ImGui::Begin("##ReserveBar", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize     |
            ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar  |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::SetCursorPosY(8.0f);

        // Title
        ImGui::TextColored(Colors::Gold, "  ◈ OMNIBRAIN RESERVE NEXUS");
        ImGui::SameLine();
        ImGui::SetCursorPosX(280.0f);

        // SI — colour-coded: red<1, gold 1-2, bright>2
        double si = M.solvencyIndex.load();
        ImVec4 siColor = (si < 1.0) ? Colors::Red
                       : (si < 2.0) ? Colors::Gold
                                    : Colors::GoldBright;
        ImGui::TextColored(siColor,
            "  SOLVENCY INDEX  %s", fmt_si(si).c_str());
        ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();

        ImGui::TextColored(Colors::Cyan,
            "|  LIQUID ASSETS  %s",
            fmt_money(M.liquidAssets.load()).c_str());
        ImGui::SameLine();

        ImGui::TextColored(Colors::Green,
            "|  YIELD RATE  %s",
            fmt_pct(M.yieldRate.load()).c_str());
        ImGui::SameLine();

        ImGui::TextColored(Colors::Pink,
            "|  FLOAT VOLUME  %s",
            fmt_money(M.floatVolume.load()).c_str());
        ImGui::SameLine();

        ImGui::TextColored(Colors::GoldBright,
            "|  ORGANISMS ONLINE  %u",
            M.organismsOnline.load());
        ImGui::SameLine();

        ImGui::TextColored(Colors::GoldDim,
            "|  RESERVE TOTAL  %s",
            fmt_money(M.reserveTotal.load()).c_str());

        // SPACE / E buttons
        ImGui::SameLine();
        ImGui::SetCursorPosX(m_displayW - 340.0f);
        if (ImGui::Button("SPACE: TENDER SOLVENCY")) {
            if (onTenderSolvency) onTenderSolvency();
        }
        ImGui::SameLine();
        if (ImGui::Button("E: ESTABLISH PAYMENT")) {
            if (onEstablishPayment) onEstablishPayment();
        }

        ImGui::End();
    }

    //==========================================================================
    // PANEL 2 — LEFT PRIVATE WALLET DOMAIN PANEL
    //==========================================================================
    void _panel_left_domains() {
        const float panelW = 270.0f;
        const float panelH = m_displayH - 54.0f - 160.0f;

        ImGui::SetNextWindowPos(ImVec2(0, 54), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.93f);
        ImGui::Begin("##DomainPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        ImGui::TextColored(Colors::Gold, "◈ PRIVATE WALLET DOMAINS");
        ImGui::TextColored(Colors::GoldDim,
            "  Click domain to acquire deposit");
        ImGui::Separator();

        auto& domains = m_nexus.domains();
        for (uint32_t i = 0; i < static_cast<uint32_t>(domains.size()); ++i) {
            const auto& D = domains[i];
            ImGui::PushID(static_cast<int>(i));

            // Domain card
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                ImVec4(0.10f, 0.05f, 0.16f, 0.85f));
            ImGui::BeginChild("##DC", ImVec2(0, 74.0f), true);

            ImGui::TextColored(Colors::GoldBright, "  %s", D.label.c_str());
            ImGui::TextColored(Colors::White,
                "  BAL  %s", fmt_money(D.balance).c_str());
            ImGui::TextColored(Colors::Green,
                "  YIELD  %s", fmt_money(D.pendingYield).c_str());

            ImGui::SameLine();
            ImGui::SetCursorPosX(panelW - 105.0f);
            if (ImGui::Button("ACQUIRE", ImVec2(90.0f, 20.0f))) {
                if (onDomainClick) onDomainClick(i);
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::PopID();
        }
        ImGui::End();
    }

    //==========================================================================
    // PANEL 3 — RIGHT ECONOMIC ORGANISM & SYNDICATE PANEL
    //==========================================================================
    void _panel_right_organisms() {
        const float panelW = 290.0f;
        const float panelH = m_displayH - 54.0f - 160.0f;

        ImGui::SetNextWindowPos(
            ImVec2(m_displayW - panelW, 54), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.93f);
        ImGui::Begin("##OrganismPanel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        ImGui::TextColored(Colors::Gold,
            "◈ ECONOMIC ORGANISMS & SYNDICATES");
        ImGui::Separator();

        auto& orgs = m_registry.all();
        for (uint32_t i = 0; i < static_cast<uint32_t>(orgs.size()); ++i) {
            const auto& org = orgs[i];
            ImGui::PushID(static_cast<int>(i));

            // Organism card — NARU-ATUM gets special highlight
            bool isNaru = (org.name() == "NARU-ATUM");
            ImVec4 cardBg = isNaru
                ? ImVec4(0.14f, 0.08f, 0.03f, 0.96f)
                : ImVec4(0.08f, 0.04f, 0.12f, 0.92f);

            ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
            ImGui::BeginChild("##OC", ImVec2(0, 190.0f), true);

            // Name + state
            ImVec4 nameColor = isNaru ? Colors::GoldBright : Colors::Cyan;
            ImGui::TextColored(nameColor,
                " %s  %s", org.sigil().c_str(), org.name().c_str());

            ImVec4 stateColor = (org.state() == OrganismState::Transcendent)
                ? Colors::GoldBright
                : (org.state() == OrganismState::Circulating)
                ? Colors::Green : Colors::White;
            ImGui::TextColored(stateColor,
                " STATE  %s", org.stateLabel().c_str());

            ImGui::TextColored(Colors::Gold,
                " CAPITAL  %s", fmt_money(org.capital()).c_str());
            ImGui::TextColored(Colors::Green,
                " YIELD    %s", fmt_pct(org.yieldRate()).c_str());
            ImGui::TextColored(Colors::Cyan,
                " CIRCULATIONS  %u", org.circulateCount());

            // Inventory (first 2 items)
            auto& inv = org.inventory();
            if (!inv.empty()) {
                ImGui::TextColored(Colors::GoldDim,
                    " ∷ %s  %.2f", inv[0].label.c_str(), inv[0].quantity);
            }
            if (inv.size() > 1) {
                ImGui::TextColored(Colors::GoldDim,
                    " ∷ %s  %.2f", inv[1].label.c_str(), inv[1].quantity);
            }

            // Syndicate links
            if (!org.syndicateLinks().empty()) {
                ImGui::TextColored(Colors::Pink,
                    " SYNDICATE LINKS: %zu",
                    org.syndicateLinks().size());
            }

            // CIRCULATE button
            ImGui::Spacing();
            ImVec4 btnColor = isNaru
                ? ImVec4(0.55f, 0.35f, 0.02f, 1.0f)
                : ImVec4(0.20f, 0.12f, 0.05f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
            if (ImGui::Button("CIRCULATE \xE2\x80\xA2 EXECUTE PATH",
                              ImVec2(-1.0f, 24.0f))) {
                if (onCirculate) onCirculate(i);
            }
            ImGui::PopStyleColor();

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::PopID();
        }
        ImGui::End();
    }

    //==========================================================================
    // PANEL 4 — BOTTOM PAYMENT ESTABLISHMENT & YIELD DISTRIBUTION FEED
    //==========================================================================
    void _panel_bottom_payment_feed() {
        const float panelH = 155.0f;

        ImGui::SetNextWindowPos(
            ImVec2(0, m_displayH - panelH), ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2(m_displayW, panelH), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.92f);
        ImGui::Begin("##PaymentFeed", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        ImGui::TextColored(Colors::Gold,
            "◈ PAYMENT ESTABLISHMENT & YIELD DISTRIBUTION FEED");
        ImGui::SameLine();
        ImGui::TextColored(Colors::GoldDim,
            "  —  ATOMIC SETTLEMENT CONFIRMATIONS");
        ImGui::Separator();

        // Scrolling payment log
        ImGui::BeginChild("##PayLog",
            ImVec2(0, panelH - 52.0f), false,
            ImGuiWindowFlags_HorizontalScrollbar);

        const auto& log = m_nexus.paymentLog();
        // Draw newest-first
        for (int i = static_cast<int>(log.size()) - 1; i >= 0; --i) {
            const auto& pmt = log[i];
            ImVec4 lineColor = pmt.complete ? Colors::Green : Colors::GoldDim;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "[" << pmt.paymentId << "]"
                << "  " << pmt.fromDomain
                << "  →  " << pmt.toDomain
                << "  |  AMT $" << pmt.amount
                << "  |  YIELD $" << pmt.yieldApplied
                << "  |  " << pmt.status;

            ImGui::TextColored(lineColor, "%s", oss.str().c_str());
        }

        // Auto-scroll to bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }

    //==========================================================================
    // PANEL 5 — SPHERE STATUS OVERLAY (centre, minimal, transparent)
    //==========================================================================
    void _panel_sphere_overlay() {
        ImGui::SetNextWindowPos(
            ImVec2(m_displayW * 0.5f - 160.0f, 60.0f),
            ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(320.0f, 90.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("##SphereOverlay", nullptr,
            ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize  |
            ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoInputs  |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::TextColored(Colors::GoldBright,
            "         ◈  OMNIBRAIN  ◈");
        ImGui::TextColored(Colors::Gold,
            "     HOLOGRAPHIC SPHERE ACTIVE");

        double si = m_nexus.metrics().solvencyIndex.load();
        ImVec4 siCol = si >= 1.0 ? Colors::Green : Colors::Red;
        ImGui::TextColored(siCol,
            "   SI = %-8s  GLOW = %.2f",
            fmt_si(si).c_str(), m_sphere.coreGlow());

        ImGui::End();
    }

    //==========================================================================
    // PANEL 6 — NODE INSPECTOR POPUP
    //==========================================================================
    void _panel_node_inspector(const NodeInspection& insp) {
        if (!insp.valid) return;

        ImGui::SetNextWindowPos(
            ImVec2(m_displayW * 0.5f - 180.0f, m_displayH * 0.5f - 100.0f),
            ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(360.0f, 200.0f), ImGuiCond_Appearing);
        ImGui::SetNextWindowBgAlpha(0.94f);

        bool open = true;
        ImGui::Begin("NODE INSPECTOR", &open);

        ImGui::TextColored(Colors::GoldBright,
            "NODE  %s", insp.label.c_str());
        ImGui::Separator();

        ImGui::TextColored(Colors::Cyan,
            "POSITION   [%.3f,  %.3f,  %.3f]",
            insp.x, insp.y, insp.z);
        ImGui::TextColored(Colors::Pink,
            "FORCE VEC  [%.3f,  %.3f,  %.3f]",
            insp.forceX, insp.forceY, insp.forceZ);
        ImGui::TextColored(Colors::Gold,
            "VALUE      %.2f", insp.value);
        ImGui::TextColored(Colors::Green,
            "FLOW       %.4f", insp.flow);

        ImVec4 stateCol = (insp.state == 2) ? Colors::GoldBright
                        : (insp.state == 1) ? Colors::Green
                                            : Colors::White;
        ImGui::TextColored(stateCol,
            "STATE      %s  [%u]",
            insp.stateLabel.c_str(), insp.state);
        ImGui::TextColored(Colors::Cyan,
            "PATHS      %u connected", insp.connectedPaths);

        ImGui::End();
    }

    //==========================================================================
    // PANEL 7 — OMNIBRAIN CONSOLE (bottom-left, collapsible)
    //==========================================================================
    void _panel_omnibrain_console(const std::string& log) {
        const float conW = 560.0f;
        const float conH = 260.0f;

        ImGui::SetNextWindowPos(
            ImVec2(0.0f, m_displayH - 155.0f - conH),
            ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(conW, conH), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.90f);
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);

        ImGui::Begin("◈  OMNIBRAIN CONSOLE", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::BeginChild("##Log",
            ImVec2(0, conH - 50.0f), false);

        // Split log string by newlines and print each line
        std::istringstream iss(log);
        std::string line;
        while (std::getline(iss, line)) {
            ImVec4 lc = Colors::White;
            if (line.find("[SPACE]") != std::string::npos) lc = Colors::GoldBright;
            else if (line.find("[E]")    != std::string::npos) lc = Colors::Green;
            else if (line.find("BOOT")   != std::string::npos) lc = Colors::Cyan;
            else if (line.find("PMT")    != std::string::npos) lc = Colors::Gold;
            else if (line.find("ERROR")  != std::string::npos) lc = Colors::Red;
            ImGui::TextColored(lc, "%s", line.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::Separator();
        ImGui::TextColored(Colors::GoldDim,
            "SPACE=solvency  E=payment  C=circulate  N=inspect  R=recalc");

        ImGui::End();
    }
};

} // namespace OmniBrain
