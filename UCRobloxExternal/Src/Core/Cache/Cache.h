#pragma once
#include "../../Game/SDK/SDK.h"
#include "../Globals/Globals.h"
#include "../Vars/Vars.h"
#include <vector>
#include <string>
#include <algorithm>

namespace PlayerCache {

    struct CachedPlayer {
        uintptr_t playerAddr;
        uintptr_t characterAddr;
        uintptr_t humanoidAddr;
        uintptr_t rootPartAddr;
        uintptr_t teamAddr;

        std::string name;
        RBX::Vec3 position;
        float health;
        float maxHealth;
        float distance;

        bool isValid;
        bool isNPC;
    };

    inline std::vector<CachedPlayer> players;
    inline RBX::Vec3 localPlayerPos;
    inline uintptr_t localPlayerTeam = 0;

    inline void UpdatePlayers() {
        auto playerList = Globals::players.GetChildList();

        localPlayerTeam = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + offsets::Team);

        auto localChar = Globals::localPlayer.GetModelRef();
        if (localChar.Addr == 0) return;

        auto localRoot = localChar.FindChild("HumanoidRootPart");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("Torso");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("UpperTorso");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("Head");
        if (localRoot.Addr == 0) return;

        localPlayerPos = localRoot.GetPos();

        for (auto& cached : players) {
            cached.isValid = false;
        }

        // --- REAL PLAYERS ---
        for (auto& plr : playerList) {
            if (plr.Addr == Globals::localPlayer.Addr) continue;

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
                players.push_back(newPlayer);
                existingPlayer = &players.back();
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

            existingPlayer->rootPartAddr = rootPart.Addr;
            existingPlayer->position = rootPart.GetPos();

            existingPlayer->health = Coms->ReadMemory<float>(humanoid.Addr + offsets::Health);
            existingPlayer->maxHealth = Coms->ReadMemory<float>(humanoid.Addr + offsets::MaxHealth);
            existingPlayer->distance = rootPart.CalcDistance(localPlayerPos);
            existingPlayer->isValid = true;
        }

        // --- BOTS / NPCS ---
        if (Vars::ESP::showNPCs || Vars::Aimbot::targetNPCs) {
            auto workspaceChildren = Globals::workspace.GetChildList();
            for (auto& child : workspaceChildren) {
                if (child.Addr == localChar.Addr) continue;

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

                existingNPC->teamAddr = 0;
                existingNPC->characterAddr = child.Addr;
                existingNPC->humanoidAddr = humanoid.Addr;
                existingNPC->rootPartAddr = rootPart.Addr;
                existingNPC->position = rootPart.GetPos();
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
    }
}