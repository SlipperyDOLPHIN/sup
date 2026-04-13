#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Vars/Vars.h"
#include "../../../Game/SDK/SDK.h"
#include <windows.h>
#include <cmath>
#include <chrono>

namespace Aimbot {

    inline uintptr_t lockedPlayerAddr = 0;
    inline auto lastTriggerTime = std::chrono::high_resolution_clock::now();

    inline void MoveMouse(float x, float y)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dx = static_cast<LONG>(x);
        input.mi.dy = static_cast<LONG>(y);
        SendInput(1, &input, sizeof(INPUT));
    }

    inline void MouseClick() {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(10);

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    inline float GetDistance2D(RBX::Vec2 a, RBX::Vec2 b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        return sqrtf(dx * dx + dy * dy);
    }

    inline void RunTriggerBot(const RBX::Mat4& viewMatrix) {
        if (!Vars::TriggerBot::enabled) return;

        bool keyPressed = false;
        if (Vars::TriggerBot::triggerKey == 1) keyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 2) keyPressed = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 4) keyPressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 5) keyPressed = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 6) keyPressed = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 16) keyPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 17) keyPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
        else if (Vars::TriggerBot::triggerKey == 18) keyPressed = (GetAsyncKeyState(VK_MENU) & 0x8000);
        else if (Vars::TriggerBot::triggerKey > 0) keyPressed = (GetAsyncKeyState(Vars::TriggerBot::triggerKey) & 0x8000);

        if (!keyPressed) return;

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTriggerTime).count() < Vars::TriggerBot::clickDelay) {
            return;
        }

        POINT mousePos;
        GetCursorPos(&mousePos);
        RBX::Vec2 aimCenter = { static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid) continue;

            if (plr.isNPC && !Vars::Aimbot::targetNPCs) continue;
            if (!plr.isNPC && Vars::Aimbot::teamCheck && plr.teamAddr == PlayerCache::localPlayerTeam && plr.teamAddr != 0) continue;

            auto character = RBX::RbxInstance(plr.characterAddr);
            auto targetPart = character.FindChild("Head");
            if (targetPart.Addr == 0) targetPart = character.FindChild("UpperTorso");
            if (targetPart.Addr == 0) targetPart = character.FindChild("Torso");
            if (targetPart.Addr == 0) continue;

            RBX::Vec2 screenPos = W2S::WorldToScreen(targetPart.GetPos(), viewMatrix);
            if (screenPos.X == 0 && screenPos.Y == 0) continue;

            float dist2D = GetDistance2D(aimCenter, screenPos);

            if (dist2D < Vars::TriggerBot::triggerDistance) {
                MouseClick();
                lastTriggerTime = std::chrono::high_resolution_clock::now();
                break;
            }
        }
    }

    inline void RunAimbot(const RBX::Mat4& viewMatrix, ImDrawList* drawList) {
        if (!Vars::Aimbot::enabled) {
            Vars::Aimbot::currentTargetName = "None";
            return;
        }

        bool keyPressed = false;
        if (Vars::Aimbot::aimbotKey == 1) keyPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 2) keyPressed = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 4) keyPressed = (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 5) keyPressed = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 6) keyPressed = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 16) keyPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 17) keyPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
        else if (Vars::Aimbot::aimbotKey == 18) keyPressed = (GetAsyncKeyState(VK_MENU) & 0x8000);
        else if (Vars::Aimbot::aimbotKey > 0) keyPressed = (GetAsyncKeyState(Vars::Aimbot::aimbotKey) & 0x8000);

        if (!keyPressed)
        {
            lockedPlayerAddr = 0;
            Vars::Aimbot::currentTargetName = "None";
            return;
        }

        float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

        POINT mousePos;
        GetCursorPos(&mousePos);
        RBX::Vec2 aimCenter = { static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

        if (lockedPlayerAddr == 0) {
            float bestScore = 999999.0f;
            RBX::Vec2 closestTarget = { 0,0 };
            uintptr_t closestPlayerAddr = 0;
            std::string tempTargetName = "";

            for (auto& plr : PlayerCache::players) {
                if (!plr.isValid) continue;

                if (plr.isNPC && !Vars::Aimbot::targetNPCs) continue;
                if (!plr.isNPC && Vars::Aimbot::teamCheck && plr.teamAddr == PlayerCache::localPlayerTeam && plr.teamAddr != 0) continue;

                auto character = RBX::RbxInstance(plr.characterAddr);
                RBX::RbxInstance targetPart = RBX::RbxInstance(0);

                if (Vars::Aimbot::aimTarget == 0) {
                    targetPart = character.FindChild("Head");
                }
                else {
                    targetPart = RBX::RbxInstance(plr.rootPartAddr);
                }

                if (targetPart.Addr == 0) continue;

                RBX::Vec3 targetPos = targetPart.GetPos();
                RBX::Vec2 screenPos = W2S::WorldToScreen(targetPos, viewMatrix);

                if (screenPos.X == 0 && screenPos.Y == 0) continue;
                if (screenPos.X < 0 || screenPos.Y < 0 || screenPos.X > screenW || screenPos.Y > screenH) continue;

                float dist2D = GetDistance2D(aimCenter, screenPos);

                if (dist2D < Vars::Aimbot::fovRadius) {
                    float currentScore = (Vars::Aimbot::aimMethod == 0) ? dist2D : plr.distance;

                    if (currentScore < bestScore) {
                        bestScore = currentScore;
                        closestTarget = screenPos;
                        closestPlayerAddr = plr.playerAddr;
                        tempTargetName = plr.name;
                    }
                }
            }

            if (closestPlayerAddr != 0) {
                lockedPlayerAddr = closestPlayerAddr;
                Vars::Aimbot::currentTargetName = tempTargetName;
            }
        }

        if (lockedPlayerAddr != 0) {
            bool foundLockedPlayer = false;
            RBX::Vec2 targetScreenPos = { 0, 0 };

            for (auto& plr : PlayerCache::players) {
                if (!plr.isValid) continue;
                if (plr.playerAddr != lockedPlayerAddr) continue;

                auto character = RBX::RbxInstance(plr.characterAddr);
                RBX::RbxInstance targetPart = RBX::RbxInstance(0);

                if (Vars::Aimbot::aimTarget == 0) {
                    targetPart = character.FindChild("Head");
                }
                else {
                    targetPart = RBX::RbxInstance(plr.rootPartAddr);
                }

                if (targetPart.Addr == 0) break;

                RBX::Vec3 targetPos = targetPart.GetPos();
                RBX::Vec2 screenPos = W2S::WorldToScreen(targetPos, viewMatrix);

                if (screenPos.X == 0 && screenPos.Y == 0) break;
                if (screenPos.X < 0 || screenPos.Y < 0 || screenPos.X > screenW || screenPos.Y > screenH) break;

                targetScreenPos = screenPos;
                foundLockedPlayer = true;
                break;
            }

            if (!foundLockedPlayer) {
                lockedPlayerAddr = 0;
                Vars::Aimbot::currentTargetName = "None";
                return;
            }

            if (Vars::Aimbot::drawTargetLine) {
                drawList->AddLine(
                    ImVec2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)),
                    ImVec2(targetScreenPos.X, targetScreenPos.Y),
                    IM_COL32(255, 50, 50, 255),
                    1.5f
                );
            }

            float dx = targetScreenPos.X - mousePos.x;
            float dy = targetScreenPos.Y - mousePos.y;

            float smoothFactor = 1.0f / Vars::Aimbot::smoothing;

            MoveMouse(dx * smoothFactor, dy * smoothFactor);
        }
    }
}