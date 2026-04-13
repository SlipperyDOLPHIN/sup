#pragma once
#include <Windows.h>
#include <string>
#include "../Vars/Vars.h"

namespace Config {
    inline std::string GetPath() {
        char path[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, path);
        return std::string(path) + "\\roblox_external_config.ini";
    }

    inline void WriteBool(const char* section, const char* key, bool value, const std::string& file) {
        WritePrivateProfileStringA(section, key, value ? "1" : "0", file.c_str());
    }
    inline void WriteInt(const char* section, const char* key, int value, const std::string& file) {
        WritePrivateProfileStringA(section, key, std::to_string(value).c_str(), file.c_str());
    }
    inline void WriteFloat(const char* section, const char* key, float value, const std::string& file) {
        WritePrivateProfileStringA(section, key, std::to_string(value).c_str(), file.c_str());
    }
    inline void WriteColor(const char* section, const char* key, float* color, const std::string& file) {
        std::string val = std::to_string(color[0]) + "," + std::to_string(color[1]) + "," + std::to_string(color[2]) + "," + std::to_string(color[3]);
        WritePrivateProfileStringA(section, key, val.c_str(), file.c_str());
    }

    inline bool ReadBool(const char* section, const char* key, bool defaultVal, const std::string& file) {
        return GetPrivateProfileIntA(section, key, defaultVal, file.c_str()) != 0;
    }
    inline int ReadInt(const char* section, const char* key, int defaultVal, const std::string& file) {
        return GetPrivateProfileIntA(section, key, defaultVal, file.c_str());
    }
    inline float ReadFloat(const char* section, const char* key, float defaultVal, const std::string& file) {
        char buf[64];
        GetPrivateProfileStringA(section, key, std::to_string(defaultVal).c_str(), buf, 64, file.c_str());
        return std::stof(buf);
    }
    inline void ReadColor(const char* section, const char* key, float* color, const std::string& file) {
        char buf[128];
        GetPrivateProfileStringA(section, key, "", buf, 128, file.c_str());
        std::string s(buf);
        if (s.empty()) return;
        size_t pos = 0;
        for (int i = 0; i < 4; i++) {
            size_t next = s.find(',', pos);
            if (next == std::string::npos && i < 3) break;
            try { color[i] = std::stof(s.substr(pos, next - pos)); }
            catch (...) {}
            if (next == std::string::npos) break;
            pos = next + 1;
        }
    }

    inline void Save() {
        std::string file = GetPath();

        WriteBool("Global", "showHUD", Vars::showHUD, file);

        WriteBool("Aimbot", "enabled", Vars::Aimbot::enabled, file);
        WriteBool("Aimbot", "teamCheck", Vars::Aimbot::teamCheck, file);
        WriteBool("Aimbot", "targetNPCs", Vars::Aimbot::targetNPCs, file);
        WriteBool("Aimbot", "showFOV", Vars::Aimbot::showFOV, file);
        WriteBool("Aimbot", "drawTargetLine", Vars::Aimbot::drawTargetLine, file);
        WriteFloat("Aimbot", "fovRadius", Vars::Aimbot::fovRadius, file);
        WriteFloat("Aimbot", "smoothing", Vars::Aimbot::smoothing, file);
        WriteInt("Aimbot", "aimTarget", Vars::Aimbot::aimTarget, file);
        WriteInt("Aimbot", "aimMethod", Vars::Aimbot::aimMethod, file);
        WriteInt("Aimbot", "aimbotKey", Vars::Aimbot::aimbotKey, file);
        WriteFloat("Aimbot", "fovThickness", Vars::Aimbot::fovThickness, file);
        WriteColor("Aimbot", "fovColor", Vars::Aimbot::fovColor, file);

        WriteBool("TriggerBot", "enabled", Vars::TriggerBot::enabled, file);
        WriteInt("TriggerBot", "triggerKey", Vars::TriggerBot::triggerKey, file);
        WriteFloat("TriggerBot", "triggerDistance", Vars::TriggerBot::triggerDistance, file);
        WriteInt("TriggerBot", "clickDelay", Vars::TriggerBot::clickDelay, file);

        WriteBool("ESP", "enabled", Vars::ESP::enabled, file);
        WriteBool("ESP", "teamCheck", Vars::ESP::teamCheck, file);
        WriteBool("ESP", "showNPCs", Vars::ESP::showNPCs, file);
        WriteBool("ESP", "boxes", Vars::ESP::boxes, file);
        WriteInt("ESP", "boxStyle", Vars::ESP::boxStyle, file);
        WriteBool("ESP", "boxFill", Vars::ESP::boxFill, file);
        WriteBool("ESP", "skeleton", Vars::ESP::skeleton, file);
        WriteBool("ESP", "headDot", Vars::ESP::headDot, file);
        WriteFloat("ESP", "headDotSize", Vars::ESP::headDotSize, file);
        WriteBool("ESP", "names", Vars::ESP::names, file);
        WriteBool("ESP", "distance", Vars::ESP::distance, file);
        WriteBool("ESP", "weapon", Vars::ESP::weapon, file);
        WriteBool("ESP", "healthBar", Vars::ESP::healthBar, file);
        WriteBool("ESP", "healthText", Vars::ESP::healthText, file);
        WriteBool("ESP", "snaplines", Vars::ESP::snaplines, file);
        WriteInt("ESP", "snaplinePos", Vars::ESP::snaplinePos, file);
        WriteBool("ESP", "crosshair", Vars::ESP::crosshair, file);
        WriteFloat("ESP", "crosshairSize", Vars::ESP::crosshairSize, file);
        WriteFloat("ESP", "crosshairThickness", Vars::ESP::crosshairThickness, file);
        WriteFloat("ESP", "maxDistance", Vars::ESP::maxDistance, file);

        WriteColor("ESP", "boxColor", Vars::ESP::boxColor, file);
        WriteColor("ESP", "boxFillColor", Vars::ESP::boxFillColor, file);
        WriteColor("ESP", "skeletonColor", Vars::ESP::skeletonColor, file);
        WriteColor("ESP", "headDotColor", Vars::ESP::headDotColor, file);
        WriteColor("ESP", "snaplineColor", Vars::ESP::snaplineColor, file);
        WriteColor("ESP", "crosshairColor", Vars::ESP::crosshairColor, file);

        WriteBool("Local", "speedEnabled", Vars::Local::speedEnabled, file);
        WriteFloat("Local", "walkSpeed", Vars::Local::walkSpeed, file);
        WriteBool("Local", "jumpEnabled", Vars::Local::jumpEnabled, file);
        WriteFloat("Local", "jumpPower", Vars::Local::jumpPower, file);
        WriteBool("Local", "fovChangerEnabled", Vars::Local::fovChangerEnabled, file);
        WriteFloat("Local", "cameraFOV", Vars::Local::cameraFOV, file);
    }

    inline void Load() {
        std::string file = GetPath();

        Vars::showHUD = ReadBool("Global", "showHUD", Vars::showHUD, file);

        Vars::Aimbot::enabled = ReadBool("Aimbot", "enabled", Vars::Aimbot::enabled, file);
        Vars::Aimbot::teamCheck = ReadBool("Aimbot", "teamCheck", Vars::Aimbot::teamCheck, file);
        Vars::Aimbot::targetNPCs = ReadBool("Aimbot", "targetNPCs", Vars::Aimbot::targetNPCs, file);
        Vars::Aimbot::showFOV = ReadBool("Aimbot", "showFOV", Vars::Aimbot::showFOV, file);
        Vars::Aimbot::drawTargetLine = ReadBool("Aimbot", "drawTargetLine", Vars::Aimbot::drawTargetLine, file);
        Vars::Aimbot::fovRadius = ReadFloat("Aimbot", "fovRadius", Vars::Aimbot::fovRadius, file);
        Vars::Aimbot::smoothing = ReadFloat("Aimbot", "smoothing", Vars::Aimbot::smoothing, file);
        Vars::Aimbot::aimTarget = ReadInt("Aimbot", "aimTarget", Vars::Aimbot::aimTarget, file);
        Vars::Aimbot::aimMethod = ReadInt("Aimbot", "aimMethod", Vars::Aimbot::aimMethod, file);
        Vars::Aimbot::aimbotKey = ReadInt("Aimbot", "aimbotKey", Vars::Aimbot::aimbotKey, file);
        Vars::Aimbot::fovThickness = ReadFloat("Aimbot", "fovThickness", Vars::Aimbot::fovThickness, file);
        ReadColor("Aimbot", "fovColor", Vars::Aimbot::fovColor, file);

        Vars::TriggerBot::enabled = ReadBool("TriggerBot", "enabled", Vars::TriggerBot::enabled, file);
        Vars::TriggerBot::triggerKey = ReadInt("TriggerBot", "triggerKey", Vars::TriggerBot::triggerKey, file);
        Vars::TriggerBot::triggerDistance = ReadFloat("TriggerBot", "triggerDistance", Vars::TriggerBot::triggerDistance, file);
        Vars::TriggerBot::clickDelay = ReadInt("TriggerBot", "clickDelay", Vars::TriggerBot::clickDelay, file);

        Vars::ESP::enabled = ReadBool("ESP", "enabled", Vars::ESP::enabled, file);
        Vars::ESP::teamCheck = ReadBool("ESP", "teamCheck", Vars::ESP::teamCheck, file);
        Vars::ESP::showNPCs = ReadBool("ESP", "showNPCs", Vars::ESP::showNPCs, file);
        Vars::ESP::boxes = ReadBool("ESP", "boxes", Vars::ESP::boxes, file);
        Vars::ESP::boxStyle = ReadInt("ESP", "boxStyle", Vars::ESP::boxStyle, file);
        Vars::ESP::boxFill = ReadBool("ESP", "boxFill", Vars::ESP::boxFill, file);
        Vars::ESP::skeleton = ReadBool("ESP", "skeleton", Vars::ESP::skeleton, file);
        Vars::ESP::headDot = ReadBool("ESP", "headDot", Vars::ESP::headDot, file);
        Vars::ESP::headDotSize = ReadFloat("ESP", "headDotSize", Vars::ESP::headDotSize, file);
        Vars::ESP::names = ReadBool("ESP", "names", Vars::ESP::names, file);
        Vars::ESP::distance = ReadBool("ESP", "distance", Vars::ESP::distance, file);
        Vars::ESP::weapon = ReadBool("ESP", "weapon", Vars::ESP::weapon, file);
        Vars::ESP::healthBar = ReadBool("ESP", "healthBar", Vars::ESP::healthBar, file);
        Vars::ESP::healthText = ReadBool("ESP", "healthText", Vars::ESP::healthText, file);
        Vars::ESP::snaplines = ReadBool("ESP", "snaplines", Vars::ESP::snaplines, file);
        Vars::ESP::snaplinePos = ReadInt("ESP", "snaplinePos", Vars::ESP::snaplinePos, file);
        Vars::ESP::crosshair = ReadBool("ESP", "crosshair", Vars::ESP::crosshair, file);
        Vars::ESP::crosshairSize = ReadFloat("ESP", "crosshairSize", Vars::ESP::crosshairSize, file);
        Vars::ESP::crosshairThickness = ReadFloat("ESP", "crosshairThickness", Vars::ESP::crosshairThickness, file);
        Vars::ESP::maxDistance = ReadFloat("ESP", "maxDistance", Vars::ESP::maxDistance, file);

        ReadColor("ESP", "boxColor", Vars::ESP::boxColor, file);
        ReadColor("ESP", "boxFillColor", Vars::ESP::boxFillColor, file);
        ReadColor("ESP", "skeletonColor", Vars::ESP::skeletonColor, file);
        ReadColor("ESP", "headDotColor", Vars::ESP::headDotColor, file);
        ReadColor("ESP", "snaplineColor", Vars::ESP::snaplineColor, file);
        ReadColor("ESP", "crosshairColor", Vars::ESP::crosshairColor, file);

        Vars::Local::speedEnabled = ReadBool("Local", "speedEnabled", Vars::Local::speedEnabled, file);
        Vars::Local::walkSpeed = ReadFloat("Local", "walkSpeed", Vars::Local::walkSpeed, file);
        Vars::Local::jumpEnabled = ReadBool("Local", "jumpEnabled", Vars::Local::jumpEnabled, file);
        Vars::Local::jumpPower = ReadFloat("Local", "jumpPower", Vars::Local::jumpPower, file);
        Vars::Local::fovChangerEnabled = ReadBool("Local", "fovChangerEnabled", Vars::Local::fovChangerEnabled, file);
        Vars::Local::cameraFOV = ReadFloat("Local", "cameraFOV", Vars::Local::cameraFOV, file);
    }
}