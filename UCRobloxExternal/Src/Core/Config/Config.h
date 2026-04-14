#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "../Vars/Vars.h"

namespace Config {

    inline std::string GetExeDirectory() {
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string path(buffer);
        std::string::size_type pos = path.find_last_of("\\/");
        return path.substr(0, pos);
    }

    inline void RefreshConfigs() {
        Vars::Configs::list.clear();
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA((GetExeDirectory() + "\\*.ini").c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string fname = fd.cFileName;
                if (fname != "roblox_imgui_layout.ini") {
                    Vars::Configs::list.push_back(fname);
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
        if (Vars::Configs::list.empty()) {
            Vars::Configs::list.push_back("default.ini");
        }
    }

    inline std::string GetCurrentPath() {
        if (Vars::Configs::list.empty()) return GetExeDirectory() + "\\default.ini";
        return GetExeDirectory() + "\\" + Vars::Configs::list[Vars::Configs::selectedIndex];
    }

    inline std::string GetLayoutPath() {
        return GetExeDirectory() + "\\roblox_imgui_layout.ini";
    }

    inline void WriteBool(const char* s, const char* k, bool v, const std::string& f) { WritePrivateProfileStringA(s, k, v ? "1" : "0", f.c_str()); }
    inline void WriteInt(const char* s, const char* k, int v, const std::string& f) { WritePrivateProfileStringA(s, k, std::to_string(v).c_str(), f.c_str()); }
    inline void WriteFloat(const char* s, const char* k, float v, const std::string& f) { WritePrivateProfileStringA(s, k, std::to_string(v).c_str(), f.c_str()); }
    inline void WriteColor(const char* s, const char* k, float* c, const std::string& f) {
        std::string val = std::to_string(c[0]) + "," + std::to_string(c[1]) + "," + std::to_string(c[2]) + "," + std::to_string(c[3]);
        WritePrivateProfileStringA(s, k, val.c_str(), f.c_str());
    }

    inline bool ReadBool(const char* s, const char* k, bool d, const std::string& f) { return GetPrivateProfileIntA(s, k, d, f.c_str()) != 0; }
    inline int ReadInt(const char* s, const char* k, int d, const std::string& f) { return GetPrivateProfileIntA(s, k, d, f.c_str()); }
    inline float ReadFloat(const char* s, const char* k, float d, const std::string& f) {
        char buf[64]; GetPrivateProfileStringA(s, k, std::to_string(d).c_str(), buf, 64, f.c_str()); return std::stof(buf);
    }
    inline void ReadColor(const char* s, const char* k, float* c, const std::string& f) {
        char buf[128]; GetPrivateProfileStringA(s, k, "", buf, 128, f.c_str());
        std::string str(buf); if (str.empty()) return;
        size_t pos = 0;
        for (int i = 0; i < 4; i++) {
            size_t next = str.find(',', pos);
            if (next == std::string::npos && i < 3) break;
            try { c[i] = std::stof(str.substr(pos, next - pos)); }
            catch (...) {}
            if (next == std::string::npos) break;
            pos = next + 1;
        }
    }

    inline void Save(std::string specificName = "") {
        std::string f = specificName.empty() ? GetCurrentPath() : GetExeDirectory() + "\\" + specificName;

        WriteBool("Global", "showHUD", Vars::showHUD, f);
        WriteBool("Global", "showWatermark", Vars::showWatermark, f);
        WriteBool("Global", "showPlayerList", Vars::showPlayerList, f);
        WriteBool("Misc", "streamProof", Vars::Misc::streamProof, f);

        WriteBool("Radar", "enabled", Vars::Radar::enabled, f);
        WriteFloat("Radar", "range", Vars::Radar::range, f);
        WriteFloat("Radar", "size", Vars::Radar::size, f);
        WriteFloat("Radar", "blipSize", Vars::Radar::blipSize, f);
        WriteColor("Radar", "color", Vars::Radar::color, f);

        WriteBool("Aimbot", "enabled", Vars::Aimbot::enabled, f);
        WriteBool("Aimbot", "teamCheck", Vars::Aimbot::teamCheck, f);
        WriteBool("Aimbot", "targetNPCs", Vars::Aimbot::targetNPCs, f);
        WriteBool("Aimbot", "showFOV", Vars::Aimbot::showFOV, f);
        WriteBool("Aimbot", "dynamicFOV", Vars::Aimbot::dynamicFOV, f);
        WriteBool("Aimbot", "drawTargetLine", Vars::Aimbot::drawTargetLine, f);
        WriteBool("Aimbot", "prediction", Vars::Aimbot::prediction, f);
        WriteFloat("Aimbot", "bulletSpeed", Vars::Aimbot::bulletSpeed, f);
        WriteFloat("Aimbot", "fovRadius", Vars::Aimbot::fovRadius, f);
        WriteFloat("Aimbot", "smoothing", Vars::Aimbot::smoothing, f);
        WriteInt("Aimbot", "aimTarget", Vars::Aimbot::aimTarget, f);
        WriteInt("Aimbot", "aimMethod", Vars::Aimbot::aimMethod, f);
        WriteInt("Aimbot", "aimbotKey", Vars::Aimbot::aimbotKey, f);
        WriteFloat("Aimbot", "fovThickness", Vars::Aimbot::fovThickness, f);
        WriteColor("Aimbot", "fovColor", Vars::Aimbot::fovColor, f);

        WriteBool("TriggerBot", "enabled", Vars::TriggerBot::enabled, f);
        WriteInt("TriggerBot", "triggerKey", Vars::TriggerBot::triggerKey, f);
        WriteFloat("TriggerBot", "triggerDistance", Vars::TriggerBot::triggerDistance, f);
        WriteInt("TriggerBot", "clickDelay", Vars::TriggerBot::clickDelay, f);

        WriteBool("AutoClicker", "enabled", Vars::AutoClicker::enabled, f);
        WriteInt("AutoClicker", "clickKey", Vars::AutoClicker::clickKey, f);
        WriteInt("AutoClicker", "minCPS", Vars::AutoClicker::minCPS, f);
        WriteInt("AutoClicker", "maxCPS", Vars::AutoClicker::maxCPS, f);

        WriteBool("ESP", "enabled", Vars::ESP::enabled, f);
        WriteBool("ESP", "teamCheck", Vars::ESP::teamCheck, f);
        WriteBool("ESP", "showNPCs", Vars::ESP::showNPCs, f);
        WriteBool("ESP", "items", Vars::ESP::items, f);
        WriteFloat("ESP", "maxItemDistance", Vars::ESP::maxItemDistance, f);
        WriteColor("ESP", "itemColor", Vars::ESP::itemColor, f);
        WriteBool("ESP", "highlightTarget", Vars::ESP::highlightTarget, f);
        WriteBool("ESP", "viewAngles", Vars::ESP::viewAngles, f);
        WriteBool("ESP", "offScreenArrows", Vars::ESP::offScreenArrows, f);
        WriteFloat("ESP", "arrowRadius", Vars::ESP::arrowRadius, f);
        WriteFloat("ESP", "arrowSize", Vars::ESP::arrowSize, f);
        WriteBool("ESP", "boxes", Vars::ESP::boxes, f);
        WriteInt("ESP", "boxStyle", Vars::ESP::boxStyle, f);
        WriteBool("ESP", "boxFill", Vars::ESP::boxFill, f);
        WriteBool("ESP", "skeleton", Vars::ESP::skeleton, f);
        WriteFloat("ESP", "skeletonThickness", Vars::ESP::skeletonThickness, f);
        WriteBool("ESP", "headDot", Vars::ESP::headDot, f);
        WriteFloat("ESP", "headDotSize", Vars::ESP::headDotSize, f);
        WriteBool("ESP", "names", Vars::ESP::names, f);
        WriteBool("ESP", "distance", Vars::ESP::distance, f);
        WriteBool("ESP", "weapon", Vars::ESP::weapon, f);
        WriteBool("ESP", "healthBar", Vars::ESP::healthBar, f);
        WriteBool("ESP", "healthText", Vars::ESP::healthText, f);
        WriteBool("ESP", "textBackground", Vars::ESP::textBackground, f);
        WriteBool("ESP", "snaplines", Vars::ESP::snaplines, f);
        WriteInt("ESP", "snaplinePos", Vars::ESP::snaplinePos, f);
        WriteBool("ESP", "crosshair", Vars::ESP::crosshair, f);
        WriteFloat("ESP", "crosshairSize", Vars::ESP::crosshairSize, f);
        WriteFloat("ESP", "crosshairThickness", Vars::ESP::crosshairThickness, f);
        WriteFloat("ESP", "maxDistance", Vars::ESP::maxDistance, f);

        WriteColor("ESP", "boxColor", Vars::ESP::boxColor, f);
        WriteColor("ESP", "boxFillColor", Vars::ESP::boxFillColor, f);
        WriteColor("ESP", "skeletonColor", Vars::ESP::skeletonColor, f);
        WriteColor("ESP", "headDotColor", Vars::ESP::headDotColor, f);
        WriteColor("ESP", "snaplineColor", Vars::ESP::snaplineColor, f);
        WriteColor("ESP", "crosshairColor", Vars::ESP::crosshairColor, f);
        WriteColor("ESP", "targetHighlightColor", Vars::ESP::targetHighlightColor, f);
        WriteColor("ESP", "viewAngleColor", Vars::ESP::viewAngleColor, f);
        WriteColor("ESP", "arrowColor", Vars::ESP::arrowColor, f);

        WriteBool("Local", "speedEnabled", Vars::Local::speedEnabled, f);
        WriteFloat("Local", "walkSpeed", Vars::Local::walkSpeed, f);
        WriteBool("Local", "jumpEnabled", Vars::Local::jumpEnabled, f);
        WriteFloat("Local", "jumpPower", Vars::Local::jumpPower, f);
        WriteBool("Local", "fovChangerEnabled", Vars::Local::fovChangerEnabled, f);
        WriteFloat("Local", "cameraFOV", Vars::Local::cameraFOV, f);

        if (!specificName.empty()) RefreshConfigs();
    }

    inline void Load() {
        std::string f = GetCurrentPath();
        Vars::showHUD = ReadBool("Global", "showHUD", Vars::showHUD, f);
        Vars::showWatermark = ReadBool("Global", "showWatermark", Vars::showWatermark, f);
        Vars::showPlayerList = ReadBool("Global", "showPlayerList", Vars::showPlayerList, f);
        Vars::Misc::streamProof = ReadBool("Misc", "streamProof", Vars::Misc::streamProof, f);

        Vars::Radar::enabled = ReadBool("Radar", "enabled", Vars::Radar::enabled, f);
        Vars::Radar::range = ReadFloat("Radar", "range", Vars::Radar::range, f);
        Vars::Radar::size = ReadFloat("Radar", "size", Vars::Radar::size, f);
        Vars::Radar::blipSize = ReadFloat("Radar", "blipSize", Vars::Radar::blipSize, f);
        ReadColor("Radar", "color", Vars::Radar::color, f);

        Vars::Aimbot::enabled = ReadBool("Aimbot", "enabled", Vars::Aimbot::enabled, f);
        Vars::Aimbot::teamCheck = ReadBool("Aimbot", "teamCheck", Vars::Aimbot::teamCheck, f);
        Vars::Aimbot::targetNPCs = ReadBool("Aimbot", "targetNPCs", Vars::Aimbot::targetNPCs, f);
        Vars::Aimbot::showFOV = ReadBool("Aimbot", "showFOV", Vars::Aimbot::showFOV, f);
        Vars::Aimbot::dynamicFOV = ReadBool("Aimbot", "dynamicFOV", Vars::Aimbot::dynamicFOV, f);
        Vars::Aimbot::drawTargetLine = ReadBool("Aimbot", "drawTargetLine", Vars::Aimbot::drawTargetLine, f);
        Vars::Aimbot::prediction = ReadBool("Aimbot", "prediction", Vars::Aimbot::prediction, f);
        Vars::Aimbot::bulletSpeed = ReadFloat("Aimbot", "bulletSpeed", Vars::Aimbot::bulletSpeed, f);
        Vars::Aimbot::fovRadius = ReadFloat("Aimbot", "fovRadius", Vars::Aimbot::fovRadius, f);
        Vars::Aimbot::smoothing = ReadFloat("Aimbot", "smoothing", Vars::Aimbot::smoothing, f);
        Vars::Aimbot::aimTarget = ReadInt("Aimbot", "aimTarget", Vars::Aimbot::aimTarget, f);
        Vars::Aimbot::aimMethod = ReadInt("Aimbot", "aimMethod", Vars::Aimbot::aimMethod, f);
        Vars::Aimbot::aimbotKey = ReadInt("Aimbot", "aimbotKey", Vars::Aimbot::aimbotKey, f);
        Vars::Aimbot::fovThickness = ReadFloat("Aimbot", "fovThickness", Vars::Aimbot::fovThickness, f);
        ReadColor("Aimbot", "fovColor", Vars::Aimbot::fovColor, f);

        Vars::TriggerBot::enabled = ReadBool("TriggerBot", "enabled", Vars::TriggerBot::enabled, f);
        Vars::TriggerBot::triggerKey = ReadInt("TriggerBot", "triggerKey", Vars::TriggerBot::triggerKey, f);
        Vars::TriggerBot::triggerDistance = ReadFloat("TriggerBot", "triggerDistance", Vars::TriggerBot::triggerDistance, f);
        Vars::TriggerBot::clickDelay = ReadInt("TriggerBot", "clickDelay", Vars::TriggerBot::clickDelay, f);

        Vars::AutoClicker::enabled = ReadBool("AutoClicker", "enabled", Vars::AutoClicker::enabled, f);
        Vars::AutoClicker::clickKey = ReadInt("AutoClicker", "clickKey", Vars::AutoClicker::clickKey, f);
        Vars::AutoClicker::minCPS = ReadInt("AutoClicker", "minCPS", Vars::AutoClicker::minCPS, f);
        Vars::AutoClicker::maxCPS = ReadInt("AutoClicker", "maxCPS", Vars::AutoClicker::maxCPS, f);

        Vars::ESP::enabled = ReadBool("ESP", "enabled", Vars::ESP::enabled, f);
        Vars::ESP::teamCheck = ReadBool("ESP", "teamCheck", Vars::ESP::teamCheck, f);
        Vars::ESP::showNPCs = ReadBool("ESP", "showNPCs", Vars::ESP::showNPCs, f);
        Vars::ESP::items = ReadBool("ESP", "items", Vars::ESP::items, f);
        Vars::ESP::maxItemDistance = ReadFloat("ESP", "maxItemDistance", Vars::ESP::maxItemDistance, f);
        ReadColor("ESP", "itemColor", Vars::ESP::itemColor, f);
        Vars::ESP::highlightTarget = ReadBool("ESP", "highlightTarget", Vars::ESP::highlightTarget, f);
        Vars::ESP::viewAngles = ReadBool("ESP", "viewAngles", Vars::ESP::viewAngles, f);
        Vars::ESP::offScreenArrows = ReadBool("ESP", "offScreenArrows", Vars::ESP::offScreenArrows, f);
        Vars::ESP::arrowRadius = ReadFloat("ESP", "arrowRadius", Vars::ESP::arrowRadius, f);
        Vars::ESP::arrowSize = ReadFloat("ESP", "arrowSize", Vars::ESP::arrowSize, f);
        Vars::ESP::boxes = ReadBool("ESP", "boxes", Vars::ESP::boxes, f);
        Vars::ESP::boxStyle = ReadInt("ESP", "boxStyle", Vars::ESP::boxStyle, f);
        Vars::ESP::boxFill = ReadBool("ESP", "boxFill", Vars::ESP::boxFill, f);
        Vars::ESP::skeleton = ReadBool("ESP", "skeleton", Vars::ESP::skeleton, f);
        Vars::ESP::skeletonThickness = ReadFloat("ESP", "skeletonThickness", Vars::ESP::skeletonThickness, f);
        Vars::ESP::headDot = ReadBool("ESP", "headDot", Vars::ESP::headDot, f);
        Vars::ESP::headDotSize = ReadFloat("ESP", "headDotSize", Vars::ESP::headDotSize, f);
        Vars::ESP::names = ReadBool("ESP", "names", Vars::ESP::names, f);
        Vars::ESP::distance = ReadBool("ESP", "distance", Vars::ESP::distance, f);
        Vars::ESP::weapon = ReadBool("ESP", "weapon", Vars::ESP::weapon, f);
        Vars::ESP::healthBar = ReadBool("ESP", "healthBar", Vars::ESP::healthBar, f);
        Vars::ESP::healthText = ReadBool("ESP", "healthText", Vars::ESP::healthText, f);
        Vars::ESP::textBackground = ReadBool("ESP", "textBackground", Vars::ESP::textBackground, f);
        Vars::ESP::snaplines = ReadBool("ESP", "snaplines", Vars::ESP::snaplines, f);
        Vars::ESP::snaplinePos = ReadInt("ESP", "snaplinePos", Vars::ESP::snaplinePos, f);
        Vars::ESP::crosshair = ReadBool("ESP", "crosshair", Vars::ESP::crosshair, f);
        Vars::ESP::crosshairSize = ReadFloat("ESP", "crosshairSize", Vars::ESP::crosshairSize, f);
        Vars::ESP::crosshairThickness = ReadFloat("ESP", "crosshairThickness", Vars::ESP::crosshairThickness, f);
        Vars::ESP::maxDistance = ReadFloat("ESP", "maxDistance", Vars::ESP::maxDistance, f);

        ReadColor("ESP", "boxColor", Vars::ESP::boxColor, f);
        ReadColor("ESP", "boxFillColor", Vars::ESP::boxFillColor, f);
        ReadColor("ESP", "skeletonColor", Vars::ESP::skeletonColor, f);
        ReadColor("ESP", "headDotColor", Vars::ESP::headDotColor, f);
        ReadColor("ESP", "snaplineColor", Vars::ESP::snaplineColor, f);
        ReadColor("ESP", "crosshairColor", Vars::ESP::crosshairColor, f);
        ReadColor("ESP", "targetHighlightColor", Vars::ESP::targetHighlightColor, f);
        ReadColor("ESP", "viewAngleColor", Vars::ESP::viewAngleColor, f);
        ReadColor("ESP", "arrowColor", Vars::ESP::arrowColor, f);

        Vars::Local::speedEnabled = ReadBool("Local", "speedEnabled", Vars::Local::speedEnabled, f);
        Vars::Local::walkSpeed = ReadFloat("Local", "walkSpeed", Vars::Local::walkSpeed, f);
        Vars::Local::jumpEnabled = ReadBool("Local", "jumpEnabled", Vars::Local::jumpEnabled, f);
        Vars::Local::jumpPower = ReadFloat("Local", "jumpPower", Vars::Local::jumpPower, f);
        Vars::Local::fovChangerEnabled = ReadBool("Local", "fovChangerEnabled", Vars::Local::fovChangerEnabled, f);
        Vars::Local::cameraFOV = ReadFloat("Local", "cameraFOV", Vars::Local::cameraFOV, f);
    }
}