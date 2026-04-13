#pragma once
#include "../../Game/SDK/SDK.h"
#include "../Globals/Globals.h"
#include <vector>
#include <string>
#include <algorithm>

namespace PlayerCache {

    struct CachedPlayer {
        uintptr_t playerAddr;
        uintptr_t characterAddr;
        uintptr_t humanoidAddr;
        uintptr_t rootPartAddr;
        uintptr_t teamAddr; // [NEW] Team Address

        std::string name;
        RBX::Vec3 position;
        float health;       // [FIXED] Roblox health is a float
        float maxHealth;    // [FIXED] Roblox health is a float
        float distance;

        bool isValid;
    };

    inline std::vector<CachedPlayer> players;
    inline RBX::Vec3 localPlayerPos;
    inline uintptr_t localPlayerTeam = 0; // [NEW] Store LocalPlayer's Team

    inline void UpdatePlayers() {
        auto playerList = Globals::players.GetChildList();

        // [NEW] Get LocalPlayer's Team
        localPlayerTeam = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + offsets::Team);

        auto localChar = Globals::localPlayer.GetModelRef();
        if (localChar.Addr == 0) return;

        // [FIXED] Fallback logic so it doesn't break on custom game rigs
        auto localRoot = localChar.FindChild("HumanoidRootPart");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("Torso");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("UpperTorso");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("Head");
        if (localRoot.Addr == 0) return;

        localPlayerPos = localRoot.GetPos();

        for (auto& cached : players) {
            cached.isValid = false;
        }

        for (auto& plr : playerList) {
            if (plr.Addr == Globals::localPlayer.Addr) continue;

            CachedPlayer* existingPlayer = nullptr;
            for (auto& cached : players) {
                if (cached.playerAddr == plr.Addr) {
                    existingPlayer = &cached;
                    break;
                }
            }

            if (existingPlayer == nullptr) {
                CachedPlayer newPlayer;
                newPlayer.playerAddr = plr.Addr;
                newPlayer.name = plr.GetName();
                players.push_back(newPlayer);
                existingPlayer = &players.back();
            }

            // [NEW] Read target player's Team
            existingPlayer->teamAddr = Coms->ReadMemory<uintptr_t>(plr.Addr + offsets::Team);

            auto character = plr.GetModelRef();
            if (character.Addr == 0) continue;

            existingPlayer->characterAddr = character.Addr;

            auto humanoid = character.FindChildByClass("Humanoid");
            if (humanoid.Addr == 0) continue;

            existingPlayer->humanoidAddr = humanoid.Addr;

            // [FIXED] Fallback logic for missing HumanoidRootPart
            auto rootPart = character.FindChild("HumanoidRootPart");
            if (rootPart.Addr == 0) rootPart = character.FindChild("Torso");
            if (rootPart.Addr == 0) rootPart = character.FindChild("UpperTorso");
            if (rootPart.Addr == 0) rootPart = character.FindChild("Head");
            if (rootPart.Addr == 0) continue;

            existingPlayer->rootPartAddr = rootPart.Addr;
            existingPlayer->position = rootPart.GetPos();

            // [FIXED] Reading health as float
            existingPlayer->health = Coms->ReadMemory<float>(humanoid.Addr + offsets::Health);
            existingPlayer->maxHealth = Coms->ReadMemory<float>(humanoid.Addr + offsets::MaxHealth);

            existingPlayer->distance = rootPart.CalcDistance(localPlayerPos);
            existingPlayer->isValid = true;
        }

        players.erase(
            std::remove_if(players.begin(), players.end(),
                [](const CachedPlayer& p) { return !p.isValid; }),
            players.end()
        );
    }
}