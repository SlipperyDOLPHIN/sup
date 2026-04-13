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

        std::string watermark = "Made by 0xSyntex | unKnoWnCheaTs Roblox External | FPS: " + std::to_string(fps);
        ImVec2 textSize = ImGui::CalcTextSize(watermark.c_str());
        float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        ImVec2 watermarkPos = ImVec2(screenW - textSize.x - 10, 10);

        drawList->AddText(ImVec2(watermarkPos.x - 1, watermarkPos.y), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x + 1, watermarkPos.y), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x, watermarkPos.y - 1), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x, watermarkPos.y + 1), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(watermarkPos, IM_COL32(255, 255, 255, 255), watermark.c_str());

        if (Vars::Aimbot::enabled && Vars::Aimbot::showFOV) {
            POINT p;
            GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), 64, 2.0f);
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(255, 255, 255, 255), 64, 1.0f);
        }

        auto viewMatrix = Globals::renderEngine.GetViewMat();

        Aimbot::RunAimbot(viewMatrix, drawList);
        Visuals::RenderESP(drawList, viewMatrix);

        overlay.EndFrame();
    }

    running = false;
    localThread.join();

    overlay.Cleanup();

    return 0;
}