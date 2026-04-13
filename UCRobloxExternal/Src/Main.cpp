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
    while (running) {
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

    system("cls");
    Globals::localPlayer = RBX::RbxInstance(localPlayerAddr);

    std::cout << "[+] DataModel: 0x" << std::hex << dataModelPtr << std::dec << "\n";
    std::cout << "[+] VisualEngine: 0x" << std::hex << visualEngine << std::dec << "\n";
    std::cout << "[+] Workspace: 0x" << std::hex << Globals::workspace.Addr << std::dec << "\n";
    std::cout << "[+] Players: 0x" << std::hex << Globals::players.Addr << std::dec << "\n";

    std::cout << "[+] Camera: 0x" << std::hex << Globals::camera.Addr << std::dec << "\n";
    std::cout << "[+] LocalPlayer: 0x" << std::hex << localPlayerAddr << std::dec << "\n\n";

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
        if (!IsGameRunning(L"Roblox"))
        {
            break;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Vars::menuOpen = !Vars::menuOpen;
        }

        static int frameCounter = 0;
        if (frameCounter % 3 == 0) {
            PlayerCache::UpdatePlayers();
        }
        frameCounter++;

        overlay.BeginFrame();

        overlay.RenderMenu();

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

        std::string watermark = "Made by Orange  |  Roblox External  |  FPS: " + std::to_string(fps);
        ImVec2 textSize = ImGui::CalcTextSize(watermark.c_str());
        float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));

        float paddingX = 12.0f;
        float paddingY = 8.0f;
        ImVec2 boxMin(screenW - textSize.x - (paddingX * 2) - 20, 20);
        ImVec2 boxMax(screenW - 20, 20 + textSize.y + (paddingY * 2));

        drawList->AddRectFilled(boxMin, boxMax, IM_COL32(20, 20, 22, 240), 4.0f);
        drawList->AddRectFilled(ImVec2(boxMin.x + 1, boxMin.y + 1), ImVec2(boxMax.x - 1, boxMin.y + 3), IM_COL32(255, 140, 0, 255), 4.0f);
        drawList->AddRect(boxMin, boxMax, IM_COL32(60, 60, 65, 255), 4.0f);

        ImVec2 textPos(boxMin.x + paddingX, boxMin.y + paddingY + 2);
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(textPos, IM_COL32(240, 240, 240, 255), watermark.c_str());

        if (Vars::Aimbot::enabled && Vars::Aimbot::showFOV) {
            ImU32 fovCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::Aimbot::fovColor[0], Vars::Aimbot::fovColor[1], Vars::Aimbot::fovColor[2], Vars::Aimbot::fovColor[3]));
            POINT p;
            GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), 64, Vars::Aimbot::fovThickness + 1.5f);
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, fovCol, 64, Vars::Aimbot::fovThickness);
        }

        auto viewMatrix = Globals::renderEngine.GetViewMat();

        Aimbot::RunTriggerBot(viewMatrix);

        Aimbot::RunAimbot(viewMatrix, drawList);
        Visuals::RenderESP(drawList, viewMatrix);

        overlay.EndFrame();
    }

    running = false;
    localThread.join();

    overlay.Cleanup();

    return 0;
}