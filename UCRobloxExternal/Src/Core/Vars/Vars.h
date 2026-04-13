#pragma once

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
        inline int aimMethod = 0; // [NEW] 0 = Crosshair, 1 = Distance
        inline int aimbotKey = 2;

        inline float fovColor[4] = { 1.0f, 1.0f, 1.0f, 0.8f };
        inline float fovThickness = 1.0f;
    }

    namespace ESP {
        inline bool enabled = false;
        inline bool teamCheck = false;
        inline bool showNPCs = false;
        inline bool boxes = false;
        inline int boxStyle = 0;
        inline bool boxFill = false; // [NEW]
        inline bool skeleton = false;
        inline bool names = false;
        inline bool distance = false;
        inline bool weapon = false;  // [NEW]
        inline bool healthBar = false;
        inline bool healthText = false; // [NEW]
        inline bool snaplines = false;
        inline int snaplinePos = 0;
        inline bool crosshair = false;
        inline float maxDistance = 1000.0f;

        inline float boxColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float boxFillColor[4] = { 0.0f, 0.0f, 0.0f, 0.4f }; // [NEW] Transparent black by default
        inline float skeletonColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float snaplineColor[4] = { 1.0f, 1.0f, 1.0f, 0.6f };
    }

    namespace Local {
        inline bool speedEnabled = false;
        inline float walkSpeed = 16.0f;
        inline bool jumpEnabled = false;
        inline float jumpPower = 50.0f;

        inline bool fovChangerEnabled = false;
        inline float cameraFOV = 70.0f;
    }
}