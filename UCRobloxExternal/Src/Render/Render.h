#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <vector>
#include <string>
#include <cmath>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "../Core/Vars/Vars.h"
#include "../Core/Config/Config.h" 
#include "../Core/Cache/Cache.h"         
#include "../Core/Features/Aimbot/Aimbot.h" 

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline std::string GetKeyName(int vk) {
    if (vk == 0) return "[None]";
    if (vk == VK_LBUTTON) return "[L-Mouse]";
    if (vk == VK_RBUTTON) return "[R-Mouse]";
    if (vk == VK_MBUTTON) return "[M-Mouse]";
    if (vk == VK_XBUTTON1) return "[Mouse4]";
    if (vk == VK_XBUTTON2) return "[Mouse5]";
    if (vk == VK_SHIFT) return "[Shift]";
    if (vk == VK_CONTROL) return "[Ctrl]";
    if (vk == VK_MENU) return "[Alt]";
    char name[128];
    UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    int result = GetKeyNameTextA(scanCode << 16, name, 128);
    if (result == 0) return "[Key " + std::to_string(vk) + "]";
    return std::string("[") + name + "]";
}

inline void HotkeyButton(const char* label, int* key, bool& isWaiting) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::Button(isWaiting ? "[Press Any Key]" : GetKeyName(*key).c_str(), ImVec2(100, 25))) {
        isWaiting = true;
    }
    ImGui::PopID();
    if (isWaiting) {
        for (int i = 1; i < 255; i++) {
            if (i == VK_LWIN || i == VK_RWIN) continue;
            if (GetAsyncKeyState(i) & 0x8000) {
                *key = (i == VK_ESCAPE) ? 0 : i;
                isWaiting = false;
                break;
            }
        }
    }
}

struct Notification { std::string text; float lifespan; float maxLifespan; ImVec4 color; };

namespace Notify {
    inline std::vector<Notification> list;
    inline void Add(const std::string& text, ImVec4 color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f), float duration = 3.0f) {
        list.push_back({ text, duration, duration, color });
    }
    inline void Render() {
        if (list.empty()) return;
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        float currentY = screenSize.y - 20.0f;

        for (size_t i = 0; i < list.size(); ) {
            auto& notif = list[i];
            notif.lifespan -= ImGui::GetIO().DeltaTime;
            if (notif.lifespan <= 0.0f) { list.erase(list.begin() + i); continue; }

            float alpha = 1.0f;
            if (notif.lifespan < 0.5f) alpha = notif.lifespan / 0.5f;
            else if (notif.maxLifespan - notif.lifespan < 0.2f) alpha = (notif.maxLifespan - notif.lifespan) / 0.2f;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            ImGui::SetNextWindowPos(ImVec2(screenSize.x - 20.0f, currentY), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, notif.color);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

            std::string windowId = "##notif_" + std::to_string(i);
            if (ImGui::Begin(windowId.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav)) {
                ImGui::TextColored(notif.color, "[!]"); ImGui::SameLine(); ImGui::Text(notif.text.c_str());
                currentY -= ImGui::GetWindowHeight() + 10.0f;
            }
            ImGui::End();
            ImGui::PopStyleVar(2); ImGui::PopStyleColor(2);
            i++;
        }
    }
}

class OverlayWindow {
private:
    HWND windowHandle;
    WNDCLASSEXW windowClass;
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTarget;

    void SetupD3D11(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL obtainedLevel;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            levels, 2, D3D11_SDK_VERSION, &sd,
            &swapChain, &d3dDevice, &obtainedLevel, &d3dContext
        );

        if (hr == DXGI_ERROR_UNSUPPORTED) {
            D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                levels, 2, D3D11_SDK_VERSION, &sd,
                &swapChain, &d3dDevice, &obtainedLevel, &d3dContext
            );
        }

        ID3D11Texture2D* backBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (backBuffer) {
            d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
            backBuffer->Release();
        }
    }

    void CleanupD3D11() {
        if (renderTarget) { renderTarget->Release(); renderTarget = nullptr; }
        if (swapChain) { swapChain->Release(); swapChain = nullptr; }
        if (d3dContext) { d3dContext->Release(); d3dContext = nullptr; }
        if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
    }

public:
    OverlayWindow() : windowHandle(nullptr), d3dDevice(nullptr), d3dContext(nullptr),
        swapChain(nullptr), renderTarget(nullptr) {
        ZeroMemory(&windowClass, sizeof(windowClass));
    }

    bool Initialize() {
        windowClass.cbSize = sizeof(WNDCLASSEXW);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = OverlayWndProc;
        windowClass.hInstance = GetModuleHandleW(nullptr);
        windowClass.lpszClassName = L"Roblox External";

        if (!RegisterClassExW(&windowClass)) return false;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        windowHandle = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            windowClass.lpszClassName, L"Roblox External",
            WS_POPUP, 0, 0, screenW, screenH,
            nullptr, nullptr, windowClass.hInstance, nullptr
        );

        if (!windowHandle) return false;

        SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);
        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(windowHandle, &margins);

        ShowWindow(windowHandle, SW_SHOW);
        UpdateWindow(windowHandle);

        SetupD3D11(windowHandle);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        static std::string layoutPath = Config::GetLayoutPath();
        io.IniFilename = layoutPath.c_str();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 8);
        style.ItemInnerSpacing = ImVec2(6, 6);
        style.ScrollbarSize = 12.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.36f, 0.96f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.36f, 0.96f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.46f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.36f, 0.96f, 0.80f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.36f, 0.96f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.36f, 0.96f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.36f, 0.96f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.55f, 0.36f, 0.96f, 0.50f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.55f, 0.36f, 0.96f, 1.00f);
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

        ImGui_ImplWin32_Init(windowHandle);
        ImGui_ImplDX11_Init(d3dDevice, d3dContext);

        if (Vars::Misc::useCustomFont) {
            io.Fonts->AddFontFromFileTTF(Vars::Misc::customFontPath, Vars::Misc::fontSize);
        }

        Config::Load();

        return true;
    }

    void BeginFrame() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (Vars::menuOpen) {
            SetWindowLong(windowHandle, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        else {
            SetWindowLong(windowHandle, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void RenderMenu(const RBX::Mat4& viewMatrix) {

        if (Vars::Radar::enabled) {
            ImGui::SetNextWindowPos(ImVec2(20, 300), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(Vars::Radar::size, Vars::Radar::size));

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            if (ImGui::Begin("RadarWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse)) {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetWindowPos();
                float s = Vars::Radar::size;
                float radius = s / 2.0f;
                ImVec2 center = ImVec2(p.x + radius, p.y + radius);

                drawList->AddCircleFilled(center, radius, IM_COL32(20, 20, 22, 220), 64);
                drawList->AddCircle(center, radius, IM_COL32(100, 150, 255, 255), 64, 3.0f);

                drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(60, 60, 65, 150));
                drawList->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(60, 60, 65, 150));
                drawList->AddCircleFilled(center, 3.0f, IM_COL32(255, 255, 255, 255));

                RBX::Vec3 rightVec = PlayerCache::localPlayerCFrame.GetRightVector();
                RBX::Vec3 lookVec = PlayerCache::localPlayerCFrame.GetLookVector();

                for (auto& plr : PlayerCache::players) {
                    if (!plr.isValid) continue;
                    if (plr.isNPC && !Vars::ESP::showNPCs) continue;
                    if (!plr.isNPC && Vars::ESP::teamCheck && plr.teamAddr == PlayerCache::localPlayerTeam && plr.teamAddr != 0) continue;

                    float dx = plr.position.X - PlayerCache::localPlayerPos.X;
                    float dy = plr.position.Y - PlayerCache::localPlayerPos.Y;
                    float dz = plr.position.Z - PlayerCache::localPlayerPos.Z;

                    float relX = (dx * rightVec.X) + (dy * rightVec.Y) + (dz * rightVec.Z);
                    float relZ = (dx * lookVec.X) + (dy * lookVec.Y) + (dz * lookVec.Z);

                    float rX = center.x + (relX / Vars::Radar::range) * radius;
                    float rY = center.y + (relZ / Vars::Radar::range) * radius;

                    ImU32 blipCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::Radar::color[0], Vars::Radar::color[1], Vars::Radar::color[2], Vars::Radar::color[3]));
                    if (Vars::ESP::highlightTarget && plr.playerAddr == Aimbot::lockedPlayerAddr) {
                        blipCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::targetHighlightColor[0], Vars::ESP::targetHighlightColor[1], Vars::ESP::targetHighlightColor[2], Vars::ESP::targetHighlightColor[3]));
                    }

                    float distFromCenter = sqrtf((rX - center.x) * (rX - center.x) + (rY - center.y) * (rY - center.y));

                    if (distFromCenter < radius - Vars::Radar::blipSize) {
                        drawList->AddCircleFilled(ImVec2(rX, rY), Vars::Radar::blipSize, blipCol);
                    }
                    else if (plr.distance <= Vars::Radar::range * 1.5f) {
                        float angle = atan2f(rY - center.y, rX - center.x);
                        float clampX = center.x + cosf(angle) * (radius - Vars::Radar::blipSize);
                        float clampY = center.y + sinf(angle) * (radius - Vars::Radar::blipSize);
                        drawList->AddCircleFilled(ImVec2(clampX, clampY), Vars::Radar::blipSize, blipCol);
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        if (Vars::showPlayerList) {
            ImGui::SetNextWindowSize(ImVec2(280, 400), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.09f, 0.85f));
            if (ImGui::Begin("ESP Player List", &Vars::showPlayerList, ImGuiWindowFlags_NoCollapse)) {
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 0.9f, 1.0f), "Players Tracked: %d", PlayerCache::players.size());
                ImGui::Separator();
                if (ImGui::BeginTable("player_table", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY)) {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("HP", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                    ImGui::TableSetupColumn("Dist", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                    ImGui::TableHeadersRow();
                    for (auto& plr : PlayerCache::players) {
                        if (!plr.isValid || plr.isNPC) continue;
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn(); ImGui::Text("%s", plr.name.c_str());
                        ImGui::TableNextColumn(); ImGui::Text("%.0f", plr.health);
                        ImGui::TableNextColumn(); ImGui::Text("%.0fm", plr.distance);
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }

        if (Vars::showHUD) {
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.11f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);

            if (ImGui::Begin("Status HUD", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.0f, 1.0f), "Orange External");
                ImGui::Separator();

                if (Vars::Aimbot::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] Aimbot Active");
                if (Vars::Aimbot::enabled && Vars::Aimbot::currentTargetName != "None") {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "    -> Locked: %s", Vars::Aimbot::currentTargetName.c_str());
                }

                if (Vars::TriggerBot::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] TriggerBot Active");
                if (Vars::AutoClicker::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] AutoClicker Active");
                if (Vars::ESP::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] ESP Active");
                if (Vars::Local::speedEnabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] WalkSpeed [%.0f]", Vars::Local::walkSpeed);
                if (Vars::Local::fovChangerEnabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] Custom FOV [%.0f]", Vars::Local::cameraFOV);
                if (Vars::Misc::streamProof) ImGui::TextColored(ImVec4(0.6f, 0.4f, 1.0f, 1.0f), "[+] Stream Proof Active");

                if (!Vars::Aimbot::enabled && !Vars::ESP::enabled && !Vars::Local::speedEnabled && !Vars::Local::fovChangerEnabled && !Vars::TriggerBot::enabled && !Vars::Misc::streamProof && !Vars::AutoClicker::enabled) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Idle...");
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        if (!Vars::menuOpen) {
            Notify::Render();
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(780, 640), ImGuiCond_FirstUseEver);
        ImGui::Begin("dolphin.club | EX", &Vars::menuOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::BeginChild("TabBar", ImVec2(160, 0), true);

        ImVec2 buttonSize(130, 40);
        const char* tabs[] = { "Aimbot", "Visuals", "Local", "Misc", "Settings" };
        for (int i = 0; i < 5; i++) {
            ImGui::SetCursorPosX(15);
            if (Vars::selectedTab == i) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.85f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.95f, 1.0f));
                if (ImGui::Button(tabs[i], buttonSize)) Vars::selectedTab = i;
                ImGui::PopStyleColor(2);
            }
            else {
                if (ImGui::Button(tabs[i], buttonSize)) Vars::selectedTab = i;
            }
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(15); ImGui::Checkbox("Show HUD", &Vars::showHUD);
        ImGui::SetCursorPosX(15); ImGui::Checkbox("Watermark", &Vars::showWatermark);
        ImGui::SetCursorPosX(15); ImGui::Checkbox("Player List", &Vars::showPlayerList);

        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("Content", ImVec2(0, 0), true);

        static bool waitAim = false, waitTrig = false, waitClick = false, waitNoclip = false, waitInfJump = false, waitMenu = false, waitPanic = false;

        auto BeginPanel = [](const char* name) {
            ImGui::BeginChild(name, ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
            ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), name);
            ImGui::Separator();
            ImGui::Spacing();
            };

        if (Vars::selectedTab == 0) {
            ImGui::Columns(2, nullptr, false);

            BeginPanel("Aimbot Settings");
            ImGui::Checkbox("Enable Aimbot", &Vars::Aimbot::enabled);
            ImGui::Checkbox("Team Check", &Vars::Aimbot::teamCheck);
            ImGui::Checkbox("Target NPCs", &Vars::Aimbot::targetNPCs);
            ImGui::Checkbox("Draw Target Line", &Vars::Aimbot::drawTargetLine);
            HotkeyButton("Aimbot Hotkey", &Vars::Aimbot::aimbotKey, waitAim);
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Smoothing & Method");
            ImGui::SliderFloat("##Smoothing", &Vars::Aimbot::smoothing, 1.0f, 20.0f, "Smoothing: %.1f");
            ImGui::Spacing();
            const char* methods[] = { "Closest to Crosshair", "Closest Distance" };
            ImGui::Combo("Aim Method", &Vars::Aimbot::aimMethod, methods, 2);
            const char* targets[] = { "Head", "HumanoidRootPart" };
            ImGui::Combo("Aim Target", &Vars::Aimbot::aimTarget, targets, 2);
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Velocity Prediction");
            ImGui::Checkbox("Enable Prediction", &Vars::Aimbot::prediction);
            if (Vars::Aimbot::prediction) ImGui::SliderFloat("Bullet Speed", &Vars::Aimbot::bulletSpeed, 100.0f, 3000.0f, "%.0f");
            ImGui::EndChild();

            ImGui::NextColumn();

            BeginPanel("Field of View");
            ImGui::Checkbox("Show FOV", &Vars::Aimbot::showFOV);
            ImGui::SameLine(); ImGui::Checkbox("Dynamic Size", &Vars::Aimbot::dynamicFOV);
            if (Vars::Aimbot::showFOV) {
                ImGui::ColorEdit4("FOV Color", Vars::Aimbot::fovColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Thickness", &Vars::Aimbot::fovThickness, 1.0f, 5.0f, "%.1f");
                ImGui::SliderFloat("Radius", &Vars::Aimbot::fovRadius, 10.0f, 500.0f, "%.0f");
            }
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("TriggerBot");
            ImGui::Checkbox("Enable TriggerBot", &Vars::TriggerBot::enabled);
            ImGui::SameLine(); ImGui::Checkbox("Randomize", &Vars::TriggerBot::randomizeDelay);
            HotkeyButton("Trigger Hotkey", &Vars::TriggerBot::triggerKey, waitTrig);
            if (Vars::TriggerBot::enabled) {
                ImGui::SliderFloat("Hitbox", &Vars::TriggerBot::triggerDistance, 5.0f, 50.0f, "%.1f px");
                ImGui::SliderInt("Delay", &Vars::TriggerBot::clickDelay, 10, 500, "%d ms");
            }
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Auto-Clicker");
            ImGui::Checkbox("Enable AutoClicker", &Vars::AutoClicker::enabled);
            ImGui::SameLine(); ImGui::Checkbox("Random", &Vars::AutoClicker::randomizeDelay);
            HotkeyButton("Clicker Hotkey", &Vars::AutoClicker::clickKey, waitClick);
            if (Vars::AutoClicker::enabled) {
                ImGui::SliderInt("Min CPS", &Vars::AutoClicker::minCPS, 1, 30);
                ImGui::SliderInt("Max CPS", &Vars::AutoClicker::maxCPS, 1, 30);
            }
            ImGui::EndChild();

            ImGui::Columns(1);
        }
        else if (Vars::selectedTab == 1) {
            ImGui::Columns(2, nullptr, false);

            BeginPanel("General ESP");
            ImGui::Checkbox("Enable ESP", &Vars::ESP::enabled);
            ImGui::Checkbox("Team Check", &Vars::ESP::teamCheck);
            ImGui::Checkbox("Show NPCs", &Vars::ESP::showNPCs);
            ImGui::Checkbox("Health Coloring", &Vars::ESP::healthColoring);
            ImGui::SliderFloat("Max Dist", &Vars::ESP::maxDistance, 50.0f, 5000.0f, "%.0f studs");
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Elements");
            ImGui::Checkbox("Boxes", &Vars::ESP::boxes);
            if (Vars::ESP::boxes) {
                const char* boxStyles[] = { "Full Box", "Corner Box" };
                ImGui::Combo("Style", &Vars::ESP::boxStyle, boxStyles, 2);
                ImGui::Checkbox("Box Fill", &Vars::ESP::boxFill);
            }
            ImGui::Spacing();
            ImGui::Checkbox("Skeleton", &Vars::ESP::skeleton);
            if (Vars::ESP::skeleton) ImGui::SliderFloat("Skel Thick", &Vars::ESP::skeletonThickness, 1.0f, 5.0f, "%.1f");
            ImGui::Spacing();
            ImGui::Checkbox("Names", &Vars::ESP::names);
            ImGui::Checkbox("Distance", &Vars::ESP::distance);
            ImGui::Checkbox("Weapon", &Vars::ESP::weapon);
            ImGui::Checkbox("Health Bar", &Vars::ESP::healthBar);
            if (Vars::ESP::healthBar) { ImGui::SameLine(); ImGui::Checkbox("HP Text", &Vars::ESP::healthText); }
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Crosshair");
            ImGui::Checkbox("Crosshair", &Vars::ESP::crosshair);
            if (Vars::ESP::crosshair) {
                ImGui::SliderFloat("Size", &Vars::ESP::crosshairSize, 2.0f, 30.0f, "%.0f");
                ImGui::SliderFloat("Thick", &Vars::ESP::crosshairThickness, 1.0f, 5.0f, "%.1f");
                ImGui::ColorEdit4("Color##CH", Vars::ESP::crosshairColor, ImGuiColorEditFlags_NoInputs);
            }
            ImGui::EndChild();

            ImGui::NextColumn();

            BeginPanel("Target Extras");
            ImGui::Checkbox("Head Dot", &Vars::ESP::headDot);
            if (Vars::ESP::headDot) ImGui::SliderFloat("Dot Size", &Vars::ESP::headDotSize, 2.0f, 15.0f, "%.1f");

            ImGui::Checkbox("Snaplines", &Vars::ESP::snaplines);
            if (Vars::ESP::snaplines) {
                const char* linePos[] = { "Bottom", "Center", "Top" };
                ImGui::Combo("Position", &Vars::ESP::snaplinePos, linePos, 3);
            }

            ImGui::Checkbox("Target Highlight", &Vars::ESP::highlightTarget);
            ImGui::Checkbox("View Tracers", &Vars::ESP::viewAngles);
            ImGui::Checkbox("Off-Screen Arrows", &Vars::ESP::offScreenArrows);
            if (Vars::ESP::offScreenArrows) ImGui::SliderFloat("Arrow Rad", &Vars::ESP::arrowRadius, 50.0f, 400.0f, "%.0f");
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Environment");
            ImGui::Checkbox("Item ESP", &Vars::ESP::items);
            if (Vars::ESP::items) {
                ImGui::SliderFloat("Item Dist", &Vars::ESP::maxItemDistance, 10.0f, 1000.0f, "%.0f");
                ImGui::ColorEdit4("Item Color", Vars::ESP::itemColor, ImGuiColorEditFlags_NoInputs);
            }
            ImGui::Spacing();
            ImGui::Checkbox("2D Radar", &Vars::Radar::enabled);
            if (Vars::Radar::enabled) {
                ImGui::SliderFloat("Radar Range", &Vars::Radar::range, 50.0f, 1000.0f, "%.0f");
                ImGui::SliderFloat("Radar Size", &Vars::Radar::size, 100.0f, 400.0f, "%.0f");
            }
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Colors");
            ImGui::Checkbox("Text Background", &Vars::ESP::textBackground);
            ImGui::Spacing();
            ImGui::ColorEdit4("Box Color", Vars::ESP::boxColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::boxFill) ImGui::ColorEdit4("Fill Color", Vars::ESP::boxFillColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::skeleton) ImGui::ColorEdit4("Skeleton Color", Vars::ESP::skeletonColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::headDot) ImGui::ColorEdit4("Head Dot Color", Vars::ESP::headDotColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::snaplines) ImGui::ColorEdit4("Snapline Color", Vars::ESP::snaplineColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::highlightTarget) ImGui::ColorEdit4("Target Color", Vars::ESP::targetHighlightColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::viewAngles) ImGui::ColorEdit4("Tracer Color", Vars::ESP::viewAngleColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::ESP::offScreenArrows) ImGui::ColorEdit4("Arrow Color", Vars::ESP::arrowColor, ImGuiColorEditFlags_NoInputs);
            if (Vars::Radar::enabled) ImGui::ColorEdit4("Radar Blip Color", Vars::Radar::color, ImGuiColorEditFlags_NoInputs);
            ImGui::EndChild();

            ImGui::Columns(1);
        }
        else if (Vars::selectedTab == 2) {
            ImGui::Columns(2, nullptr, false);

            BeginPanel("Movement");
            ImGui::Checkbox("WalkSpeed", &Vars::Local::speedEnabled);
            ImGui::SliderFloat("##WalkSpeed", &Vars::Local::walkSpeed, 16.0f, 200.0f, "Speed: %.0f");
            ImGui::Spacing();
            ImGui::Checkbox("JumpPower", &Vars::Local::jumpEnabled);
            ImGui::SliderFloat("##JumpPower", &Vars::Local::jumpPower, 50.0f, 200.0f, "Power: %.0f");
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Camera");
            ImGui::Checkbox("Custom FOV", &Vars::Local::fovChangerEnabled);
            ImGui::SliderFloat("##CamFOV", &Vars::Local::cameraFOV, 20.0f, 120.0f, "FOV: %.0f");
            ImGui::EndChild();

            ImGui::NextColumn();

            BeginPanel("Exploits");
            ImGui::Checkbox("Noclip", &Vars::Local::noclipEnabled);
            ImGui::Checkbox("Infinite Jump", &Vars::Local::infiniteJumpEnabled);
            ImGui::EndChild();

            ImGui::Columns(1);
        }
        else if (Vars::selectedTab == 3) {
            ImGui::Columns(2, nullptr, false);

            BeginPanel("Security");
            ImGui::Checkbox("Stream Proof (OBS Bypass)", &Vars::Misc::streamProof);
            ImGui::Checkbox("Spectator Warning", &Vars::Misc::spectatorWarning);
            ImGui::Checkbox("Anti-Mod / Staff Detect", &Vars::Misc::antiMod);
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("Global Hotkeys");
            HotkeyButton("Menu Toggle", &Vars::Misc::Hotkeys::menuKey, waitMenu);
            HotkeyButton("Panic Key", &Vars::Misc::Hotkeys::panicKey, waitPanic);
            HotkeyButton("Noclip Key", &Vars::Misc::Hotkeys::noclipKey, waitNoclip);
            HotkeyButton("Inf Jump Key", &Vars::Misc::Hotkeys::infiniteJumpKey, waitInfJump);
            ImGui::EndChild();

            ImGui::NextColumn();

            BeginPanel("UI Customization");
            ImGui::Checkbox("Use Custom Font", &Vars::Misc::useCustomFont);
            if (Vars::Misc::useCustomFont) {
                ImGui::InputText("Font Path", Vars::Misc::customFontPath, MAX_PATH);
                ImGui::SliderFloat("Font Size", &Vars::Misc::fontSize, 8.0f, 24.0f, "%.1f");
                if (ImGui::Button("Reload Font", ImVec2(-1, 25))) {
                    Notify::Add("Font reload requires restart", ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                }
            }
            ImGui::EndChild();

            ImGui::Columns(1);
        }
        else if (Vars::selectedTab == 4) {
            BeginPanel("Configuration Manager");
            if (ImGui::Button("Refresh Config List", ImVec2(180, 25))) { Config::RefreshConfigs(); Notify::Add("Configs Refreshed"); }

            if (Vars::Configs::list.empty()) Config::RefreshConfigs();
            std::vector<const char*> configNames;
            for (auto& s : Vars::Configs::list) configNames.push_back(s.c_str());
            ImGui::Combo("##ConfigList", &Vars::Configs::selectedIndex, configNames.data(), static_cast<int>(configNames.size()));

            ImGui::Spacing();
            if (ImGui::Button("Load Selected", ImVec2(140, 30))) { Config::Load(); Notify::Add("Config Loaded!"); }
            ImGui::SameLine();
            if (ImGui::Button("Save Selected", ImVec2(140, 30))) { Config::Save(); Notify::Add("Config Overwritten!"); }
            ImGui::EndChild();

            ImGui::Spacing();

            BeginPanel("System");
            ImGui::Text("Build: Premium UI Polish");
            ImGui::Text("Status: Undetected");
            ImGui::Spacing();
            if (ImGui::Button("Force Refresh Cache", ImVec2(180, 30))) { Vars::Misc::forceRefresh = true; }
            if (ImGui::Button("Panic / Exit Cheat", ImVec2(180, 30))) { Vars::Misc::exitCheat = true; }
            ImGui::EndChild();
        }

        ImGui::EndChild();
        ImGui::End();

        if (Vars::Misc::antiMod && PlayerCache::moderatorInServer) {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, 100.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.0f, 0.0f, 0.8f));
            if (ImGui::Begin("StaffWarning", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs)) {
                ImGui::Text("!!! STAFF DETECTED IN SERVER !!!");
                ImGui::Text("Name: %s", PlayerCache::moderatorName.c_str());
            }
            ImGui::End();
            ImGui::PopStyleColor();
        }

        Notify::Render();
    }

    void EndFrame() {
        ImGui::Render();
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        d3dContext->OMSetRenderTargets(1, &renderTarget, nullptr);
        d3dContext->ClearRenderTargetView(renderTarget, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swapChain->Present(0, 0);
    }

    void Cleanup() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupD3D11();
        if (windowHandle) { DestroyWindow(windowHandle); windowHandle = nullptr; }
        UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
    }

    HWND GetWindowHandle() const { return windowHandle; }
};