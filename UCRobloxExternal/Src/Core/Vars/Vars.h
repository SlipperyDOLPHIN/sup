#pragma once
#include <string>
#include <vector>

namespace Vars {
    inline bool menuOpen = false;
    inline int selectedTab = 0;
    inline bool showHUD = true;
    inline bool showWatermark = true;
    inline bool showPlayerList = false;

    namespace Configs {
        inline std::vector<std::string> list;
        inline int selectedIndex = 0;
        inline char newConfigName[64] = "";
    }

    namespace Radar {
        inline bool enabled = false;
        inline float range = 250.0f;
        inline float size = 200.0f;
        inline float blipSize = 4.0f;
        inline float color[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
    }

    namespace Aimbot {
        inline bool enabled = false;
        inline bool teamCheck = false;
        inline bool targetNPCs = false;
        inline bool showFOV = false;
        inline bool dynamicFOV = true;
        inline bool drawTargetLine = false;

        inline bool prediction = false;
        inline float bulletSpeed = 1000.0f;

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

    namespace AutoClicker {
        inline bool enabled = false;
        inline int clickKey = 0;
        inline int minCPS = 10;
        inline int maxCPS = 14;
    }

    namespace ESP {
        inline bool enabled = false;
        inline bool teamCheck = false;
        inline bool showNPCs = false;

        inline bool items = false;
        inline float maxItemDistance = 300.0f;
        inline float itemColor[4] = { 0.2f, 1.0f, 0.8f, 1.0f };

        inline bool highlightTarget = true;
        inline float targetHighlightColor[4] = { 1.0f, 0.8f, 0.0f, 1.0f };

        inline bool viewAngles = false;
        inline float viewAngleLength = 5.0f;
        inline float viewAngleColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };

        inline bool offScreenArrows = false;
        inline float arrowRadius = 150.0f;
        inline float arrowSize = 8.0f; // [FIXED] Much smaller, sharper arrows
        inline float arrowColor[4] = { 1.0f, 0.2f, 0.2f, 1.0f };

        inline bool boxes = false;
        inline int boxStyle = 0;
        inline bool boxFill = false;
        inline bool skeleton = false;
        inline float skeletonThickness = 1.5f;
        inline bool headDot = false;
        inline float headDotSize = 4.0f;

        inline bool names = false;
        inline bool distance = false;
        inline bool weapon = false;
        inline bool healthBar = false;
        inline bool healthText = false;
        inline bool textBackground = false;

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
        inline bool forceRefresh = false;
        inline bool exitCheat = false;
    }
}