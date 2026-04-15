#include "Memory/Communication.h"
#include "Game/Offsets/Offsets.h"
#include "Game/SDK/SDK.h"
#include "Render/Render.h"
#include "Core/Globals/Globals.h"
#include "Core/Cache/Cache.h"
#include "Core/Features/Visuals/Visuals.h"
#include "Core/Features/Aimbot/Aimbot.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <atomic>

bool IsGameRunning(const wchar_t* windowTitle)
{
    HWND hwnd = FindWindowW(NULL, windowTitle);
    return hwnd != NULL;
}

std::atomic<bool> running(true);

void LocalPlayerThread() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    bool lastNoclip = false;
    while (running) {
        try {
            if (Globals::camera.Addr != 0 && Vars::Local::fovChangerEnabled) {
                Coms->WriteMemory<float>(Globals::camera.Addr + offsets::FOV, Vars::Local::cameraFOV);
            }

            auto character = Globals::localPlayer.GetModelRef();
            if (character.Addr == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            auto humanoid = character.FindChildByClass("Humanoid");
            if (humanoid.Addr == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            if (Vars::Local::speedEnabled) {
                RBX::ModifyWalkSpeed(humanoid, Vars::Local::walkSpeed);
            }

            if (Vars::Local::jumpEnabled) {
                RBX::ModifyJumpPower(humanoid, Vars::Local::jumpPower);
            }

            if (Vars::Local::noclipEnabled) {
                RBX::SetCanCollide(character, false);
                lastNoclip = true;
            }
            else if (lastNoclip) {
                RBX::SetCanCollide(character, true);
                lastNoclip = false;
            }

            if (Vars::Local::infiniteJumpEnabled) {
                if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                    Coms->WriteMemory<uint8_t>(humanoid.Addr + offsets::Sit, 0); // Unsit if sitting
                    // Setting HumanoidState to Jump (3) or just setting Jump property
                    // Offset for Jump property is often around 0x1DC (Sit)
                    // Let's try writing to the jump state
                    Coms->WriteMemory<int>(humanoid.Addr + 0x1B8, 1); // Jump flag
                }
            }

        }
        catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    std::cout << "[*] Searching for Roblox...\n";

    while (!Coms->Connect(L"RobloxPlayerBeta.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    system("cls");

    auto baseAddr = Coms->GetBase();
    std::cout << "[+] Process ID: " << Coms->GetPID() << "\n";
    std::cout << "[+] Base Address: 0x" << std::hex << baseAddr << std::dec << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));
    system("cls");

    auto fakeDataModelAddr = baseAddr + offsets::FakeDataModelPointer;
    auto fakeDataModel = Coms->ReadMemory<uintptr_t>(fakeDataModelAddr);
    auto dataModelAddr = fakeDataModel + offsets::FakeDataModelToDataModel;
    auto dataModelPtr = Coms->ReadMemory<uintptr_t>(dataModelAddr);

    auto visualEngineAddr = baseAddr + offsets::VisualEnginePointer;
    auto visualEngine = Coms->ReadMemory<uintptr_t>(visualEngineAddr);

    Globals::dataModel = RBX::RbxInstance(dataModelPtr);
    Globals::renderEngine = RBX::RenderEngine(visualEngine);
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");

    auto localPlayerAddr = Coms->ReadMemory<uintptr_t>(Globals::players.Addr + offsets::LocalPlayer);
    Globals::localPlayer = RBX::RbxInstance(localPlayerAddr);

    OverlayWindow overlay;
    if (!overlay.Initialize()) {
        std::cout << "[!] Failed to initialize overlay\n";
        return -1;
    }

    std::cout << "[+] Overlay initialized\n";
    std::cout << "[*] Press INSERT to toggle menu\n\n";

    std::thread localThread(LocalPlayerThread);

    while (Coms->IsConnected())
    {
        if (!IsGameRunning(L"Roblox") || Vars::Misc::exitCheat) {
            break;
        }

        // --- Anti-Mod Auto Exit ---
        if (Vars::Misc::antiMod && PlayerCache::moderatorInServer) {
            Notify::Add("STAFF DETECTED - EMERGENCY EXITING", ImVec4(1.0f, 0.2f, 0.2f, 1.0f), 5.0f);
            Vars::Misc::exitCheat = true;
        }

        if (Vars::Misc::forceRefresh) {
            PlayerCache::players.clear();
            fakeDataModelAddr = baseAddr + offsets::FakeDataModelPointer;
            fakeDataModel = Coms->ReadMemory<uintptr_t>(fakeDataModelAddr);
            dataModelAddr = fakeDataModel + offsets::FakeDataModelToDataModel;
            dataModelPtr = Coms->ReadMemory<uintptr_t>(dataModelAddr);
            visualEngineAddr = baseAddr + offsets::VisualEnginePointer;
            visualEngine = Coms->ReadMemory<uintptr_t>(visualEngineAddr);

            Globals::dataModel = RBX::RbxInstance(dataModelPtr);
            Globals::renderEngine = RBX::RenderEngine(visualEngine);
            Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
            Globals::players = Globals::dataModel.FindChildByClass("Players");
            Globals::camera = Globals::workspace.FindChildByClass("Camera");

            localPlayerAddr = Coms->ReadMemory<uintptr_t>(Globals::players.Addr + offsets::LocalPlayer);
            Globals::localPlayer = RBX::RbxInstance(localPlayerAddr);
            Vars::Misc::forceRefresh = false;
        }

        // --- Hotkey Manager ---
        if (Vars::Misc::Hotkeys::menuKey != 0 && GetAsyncKeyState(Vars::Misc::Hotkeys::menuKey) & 1) {
            Vars::menuOpen = !Vars::menuOpen;
        }

        if (Vars::Misc::Hotkeys::panicKey != 0 && GetAsyncKeyState(Vars::Misc::Hotkeys::panicKey) & 1) {
            Vars::Misc::exitCheat = true;
        }

        if (Vars::Misc::Hotkeys::noclipKey != 0 && GetAsyncKeyState(Vars::Misc::Hotkeys::noclipKey) & 1) {
            Vars::Local::noclipEnabled = !Vars::Local::noclipEnabled;
            Notify::Add(Vars::Local::noclipEnabled ? "Noclip Enabled" : "Noclip Disabled", Vars::Local::noclipEnabled ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        }

        if (Vars::Misc::Hotkeys::infiniteJumpKey != 0 && GetAsyncKeyState(Vars::Misc::Hotkeys::infiniteJumpKey) & 1) {
            Vars::Local::infiniteJumpEnabled = !Vars::Local::infiniteJumpEnabled;
            Notify::Add(Vars::Local::infiniteJumpEnabled ? "Infinite Jump Enabled" : "Infinite Jump Disabled", Vars::Local::infiniteJumpEnabled ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        }

        if (Vars::Misc::streamProof) {
            SetWindowDisplayAffinity(overlay.GetWindowHandle(), WDA_EXCLUDEFROMCAPTURE);
        }
        else {
            SetWindowDisplayAffinity(overlay.GetWindowHandle(), WDA_NONE);
        }

        try {
            static int frameCounter = 0;
            if (frameCounter % 3 == 0) {
                PlayerCache::UpdatePlayers();
            }
            frameCounter++;
        }
        catch (...) {}

        auto viewMatrix = Globals::renderEngine.GetViewMat();

        overlay.BeginFrame();
        overlay.RenderMenu(viewMatrix);

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        static auto lastTime = std::chrono::high_resolution_clock::now();
        static int frameCount = 0;
        static int fps = 0;

        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

        if (elapsed >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }

        if (Vars::showWatermark) {
            std::string watermark = "dolphin.club | EX | FPS: " + std::to_string(fps);
            ImVec2 textSize = ImGui::CalcTextSize(watermark.c_str());
            float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
            float paddingX = 12.0f;
            float paddingY = 8.0f;
            ImVec2 boxMin(screenW - textSize.x - (paddingX * 2) - 20, 20);
            ImVec2 boxMax(screenW - 20, 20 + textSize.y + (paddingY * 2));

            drawList->AddRectFilled(boxMin, boxMax, IM_COL32(20, 20, 22, 240), 6.0f);
            drawList->AddRectFilled(ImVec2(boxMin.x + 1, boxMin.y + 1), ImVec2(boxMax.x - 1, boxMin.y + 3), IM_COL32(100, 150, 255, 255), 6.0f);
            drawList->AddRect(boxMin, boxMax, IM_COL32(50, 50, 55, 255), 6.0f);

            ImVec2 textPos(boxMin.x + paddingX, boxMin.y + paddingY + 2);
            drawList->AddText(textPos, IM_COL32(240, 240, 240, 255), watermark.c_str());
        }

        float dynamicRadius = Vars::Aimbot::fovRadius;
        if (Vars::Aimbot::dynamicFOV) {
            float fovScale = viewMatrix.data[5];
            if (fovScale > 0.1f) dynamicRadius = Vars::Aimbot::fovRadius * (1.0f / fovScale);
        }

        if (Vars::Aimbot::enabled && Vars::Aimbot::showFOV) {
            ImU32 fovCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::Aimbot::fovColor[0], Vars::Aimbot::fovColor[1], Vars::Aimbot::fovColor[2], Vars::Aimbot::fovColor[3]));
            POINT p; GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, dynamicRadius, IM_COL32(0, 0, 0, 255), 64, Vars::Aimbot::fovThickness + 1.5f);
            drawList->AddCircle(center, dynamicRadius, fovCol, 64, Vars::Aimbot::fovThickness);
        }

        try {
            Aimbot::RunAutoClicker();
            Aimbot::RunTriggerBot(viewMatrix);
            Aimbot::RunAimbot(viewMatrix, drawList, dynamicRadius);
            Visuals::RenderESP(drawList, viewMatrix);
        }
        catch (...) {}

        overlay.EndFrame();
    }

    running = false;
    localThread.join();
    overlay.Cleanup();
    return 0;
}