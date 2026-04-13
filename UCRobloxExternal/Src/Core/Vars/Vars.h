#pragma once
#include <string>

namespace Vars {
    inline bool menuOpen = false;
    inline int selectedTab = 0;
    inline bool showHUD = true;

    namespace Aimbot {
        inline bool enabled = false;
        inline bool teamCheck = false;
        inline bool targetNPCs = false;
        inline bool showFOV = false;
        inline bool drawTargetLine = false;
        inline float fovRadius = 100.0f;
        inline float smoothing = 5.0f;
        inline int aimTarget = 0;
        inline int aimMethod = 0;
        inline int aimbotKey = 2;

        inline float fovColor[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
        inline float fovThickness = 1.0f;

        inline std::string currentTargetName = "None";
    }

    namespace TriggerBot {
        inline bool enabled = false;
        inline int triggerKey = 18;
        inline float triggerDistance = 15.0f;
        inline int clickDelay = 50;
    }

    namespace ESP {
        inline bool enabled = false;
        inline bool teamCheck = false;
        inline bool showNPCs = false;
        inline bool boxes = false;
        inline int boxStyle = 0;
        inline bool boxFill = false;
        inline bool skeleton = false;
        inline bool headDot = false;
        inline float headDotSize = 4.0f;
        inline bool names = false;
        inline bool distance = false;
        inline bool weapon = false;
        inline bool healthBar = false;
        inline bool healthText = false;
        inline bool snaplines = false;
        inline int snaplinePos = 0;

        inline bool crosshair = false;
        inline float crosshairSize = 10.0f;
        inline float crosshairThickness = 1.0f;

        inline float maxDistance = 1000.0f;

        inline float boxColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float boxFillColor[4] = { 0.0f, 0.0f, 0.0f, 0.4f };
        inline float skeletonColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float headDotColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        inline float snaplineColor[4] = { 1.0f, 1.0f, 1.0f, 0.6f };
        inline float crosshairColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    namespace Local {
        inline bool speedEnabled = false;
        inline float walkSpeed = 16.0f;
        inline bool jumpEnabled = false;
        inline float jumpPower = 50.0f;

        inline bool fovChangerEnabled = false;
        inline float cameraFOV = 70.0f;
    }

    namespace Misc {
        inline bool streamProof = false;
    }
}