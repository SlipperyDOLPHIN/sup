#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "../Core/Vars/Vars.h"
#include "../Core/Config/Config.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
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

        if (!RegisterClassExW(&windowClass))
            return false;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        windowHandle = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            windowClass.lpszClassName, L"Roblox External",
            WS_POPUP, 0, 0, screenW, screenH,
            nullptr, nullptr, windowClass.hInstance, nullptr
        );

        if (!windowHandle)
            return false;

        SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);

        MARGINS margins = { -1, -1, -1, -1 };
        DwmExtendFrameIntoClientArea(windowHandle, &margins);

        ShowWindow(windowHandle, SW_SHOW);
        UpdateWindow(windowHandle);

        SetupD3D11(windowHandle);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 8.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.40f, 0.60f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.60f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.60f, 0.95f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.50f, 0.85f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.40f, 0.60f, 0.95f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.50f, 0.85f, 1.00f);

        ImGui_ImplWin32_Init(windowHandle);
        ImGui_ImplDX11_Init(d3dDevice, d3dContext);

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

    void RenderMenu() {
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
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "    -> Locked: %s", Vars::Aimbot::currentTargetName.c_str());
                }

                if (Vars::TriggerBot::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] TriggerBot Active");
                if (Vars::ESP::enabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] ESP Active");
                if (Vars::Local::speedEnabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] WalkSpeed [%.0f]", Vars::Local::walkSpeed);
                if (Vars::Local::fovChangerEnabled) ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "[+] Custom FOV [%.0f]", Vars::Local::cameraFOV);
                if (Vars::Misc::streamProof) ImGui::TextColored(ImVec4(0.6f, 0.4f, 1.0f, 1.0f), "[+] Stream Proof Active");

                if (!Vars::Aimbot::enabled && !Vars::ESP::enabled && !Vars::Local::speedEnabled && !Vars::Local::fovChangerEnabled && !Vars::TriggerBot::enabled && !Vars::Misc::streamProof) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Idle...");
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        if (!Vars::menuOpen) return;

        ImGui::SetNextWindowSize(ImVec2(700, 560), ImGuiCond_FirstUseEver);
        ImGui::Begin("Roblox External", &Vars::menuOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

        ImGui::BeginChild("TabBar", ImVec2(160, 0), true);

        ImGui::SetCursorPosX(15);
        ImVec2 buttonSize(130, 40);

        if (Vars::selectedTab == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.95f, 1.0f));
            if (ImGui::Button("Aimbot", buttonSize)) Vars::selectedTab = 0;
            ImGui::PopStyleColor(2);
        }
        else {
            if (ImGui::Button("Aimbot", buttonSize)) Vars::selectedTab = 0;
        }

        ImGui::SetCursorPosX(15);
        if (Vars::selectedTab == 1) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.95f, 1.0f));
            if (ImGui::Button("Visuals", buttonSize)) Vars::selectedTab = 1;
            ImGui::PopStyleColor(2);
        }
        else {
            if (ImGui::Button("Visuals", buttonSize)) Vars::selectedTab = 1;
        }

        ImGui::SetCursorPosX(15);
        if (Vars::selectedTab == 2) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.95f, 1.0f));
            if (ImGui::Button("Local", buttonSize)) Vars::selectedTab = 2;
            ImGui::PopStyleColor(2);
        }
        else {
            if (ImGui::Button("Local", buttonSize)) Vars::selectedTab = 2;
        }

        ImGui::SetCursorPosX(15);
        if (Vars::selectedTab == 3) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.95f, 1.0f));
            if (ImGui::Button("Settings", buttonSize)) Vars::selectedTab = 3;
            ImGui::PopStyleColor(2);
        }
        else {
            if (ImGui::Button("Settings", buttonSize)) Vars::selectedTab = 3;
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(15);
        ImGui::Checkbox("Show HUD", &Vars::showHUD);

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Content", ImVec2(0, 0), true);

        if (Vars::selectedTab == 0) {
            ImGui::Text("Aimbot");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Enable Aimbot", &Vars::Aimbot::enabled);
            ImGui::Checkbox("Team Check", &Vars::Aimbot::teamCheck);
            ImGui::Checkbox("Target NPCs/Bots", &Vars::Aimbot::targetNPCs);
            ImGui::Checkbox("Draw Target Line", &Vars::Aimbot::drawTargetLine);

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "TriggerBot (Auto-Shoot)");
            ImGui::Checkbox("Enable TriggerBot", &Vars::TriggerBot::enabled);
            if (Vars::TriggerBot::enabled) {
                ImGui::SliderFloat("Hitbox Radius", &Vars::TriggerBot::triggerDistance, 5.0f, 50.0f, "%.1f px");
                ImGui::SliderInt("Click Delay", &Vars::TriggerBot::clickDelay, 10, 500, "%d ms");
            }

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::Checkbox("Show FOV", &Vars::Aimbot::showFOV);
            if (Vars::Aimbot::showFOV) {
                ImGui::ColorEdit4("FOV Color", Vars::Aimbot::fovColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SliderFloat("Thickness", &Vars::Aimbot::fovThickness, 1.0f, 5.0f, "%.1f");
                ImGui::SliderFloat("Radius", &Vars::Aimbot::fovRadius, 10.0f, 500.0f, "%.0f");
            }

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::Text("Smoothing");
            ImGui::SliderFloat("##Smoothing", &Vars::Aimbot::smoothing, 1.0f, 20.0f, "%.1f");

            ImGui::Spacing();
            ImGui::Text("Aim Method (Priority)");
            const char* methods[] = { "Closest to Crosshair", "Closest Distance" };
            ImGui::Combo("##AimMethod", &Vars::Aimbot::aimMethod, methods, 2);

            ImGui::Spacing();
            ImGui::Text("Aim Target");
            const char* targets[] = { "Head", "HumanoidRootPart" };
            ImGui::Combo("##Target", &Vars::Aimbot::aimTarget, targets, 2);

            ImGui::Spacing();
            ImGui::Text("Aimbot Keybind");
            const char* keys[] = { "None", "Left Mouse", "Right Mouse", "Middle Mouse", "X1 Mouse", "X2 Mouse",
                                   "Shift", "Ctrl", "Alt", "C", "V", "X", "Z", "Q", "E", "R", "T", "F", "G" };
            const int keyValues[] = { 0, 1, 2, 4, 5, 6, 16, 17, 18, 0x43, 0x56, 0x58, 0x5A, 0x51, 0x45, 0x52, 0x54, 0x46, 0x47 };

            int currentKeyIndex = 0;
            for (int i = 0; i < 19; i++) {
                if (keyValues[i] == Vars::Aimbot::aimbotKey) {
                    currentKeyIndex = i;
                    break;
                }
            }

            if (ImGui::Combo("##Keybind", &currentKeyIndex, keys, 19)) {
                Vars::Aimbot::aimbotKey = keyValues[currentKeyIndex];
            }
        }
        else if (Vars::selectedTab == 1) {
            ImGui::Text("Visuals");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Enable ESP", &Vars::ESP::enabled);
            ImGui::Checkbox("Team Check", &Vars::ESP::teamCheck);
            ImGui::Checkbox("Show NPCs/Bots", &Vars::ESP::showNPCs);

            ImGui::Text("Max Render Distance");
            ImGui::SliderFloat("##MaxDist", &Vars::ESP::maxDistance, 50.0f, 5000.0f, "%.0f studs");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::Columns(2, nullptr, false);

            ImGui::Checkbox("Boxes", &Vars::ESP::boxes);
            if (Vars::ESP::boxes) {
                const char* boxStyles[] = { "Full Box", "Corner Box" };
                ImGui::Combo("##BoxStyle", &Vars::ESP::boxStyle, boxStyles, 2);
                ImGui::ColorEdit4("Box Color", Vars::ESP::boxColor, ImGuiColorEditFlags_NoInputs);

                ImGui::Checkbox("Box Fill", &Vars::ESP::boxFill);
                if (Vars::ESP::boxFill) {
                    ImGui::ColorEdit4("Fill Color", Vars::ESP::boxFillColor, ImGuiColorEditFlags_NoInputs);
                }
            }
            ImGui::Spacing();

            ImGui::Checkbox("Skeleton", &Vars::ESP::skeleton);
            if (Vars::ESP::skeleton) {
                ImGui::ColorEdit4("Bone Color", Vars::ESP::skeletonColor, ImGuiColorEditFlags_NoInputs);
            }
            ImGui::Spacing();

            ImGui::Checkbox("Head Dot", &Vars::ESP::headDot);
            if (Vars::ESP::headDot) {
                ImGui::SliderFloat("Dot Size", &Vars::ESP::headDotSize, 2.0f, 15.0f, "%.1f");
                ImGui::ColorEdit4("Dot Color", Vars::ESP::headDotColor, ImGuiColorEditFlags_NoInputs);
            }

            ImGui::NextColumn();

            ImGui::Checkbox("Snaplines", &Vars::ESP::snaplines);
            if (Vars::ESP::snaplines) {
                const char* linePos[] = { "Bottom", "Center", "Top" };
                ImGui::Combo("##LinePos", &Vars::ESP::snaplinePos, linePos, 3);
                ImGui::ColorEdit4("Line Color", Vars::ESP::snaplineColor, ImGuiColorEditFlags_NoInputs);
            }
            ImGui::Spacing();
            ImGui::Checkbox("Names", &Vars::ESP::names);
            ImGui::Checkbox("Distance", &Vars::ESP::distance);
            ImGui::Checkbox("Weapon", &Vars::ESP::weapon);

            ImGui::Checkbox("Health Bar", &Vars::ESP::healthBar);
            if (Vars::ESP::healthBar) {
                ImGui::Checkbox("Show HP Text", &Vars::ESP::healthText);
            }
            ImGui::Spacing();

            ImGui::Checkbox("Crosshair", &Vars::ESP::crosshair);
            if (Vars::ESP::crosshair) {
                ImGui::SliderFloat("CH Size", &Vars::ESP::crosshairSize, 2.0f, 30.0f, "%.0f");
                ImGui::SliderFloat("CH Thick", &Vars::ESP::crosshairThickness, 1.0f, 5.0f, "%.1f");
                ImGui::ColorEdit4("CH Color", Vars::ESP::crosshairColor, ImGuiColorEditFlags_NoInputs);
            }

            ImGui::Columns(1);
        }
        else if (Vars::selectedTab == 2) {
            ImGui::Text("Local Player");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("WalkSpeed", &Vars::Local::speedEnabled);
            ImGui::SliderFloat("##WalkSpeed", &Vars::Local::walkSpeed, 16.0f, 200.0f, "%.0f");

            ImGui::Spacing();
            ImGui::Checkbox("JumpPower", &Vars::Local::jumpEnabled);
            ImGui::SliderFloat("##JumpPower", &Vars::Local::jumpPower, 50.0f, 200.0f, "%.0f");

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::Text("Camera");
            ImGui::Checkbox("Custom FOV", &Vars::Local::fovChangerEnabled);
            ImGui::SliderFloat("##CamFOV", &Vars::Local::cameraFOV, 20.0f, 120.0f, "%.0f");
        }
        else if (Vars::selectedTab == 3) {
            ImGui::Text("Settings");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Premium Features");
            ImGui::Checkbox("Stream Proof (Hide from OBS)", &Vars::Misc::streamProof);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Hides the cheat overlay from screen recording software (OBS, Discord, etc.)");
            }

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Save and load your preferred settings.");
            ImGui::Spacing(); ImGui::Spacing();

            if (ImGui::Button("Save Config", ImVec2(140, 40))) {
                Config::Save();
            }
            ImGui::SameLine();
            if (ImGui::Button("Load Config", ImVec2(140, 40))) {
                Config::Load();
            }
        }

        ImGui::EndChild();
        ImGui::End();
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

        if (windowHandle) {
            DestroyWindow(windowHandle);
            windowHandle = nullptr;
        }

        UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
    }

    HWND GetWindowHandle() const { return windowHandle; }
};