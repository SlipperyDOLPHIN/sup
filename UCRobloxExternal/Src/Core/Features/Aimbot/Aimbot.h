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
    inline auto lastAutoClickTime = std::chrono::high_resolution_clock::now();

    inline void MoveMouse(float x, float y) {
        INPUT input = { 0 }; input.type = INPUT_MOUSE; input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dx = static_cast<LONG>(x); input.mi.dy = static_cast<LONG>(y); SendInput(1, &input, sizeof(INPUT));
    }

    inline void MouseClick() {
        INPUT input = { 0 }; input.type = INPUT_MOUSE; input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; SendInput(1, &input, sizeof(INPUT));
        Sleep(10);
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP; SendInput(1, &input, sizeof(INPUT));
    }

    inline float GetDistance2D(RBX::Vec2 a, RBX::Vec2 b) { return sqrtf((a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y)); }

    inline void RunAutoClicker() {
        if (!Vars::AutoClicker::enabled || Vars::AutoClicker::clickKey == 0) return;
        if (!(GetAsyncKeyState(Vars::AutoClicker::clickKey) & 0x8000)) return;

        auto now = std::chrono::high_resolution_clock::now();
        int cps = Vars::AutoClicker::minCPS + (rand() % (Vars::AutoClicker::maxCPS - Vars::AutoClicker::minCPS + 1));
        int delay = 1000 / cps;

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAutoClickTime).count() >= delay) {
            MouseClick();
            lastAutoClickTime = now;
        }
    }

    inline void RunTriggerBot(const RBX::Mat4& viewMatrix) {
        if (!Vars::TriggerBot::enabled || Vars::TriggerBot::triggerKey == 0) return;
        if (!(GetAsyncKeyState(Vars::TriggerBot::triggerKey) & 0x8000)) return;

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTriggerTime).count() < Vars::TriggerBot::clickDelay) return;

        POINT mousePos; GetCursorPos(&mousePos);
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

            if (GetDistance2D(aimCenter, screenPos) < Vars::TriggerBot::triggerDistance) {
                MouseClick();
                lastTriggerTime = std::chrono::high_resolution_clock::now();
                break;
            }
        }
    }

    inline void RunAimbot(const RBX::Mat4& viewMatrix, ImDrawList* drawList, float dynamicFovRadius) {
        if (!Vars::Aimbot::enabled || Vars::Aimbot::aimbotKey == 0) {
            Vars::Aimbot::currentTargetName = "None";
            return;
        }

        if (!(GetAsyncKeyState(Vars::Aimbot::aimbotKey) & 0x8000)) {
            lockedPlayerAddr = 0;
            Vars::Aimbot::currentTargetName = "None";
            return;
        }

        float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        POINT mousePos; GetCursorPos(&mousePos);
        RBX::Vec2 aimCenter = { static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };

        if (lockedPlayerAddr == 0) {
            float bestScore = 999999.0f;
            uintptr_t closestPlayerAddr = 0;
            std::string tempName = "";

            for (auto& plr : PlayerCache::players) {
                if (!plr.isValid) continue;
                if (plr.isNPC && !Vars::Aimbot::targetNPCs) continue;
                if (!plr.isNPC && Vars::Aimbot::teamCheck && plr.teamAddr == PlayerCache::localPlayerTeam && plr.teamAddr != 0) continue;

                auto character = RBX::RbxInstance(plr.characterAddr);
                RBX::RbxInstance targetPart = (Vars::Aimbot::aimTarget == 0) ? character.FindChild("Head") : RBX::RbxInstance(plr.rootPartAddr);
                if (targetPart.Addr == 0) continue;

                RBX::Vec3 targetPos = targetPart.GetPos();

                if (Vars::Aimbot::prediction) {
                    float timeToTarget = plr.distance / Vars::Aimbot::bulletSpeed;
                    targetPos.X += (plr.velocity.X * timeToTarget);
                    targetPos.Y += (plr.velocity.Y * timeToTarget);
                    targetPos.Z += (plr.velocity.Z * timeToTarget);
                }

                RBX::Vec2 screenPos = W2S::WorldToScreen(targetPos, viewMatrix);
                if (screenPos.X == 0 && screenPos.Y == 0) continue;
                if (screenPos.X < 0 || screenPos.Y < 0 || screenPos.X > screenW || screenPos.Y > screenH) continue;

                float dist = GetDistance2D(aimCenter, screenPos);
                if (dist < dynamicFovRadius) {
                    float currentScore = (Vars::Aimbot::aimMethod == 0) ? dist : plr.distance;
                    if (currentScore < bestScore) {
                        bestScore = currentScore;
                        closestPlayerAddr = plr.playerAddr;
                        tempName = plr.name;
                    }
                }
            }

            if (closestPlayerAddr != 0) {
                lockedPlayerAddr = closestPlayerAddr;
                Vars::Aimbot::currentTargetName = tempName;
            }
        }

        if (lockedPlayerAddr != 0) {
            bool foundLockedPlayer = false;
            RBX::Vec2 targetScreenPos = { 0, 0 };

            for (auto& plr : PlayerCache::players) {
                if (!plr.isValid) continue;
                if (plr.playerAddr != lockedPlayerAddr) continue;

                auto character = RBX::RbxInstance(plr.characterAddr);
                RBX::RbxInstance targetPart = (Vars::Aimbot::aimTarget == 0) ? character.FindChild("Head") : RBX::RbxInstance(plr.rootPartAddr);
                if (targetPart.Addr == 0) break;

                RBX::Vec3 targetPos = targetPart.GetPos();

                if (Vars::Aimbot::prediction) {
                    float timeToTarget = plr.distance / Vars::Aimbot::bulletSpeed;
                    targetPos.X += (plr.velocity.X * timeToTarget);
                    targetPos.Y += (plr.velocity.Y * timeToTarget);
                    targetPos.Z += (plr.velocity.Z * timeToTarget);
                }

                RBX::Vec2 screenPos = W2S::WorldToScreen(targetPos, viewMatrix);
                if (screenPos.X == 0 && screenPos.Y == 0) break;
                if (screenPos.X < 0 || screenPos.Y < 0 || screenPos.X > screenW || screenPos.Y > screenH) break;

                targetScreenPos = screenPos;
                foundLockedPlayer = true;
                break;
            }

            if (!foundLockedPlayer) { lockedPlayerAddr = 0; Vars::Aimbot::currentTargetName = "None"; return; }

            if (Vars::Aimbot::drawTargetLine) {
                drawList->AddLine(ImVec2((float)mousePos.x, (float)mousePos.y), ImVec2(targetScreenPos.X, targetScreenPos.Y), IM_COL32(255, 50, 50, 255), 1.5f);
            }

            float dx = targetScreenPos.X - mousePos.x;
            float dy = targetScreenPos.Y - mousePos.y;
            float smoothFactor = 1.0f / Vars::Aimbot::smoothing;
            MoveMouse(dx * smoothFactor, dy * smoothFactor);
        }
    }
}