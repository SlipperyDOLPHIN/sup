#pragma once
#include "../../Game/SDK/SDK.h"
#include "../Globals/Globals.h"
#include "../Vars/Vars.h"
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

namespace PlayerCache {

    struct CachedPlayer {
        uintptr_t playerAddr;
        uintptr_t characterAddr;
        uintptr_t humanoidAddr;
        uintptr_t rootPartAddr;
        uintptr_t teamAddr;

        std::string name;
        std::string equippedTool;
        RBX::Vec3 position;
        RBX::Vec3 velocity;

        float health;
        float maxHealth;
        float distance;

        bool isValid;
        bool isNPC;
        bool isModerator;
        bool isSpectatingLocal;
    };

    struct CachedItem {
        std::string name;
        RBX::Vec3 position;
        float distance;
    };

    inline std::vector<CachedPlayer> players;
    inline std::vector<CachedItem> items;

    inline RBX::Vec3 localPlayerPos;
    inline RBX::CFrame localPlayerCFrame;
    inline uintptr_t localPlayerTeam = 0;
    inline uintptr_t localCharAddr = 0;

    inline bool moderatorInServer = false;
    inline std::string moderatorName = "";
    inline bool beingSpectated = false;
    inline std::string spectatorName = "";

    inline auto lastUpdate = std::chrono::high_resolution_clock::now();

    inline void UpdatePlayers() {
        try {
            auto now = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count() / 1000.0f;
            if (deltaTime <= 0.001f) deltaTime = 0.016f;
            lastUpdate = now;

            auto playerList = Globals::players.GetChildList();
            if (playerList.size() > 200) return;

            localPlayerTeam = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + offsets::Team);

            auto localChar = Globals::localPlayer.GetModelRef();
            if (localChar.Addr == 0) return;
            localCharAddr = localChar.Addr;

            auto localRoot = localChar.FindChild("HumanoidRootPart");
            if (localRoot.Addr == 0) localRoot = localChar.FindChild("Torso");
            if (localRoot.Addr == 0) localRoot = localChar.FindChild("UpperTorso");
            if (localRoot.Addr == 0) localRoot = localChar.FindChild("Head");
            if (localRoot.Addr == 0) return;

            localPlayerPos = localRoot.GetPos();
            localPlayerCFrame = localRoot.GetCFrame();

            for (auto& cached : players) { cached.isValid = false; }
            
            moderatorInServer = false;
            beingSpectated = false;

            // --- REAL PLAYERS ---
            for (auto& plr : playerList) {
                if (plr.Addr == Globals::localPlayer.Addr || plr.Addr == 0) continue;

                CachedPlayer* existingPlayer = nullptr;
                for (auto& cached : players) {
                    if (!cached.isNPC && cached.playerAddr == plr.Addr) {
                        existingPlayer = &cached;
                        break;
                    }
                }

                if (existingPlayer == nullptr) {
                    CachedPlayer newPlayer;
                    newPlayer.playerAddr = plr.Addr;
                    newPlayer.isNPC = false;
                    newPlayer.name = plr.GetName();
                    newPlayer.isModerator = RBX::IsModerator(newPlayer.name);
                    players.push_back(newPlayer);
                    existingPlayer = &players.back();
                }

                if (existingPlayer->isModerator) {
                    moderatorInServer = true;
                    moderatorName = existingPlayer->name;
                }

                existingPlayer->teamAddr = Coms->ReadMemory<uintptr_t>(plr.Addr + offsets::Team);

                auto character = plr.GetModelRef();
                if (character.Addr == 0) continue;
                existingPlayer->characterAddr = character.Addr;

                auto humanoid = character.FindChildByClass("Humanoid");
                if (humanoid.Addr == 0) continue;
                existingPlayer->humanoidAddr = humanoid.Addr;

                auto rootPart = character.FindChild("HumanoidRootPart");
                if (rootPart.Addr == 0) rootPart = character.FindChild("Torso");
                if (rootPart.Addr == 0) rootPart = character.FindChild("UpperTorso");
                if (rootPart.Addr == 0) rootPart = character.FindChild("Head");
                if (rootPart.Addr == 0) continue;

                // Spectator Check (Proximity based)
                if (Vars::Misc::spectatorWarning) {
                    float dist = rootPart.CalcDistance(localPlayerPos);
                    float vel = sqrtf(existingPlayer->velocity.X * existingPlayer->velocity.X + existingPlayer->velocity.Y * existingPlayer->velocity.Y + existingPlayer->velocity.Z * existingPlayer->velocity.Z);
                    
                    if (dist < 10.0f && vel < 1.0f) {
                        beingSpectated = true;
                        spectatorName = existingPlayer->name;
                    }
                }

                auto tool = character.FindChildByClass("Tool");
                if (tool.Addr != 0) {
                    existingPlayer->equippedTool = tool.GetName();
                }
                else {
                    existingPlayer->equippedTool = "";
                }

                existingPlayer->rootPartAddr = rootPart.Addr;

                RBX::Vec3 newPos = rootPart.GetPos();
                existingPlayer->velocity = {
                    (newPos.X - existingPlayer->position.X) / deltaTime,
                    (newPos.Y - existingPlayer->position.Y) / deltaTime,
                    (newPos.Z - existingPlayer->position.Z) / deltaTime
                };
                existingPlayer->position = newPos;

                existingPlayer->health = Coms->ReadMemory<float>(humanoid.Addr + offsets::Health);
                existingPlayer->maxHealth = Coms->ReadMemory<float>(humanoid.Addr + offsets::MaxHealth);
                existingPlayer->distance = rootPart.CalcDistance(localPlayerPos);
                existingPlayer->isValid = true;
            }

            // --- BOTS / NPCS ---
            if (Vars::ESP::showNPCs || Vars::Aimbot::targetNPCs) {
                auto workspaceChildren = Globals::workspace.GetChildList();
                if (workspaceChildren.size() > 2000) return;

                for (auto& child : workspaceChildren) {
                    if (child.Addr == localChar.Addr || child.Addr == 0) continue;

                    bool isRealPlayer = false;
                    for (auto& cached : players) {
                        if (!cached.isNPC && cached.characterAddr == child.Addr) {
                            isRealPlayer = true;
                            break;
                        }
                    }
                    if (isRealPlayer) continue;

                    auto humanoid = child.FindChildByClass("Humanoid");
                    if (humanoid.Addr == 0) continue;

                    auto rootPart = child.FindChild("HumanoidRootPart");
                    if (rootPart.Addr == 0) rootPart = child.FindChild("Torso");
                    if (rootPart.Addr == 0) rootPart = child.FindChild("UpperTorso");
                    if (rootPart.Addr == 0) rootPart = child.FindChild("Head");
                    if (rootPart.Addr == 0) continue;

                    float health = Coms->ReadMemory<float>(humanoid.Addr + offsets::Health);
                    if (health <= 0) continue;

                    CachedPlayer* existingNPC = nullptr;
                    for (auto& cached : players) {
                        if (cached.isNPC && cached.playerAddr == child.Addr) {
                            existingNPC = &cached;
                            break;
                        }
                    }

                    if (existingNPC == nullptr) {
                        CachedPlayer newNPC;
                        newNPC.playerAddr = child.Addr;
                        newNPC.isNPC = true;
                        newNPC.name = child.GetName();
                        if (newNPC.name.empty()) newNPC.name = "Bot/NPC";
                        players.push_back(newNPC);
                        existingNPC = &players.back();
                    }

                    auto tool = child.FindChildByClass("Tool");
                    if (tool.Addr != 0) {
                        existingNPC->equippedTool = tool.GetName();
                    }
                    else {
                        existingNPC->equippedTool = "";
                    }

                    existingNPC->teamAddr = 0;
                    existingNPC->characterAddr = child.Addr;
                    existingNPC->humanoidAddr = humanoid.Addr;
                    existingNPC->rootPartAddr = rootPart.Addr;

                    RBX::Vec3 newPos = rootPart.GetPos();
                    existingNPC->velocity = {
                        (newPos.X - existingNPC->position.X) / deltaTime,
                        (newPos.Y - existingNPC->position.Y) / deltaTime,
                        (newPos.Z - existingNPC->position.Z) / deltaTime
                    };
                    existingNPC->position = newPos;

                    existingNPC->health = health;
                    existingNPC->maxHealth = Coms->ReadMemory<float>(humanoid.Addr + offsets::MaxHealth);
                    existingNPC->distance = rootPart.CalcDistance(localPlayerPos);
                    existingNPC->isValid = true;
                }
            }

            players.erase(
                std::remove_if(players.begin(), players.end(),
                    [](const CachedPlayer& p) { return !p.isValid; }),
                players.end()
            );

            // --- ITEM ESP ---
            items.clear();
            if (Vars::ESP::items) {
                auto workspaceChildren = Globals::workspace.GetChildList();
                if (workspaceChildren.size() > 2000) return;

                for (auto& child : workspaceChildren) {
                    if (child.GetClass() == "Tool") {
                        auto handle = child.FindChild("Handle");
                        if (handle.Addr != 0) {
                            RBX::Vec3 pos = handle.GetPos();
                            float dist = handle.CalcDistance(localPlayerPos);
                            if (dist <= Vars::ESP::maxItemDistance) {
                                items.push_back({ child.GetName(), pos, dist });
                            }
                        }
                    }
                }
            }

        }
        catch (...) {}
    }
}