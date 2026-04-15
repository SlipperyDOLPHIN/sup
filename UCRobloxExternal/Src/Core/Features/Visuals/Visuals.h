#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Vars/Vars.h"
#include "../../../Core/Features/Aimbot/Aimbot.h" 
#include "../../../Render/ImGui/imgui.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace Visuals {

    inline void DrawOutlinedText(ImDrawList* drawList, const ImVec2& pos, const std::string& text, ImU32 textColor, bool drawBg) {
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

        if (drawBg) {
            drawList->AddRectFilled(
                ImVec2(pos.x - 2.0f, pos.y - 1.0f),
                ImVec2(pos.x + textSize.x + 2.0f, pos.y + textSize.y + 1.0f),
                IM_COL32(10, 10, 10, 180), 3.0f
            );
        }

        drawList->AddText(ImVec2(pos.x - 1.0f, pos.y), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x + 1.0f, pos.y), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y - 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y + 1.0f), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(pos, textColor, text.c_str());
    }

    inline void RenderESP(ImDrawList* drawList, const RBX::Mat4& viewMatrix)
    {
        if (!Vars::ESP::enabled) return;

        ImU32 baseBoxCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::boxColor[0], Vars::ESP::boxColor[1], Vars::ESP::boxColor[2], Vars::ESP::boxColor[3]));
        ImU32 baseSkelCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::skeletonColor[0], Vars::ESP::skeletonColor[1], Vars::ESP::skeletonColor[2], Vars::ESP::skeletonColor[3]));
        ImU32 boxFillCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::boxFillColor[0], Vars::ESP::boxFillColor[1], Vars::ESP::boxFillColor[2], Vars::ESP::boxFillColor[3]));
        ImU32 headDotCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::headDotColor[0], Vars::ESP::headDotColor[1], Vars::ESP::headDotColor[2], Vars::ESP::headDotColor[3]));
        ImU32 snapCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::snaplineColor[0], Vars::ESP::snaplineColor[1], Vars::ESP::snaplineColor[2], Vars::ESP::snaplineColor[3]));
        ImU32 targetHighCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::targetHighlightColor[0], Vars::ESP::targetHighlightColor[1], Vars::ESP::targetHighlightColor[2], Vars::ESP::targetHighlightColor[3]));
        ImU32 viewAngCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::viewAngleColor[0], Vars::ESP::viewAngleColor[1], Vars::ESP::viewAngleColor[2], Vars::ESP::viewAngleColor[3]));
        ImU32 arrCol = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::arrowColor[0], Vars::ESP::arrowColor[1], Vars::ESP::arrowColor[2], Vars::ESP::arrowColor[3]));

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 screenCenter = ImVec2(screenSize.x / 2.0f, screenSize.y / 2.0f);

        for (auto& plr : PlayerCache::players) {
            if (!plr.isValid) continue;
            if (plr.distance > Vars::ESP::maxDistance) continue;
            if (plr.isNPC && !Vars::ESP::showNPCs) continue;
            if (!plr.isNPC && Vars::ESP::teamCheck && plr.teamAddr == PlayerCache::localPlayerTeam && plr.teamAddr != 0) continue;

            bool isTarget = (Vars::ESP::highlightTarget && plr.playerAddr == Aimbot::lockedPlayerAddr);
            ImU32 boxCol = isTarget ? targetHighCol : baseBoxCol;
            ImU32 skelCol = isTarget ? targetHighCol : baseSkelCol;

            if (!isTarget && Vars::ESP::healthColoring && Vars::ESP::healthBar) {
                float hpPerc = std::clamp(plr.health / plr.maxHealth, 0.0f, 1.0f);
                boxCol = IM_COL32(255 * (1.0f - hpPerc), 255 * hpPerc, 0, 255);
                skelCol = boxCol;
            }

            auto character = RBX::RbxInstance(plr.characterAddr);
            auto head = character.FindChild("Head");
            auto torso = character.FindChild("Torso");
            auto leftArm = character.FindChild("Left Arm");
            auto rightArm = character.FindChild("Right Arm");
            auto leftLeg = character.FindChild("Left Leg");
            auto rightLeg = character.FindChild("Right Leg");

            bool isR6 = (torso.Addr != 0);
            if (!isR6) {
                head = character.FindChild("Head");
                torso = character.FindChild("UpperTorso");
                leftArm = character.FindChild("LeftHand");
                rightArm = character.FindChild("RightHand");
                leftLeg = character.FindChild("LeftFoot");
                rightLeg = character.FindChild("RightFoot");
            }

            if (head.Addr == 0) continue;

            RBX::Vec2 screenPos = W2S::WorldToScreen(plr.position, viewMatrix);

            if (Vars::ESP::offScreenArrows) {
                float clipX = plr.position.X * viewMatrix.data[0] + plr.position.Y * viewMatrix.data[4] + plr.position.Z * viewMatrix.data[8] + viewMatrix.data[12];
                float clipY = plr.position.X * viewMatrix.data[1] + plr.position.Y * viewMatrix.data[5] + plr.position.Z * viewMatrix.data[9] + viewMatrix.data[13];
                float clipW = plr.position.X * viewMatrix.data[3] + plr.position.Y * viewMatrix.data[7] + plr.position.Z * viewMatrix.data[11] + viewMatrix.data[15];

                bool isOffScreen = (screenPos.X <= 0 || screenPos.Y <= 0 || screenPos.X >= screenSize.x || screenPos.Y >= screenSize.y || clipW < 0.1f);

                if (isOffScreen) {
                    RBX::Vec2 arrowScreenPos = screenPos;
                    if (clipW < 0.1f) {
                        clipX *= -1.0f;
                        clipY *= -1.0f;
                    }

                    float angle = atan2f(-clipY, clipX);
                    float radius = Vars::ESP::arrowRadius;
                    float arrSize = Vars::ESP::arrowSize;
                    float arrWidth = arrSize * 0.6f;

                    ImVec2 p1 = ImVec2(screenCenter.x + cosf(angle) * radius, screenCenter.y + sinf(angle) * radius);
                    
                    // Clamp to screen edges if radius is too large
                    p1.x = std::clamp(p1.x, 20.0f, screenSize.x - 20.0f);
                    p1.y = std::clamp(p1.y, 20.0f, screenSize.y - 20.0f);

                    ImVec2 dir = ImVec2(cosf(angle), sinf(angle));
                    ImVec2 norm = ImVec2(-sinf(angle), cosf(angle));

                    ImVec2 base = ImVec2(p1.x - dir.x * arrSize, p1.y - dir.y * arrSize);
                    ImVec2 p2 = ImVec2(base.x - norm.x * arrWidth, base.y - norm.y * arrWidth);
                    ImVec2 p3 = ImVec2(base.x + norm.x * arrWidth, base.y + norm.y * arrWidth);

                    drawList->AddTriangleFilled(p1, p2, p3, arrCol);
                    drawList->AddTriangle(p1, p2, p3, IM_COL32(0, 0, 0, 255), 1.0f);
                }
            }

            if (Vars::ESP::viewAngles) {
                auto headCF = head.GetCFrame();
                auto lookVec = headCF.GetLookVector();
                auto headPos = head.GetPos();

                RBX::Vec3 endPos = {
                    headPos.X + lookVec.X * Vars::ESP::viewAngleLength,
                    headPos.Y + lookVec.Y * Vars::ESP::viewAngleLength,
                    headPos.Z + lookVec.Z * Vars::ESP::viewAngleLength
                };

                RBX::Vec2 head2D = W2S::WorldToScreen(headPos, viewMatrix);
                RBX::Vec2 end2D = W2S::WorldToScreen(endPos, viewMatrix);

                if (head2D.X != 0 && end2D.X != 0) {
                    drawList->AddLine(ImVec2(head2D.X, head2D.Y), ImVec2(end2D.X, end2D.Y), IM_COL32(0, 0, 0, 255), 3.0f);
                    drawList->AddLine(ImVec2(head2D.X, head2D.Y), ImVec2(end2D.X, end2D.Y), viewAngCol, 1.5f);
                }
            }

            RBX::Vec3 boundPoints[200];
            int boundPointCount = 0;

            if (isR6) {
                auto headPos = head.GetPos();
                const float headSize = 1.0f;
                for (int i = 0; i < 8; i++) {
                    float angle = (i / 8.0f) * 6.28318f;
                    boundPoints[boundPointCount++] = { headPos.X + cosf(angle) * headSize, headPos.Y, headPos.Z + sinf(angle) * headSize };
                }
                boundPoints[boundPointCount++] = { headPos.X, headPos.Y + headSize, headPos.Z };
                boundPoints[boundPointCount++] = { headPos.X, headPos.Y - headSize * 0.5f, headPos.Z };

                if (torso.Addr != 0) {
                    auto torsoCF = torso.GetCFrame();
                    auto torsoPos = torsoCF.GetPosition();
                    auto torsoRight = torsoCF.GetRightVector();
                    auto torsoUp = torsoCF.GetUpVector();
                    auto torsoLook = torsoCF.GetLookVector();
                    const float torsoW = 2.0f, torsoH = 2.0f, torsoD = 1.0f;

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    torsoPos.X + torsoRight.X * (x * torsoW * 0.5f) + torsoUp.X * (y * torsoH * 0.5f) + torsoLook.X * (z * torsoD * 0.5f),
                                    torsoPos.Y + torsoRight.Y * (x * torsoW * 0.5f) + torsoUp.Y * (y * torsoH * 0.5f) + torsoLook.Y * (z * torsoD * 0.5f),
                                    torsoPos.Z + torsoRight.Z * (x * torsoW * 0.5f) + torsoUp.Z * (y * torsoH * 0.5f) + torsoLook.Z * (z * torsoD * 0.5f)
                                };
                            }
                        }
                    }
                }

                const float armW = 1.0f, armH = 2.0f, armD = 1.0f;
                if (rightArm.Addr != 0) {
                    auto armCF = rightArm.GetCFrame();
                    auto armPos = armCF.GetPosition();
                    auto armRight = armCF.GetRightVector();
                    auto armUp = armCF.GetUpVector();
                    auto armLook = armCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    armPos.X + armRight.X * (x * armW * 0.5f) + armUp.X * (y * armH * 0.5f) + armLook.X * (z * armD * 0.5f),
                                    armPos.Y + armRight.Y * (x * armW * 0.5f) + armUp.Y * (y * armH * 0.5f) + armLook.Y * (z * armD * 0.5f),
                                    armPos.Z + armRight.Z * (x * armW * 0.5f) + armUp.Z * (y * armH * 0.5f) + armLook.Z * (z * armD * 0.5f)
                                };
                            }
                        }
                    }
                }

                if (leftArm.Addr != 0) {
                    auto armCF = leftArm.GetCFrame();
                    auto armPos = armCF.GetPosition();
                    auto armRight = armCF.GetRightVector();
                    auto armUp = armCF.GetUpVector();
                    auto armLook = armCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    armPos.X + armRight.X * (x * armW * 0.5f) + armUp.X * (y * armH * 0.5f) + armLook.X * (z * armD * 0.5f),
                                    armPos.Y + armRight.Y * (x * armW * 0.5f) + armUp.Y * (y * armH * 0.5f) + armLook.Y * (z * armD * 0.5f),
                                    armPos.Z + armRight.Z * (x * armW * 0.5f) + armUp.Z * (y * armH * 0.5f) + armLook.Z * (z * armD * 0.5f)
                                };
                            }
                        }
                    }
                }

                const float legW = 1.0f, legH = 2.0f, legD = 1.0f;
                if (rightLeg.Addr != 0) {
                    auto legCF = rightLeg.GetCFrame();
                    auto legPos = legCF.GetPosition();
                    auto legRight = legCF.GetRightVector();
                    auto legUp = legCF.GetUpVector();
                    auto legLook = legCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    legPos.X + legRight.X * (x * legW * 0.5f) + legUp.X * (y * legH * 0.6f) + legLook.X * (z * legD * 0.5f),
                                    legPos.Y + legRight.Y * (x * legW * 0.5f) + legUp.Y * (y * legH * 0.6f) + legLook.Y * (z * legD * 0.5f),
                                    legPos.Z + legRight.Z * (x * legW * 0.5f) + legUp.Z * (y * legH * 0.6f) + legLook.Z * (z * legD * 0.5f)
                                };
                            }
                        }
                    }
                }

                if (leftLeg.Addr != 0) {
                    auto legCF = leftLeg.GetCFrame();
                    auto legPos = legCF.GetPosition();
                    auto legRight = legCF.GetRightVector();
                    auto legUp = legCF.GetUpVector();
                    auto legLook = legCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    legPos.X + legRight.X * (x * legW * 0.5f) + legUp.X * (y * legH * 0.6f) + legLook.X * (z * legD * 0.5f),
                                    legPos.Y + legRight.Y * (x * legW * 0.5f) + legUp.Y * (y * legH * 0.6f) + legLook.Y * (z * legD * 0.5f),
                                    legPos.Z + legRight.Z * (x * legW * 0.5f) + legUp.Z * (y * legH * 0.6f) + legLook.Z * (z * legD * 0.5f)
                                };
                            }
                        }
                    }
                }
            }
            else {
                auto headPos = head.GetPos();
                const float headSize = 0.8f;
                for (int i = 0; i < 8; i++) {
                    float angle = (i / 8.0f) * 6.28318f;
                    boundPoints[boundPointCount++] = { headPos.X + cosf(angle) * headSize, headPos.Y, headPos.Z + sinf(angle) * headSize };
                }
                boundPoints[boundPointCount++] = { headPos.X, headPos.Y + headSize, headPos.Z };
                boundPoints[boundPointCount++] = { headPos.X, headPos.Y - headSize * 0.5f, headPos.Z };

                if (torso.Addr != 0) {
                    auto torsoCF = torso.GetCFrame();
                    auto torsoPos = torsoCF.GetPosition();
                    auto torsoRight = torsoCF.GetRightVector();
                    auto torsoUp = torsoCF.GetUpVector();
                    auto torsoLook = torsoCF.GetLookVector();
                    const float utW = 1.6f, utH = 1.5f, utD = 0.8f;

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    torsoPos.X + torsoRight.X * (x * utW * 0.5f) + torsoUp.X * (y * utH * 0.5f) + torsoLook.X * (z * utD * 0.5f),
                                    torsoPos.Y + torsoRight.Y * (x * utW * 0.5f) + torsoUp.Y * (y * utH * 0.5f) + torsoLook.Y * (z * utD * 0.5f),
                                    torsoPos.Z + torsoRight.Z * (x * utW * 0.5f) + torsoUp.Z * (y * utH * 0.5f) + torsoLook.Z * (z * utD * 0.5f)
                                };
                            }
                        }
                    }
                }

                // [FIXED] Replaced handW with armW for R15 Arms
                const float armW = 0.8f, armH = 0.8f, armD = 0.8f;
                if (rightArm.Addr != 0) {
                    auto armCF = rightArm.GetCFrame();
                    auto armPos = armCF.GetPosition();
                    auto armRight = armCF.GetRightVector();
                    auto armUp = armCF.GetUpVector();
                    auto armLook = armCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    armPos.X + armRight.X * (x * armW * 0.5f) + armUp.X * (y * armH * 0.5f) + armLook.X * (z * armD * 0.5f),
                                    armPos.Y + armRight.Y * (x * armW * 0.5f) + armUp.Y * (y * armH * 0.5f) + armLook.Y * (z * armD * 0.5f),
                                    armPos.Z + armRight.Z * (x * armW * 0.5f) + armUp.Z * (y * armH * 0.5f) + armLook.Z * (z * armD * 0.5f)
                                };
                            }
                        }
                    }
                }

                if (leftArm.Addr != 0) {
                    auto armCF = leftArm.GetCFrame();
                    auto armPos = armCF.GetPosition();
                    auto armRight = armCF.GetRightVector();
                    auto armUp = armCF.GetUpVector();
                    auto armLook = armCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    armPos.X + armRight.X * (x * armW * 0.5f) + armUp.X * (y * armH * 0.5f) + armLook.X * (z * armD * 0.5f),
                                    armPos.Y + armRight.Y * (x * armW * 0.5f) + armUp.Y * (y * armH * 0.5f) + armLook.Y * (z * armD * 0.5f),
                                    armPos.Z + armRight.Z * (x * armW * 0.5f) + armUp.Z * (y * armH * 0.5f) + armLook.Z * (z * armD * 0.5f)
                                };
                            }
                        }
                    }
                }

                // [FIXED] Replaced footW with legW for R15 Legs
                const float legW = 0.8f, legH = 0.7f, legD = 1.2f;
                if (rightLeg.Addr != 0) {
                    auto legCF = rightLeg.GetCFrame();
                    auto legPos = legCF.GetPosition();
                    auto legRight = legCF.GetRightVector();
                    auto legUp = legCF.GetUpVector();
                    auto legLook = legCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    legPos.X + legRight.X * (x * legW * 0.5f) + legUp.X * (y * legH * 0.5f) + legLook.X * (z * legD * 0.5f),
                                    legPos.Y + legRight.Y * (x * legW * 0.5f) + legUp.Y * (y * legH * 0.5f) + legLook.Y * (z * legD * 0.5f),
                                    legPos.Z + legRight.Z * (x * legW * 0.5f) + legUp.Z * (y * legH * 0.5f) + legLook.Z * (z * legD * 0.5f)
                                };
                            }
                        }
                    }
                }

                if (leftLeg.Addr != 0) {
                    auto legCF = leftLeg.GetCFrame();
                    auto legPos = legCF.GetPosition();
                    auto legRight = legCF.GetRightVector();
                    auto legUp = legCF.GetUpVector();
                    auto legLook = legCF.GetLookVector();

                    for (int x = -1; x <= 1; x += 2) {
                        for (int y = -1; y <= 1; y += 2) {
                            for (int z = -1; z <= 1; z += 2) {
                                boundPoints[boundPointCount++] = {
                                    legPos.X + legRight.X * (x * legW * 0.5f) + legUp.X * (y * legH * 0.5f) + legLook.X * (z * legD * 0.5f),
                                    legPos.Y + legRight.Y * (x * legW * 0.5f) + legUp.Y * (y * legH * 0.5f) + legLook.Y * (z * legD * 0.5f),
                                    legPos.Z + legRight.Z * (x * legW * 0.5f) + legUp.Z * (y * legH * 0.5f) + legLook.Z * (z * legD * 0.5f)
                                };
                            }
                        }
                    }
                }
            }

            float minX = 999999.0f, minY = 999999.0f;
            float maxX = -999999.0f, maxY = -999999.0f;
            int validPoints = 0;

            for (int i = 0; i < boundPointCount; i++) {
                RBX::Vec2 screenPosBound = W2S::WorldToScreen(boundPoints[i], viewMatrix);
                if (screenPosBound.X != 0 || screenPosBound.Y != 0) {
                    minX = (std::min)(minX, screenPosBound.X);
                    minY = (std::min)(minY, screenPosBound.Y);
                    maxX = (std::max)(maxX, screenPosBound.X);
                    maxY = (std::max)(maxY, screenPosBound.Y);
                    validPoints++;
                }
            }

            if (validPoints < 3 || minX == 999999.0f) continue;

            float boxWidth = maxX - minX;
            float boxHeight = maxY - minY;
            const float MAX_BOX_WIDTH = screenSize.x * 1.5f;
            const float MAX_BOX_HEIGHT = screenSize.y * 1.5f;

            if (boxWidth > MAX_BOX_WIDTH || boxHeight > MAX_BOX_HEIGHT) continue;

            if (Vars::ESP::boxes && Vars::ESP::boxFill) {
                drawList->AddRectFilled(ImVec2(minX, minY), ImVec2(maxX, maxY), boxFillCol);
            }

            if (Vars::ESP::boxes) {
                if (Vars::ESP::boxStyle == 0) {
                    drawList->AddRect(ImVec2(minX - 1.0f, minY - 1.0f), ImVec2(maxX + 1.0f, maxY + 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
                    drawList->AddRect(ImVec2(minX + 1.0f, minY + 1.0f), ImVec2(maxX - 1.0f, maxY - 1.0f), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
                    drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), boxCol, 0.0f, 0, 1.0f);
                }
                else if (Vars::ESP::boxStyle == 1) {
                    float lineW = (maxX - minX) / 4.0f;
                    float lineH = (maxY - minY) / 4.0f;
                    float t = 1.0f;

                    auto DrawCorner = [&](ImVec2 p1, ImVec2 p2) {
                        drawList->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), t + 2.0f);
                        drawList->AddLine(p1, p2, boxCol, t);
                        };

                    DrawCorner(ImVec2(minX, minY), ImVec2(minX + lineW, minY));
                    DrawCorner(ImVec2(minX, minY), ImVec2(minX, minY + lineH));
                    DrawCorner(ImVec2(maxX, minY), ImVec2(maxX - lineW, minY));
                    DrawCorner(ImVec2(maxX, minY), ImVec2(maxX, minY + lineH));
                    DrawCorner(ImVec2(minX, maxY), ImVec2(minX + lineW, maxY));
                    DrawCorner(ImVec2(minX, maxY), ImVec2(minX, maxY - lineH));
                    DrawCorner(ImVec2(maxX, maxY), ImVec2(maxX - lineW, maxY));
                    DrawCorner(ImVec2(maxX, maxY), ImVec2(maxX, maxY - lineH));
                }
            }

            if (Vars::ESP::headDot) {
                RBX::Vec2 head2D = W2S::WorldToScreen(head.GetPos(), viewMatrix);
                if (head2D.X != 0 && head2D.Y != 0) {
                    drawList->AddCircleFilled(ImVec2(head2D.X, head2D.Y), Vars::ESP::headDotSize + 1.0f, IM_COL32(0, 0, 0, 255));
                    drawList->AddCircleFilled(ImVec2(head2D.X, head2D.Y), Vars::ESP::headDotSize, headDotCol);
                }
            }

            if (Vars::ESP::skeleton) {
                RBX::Vec2 head2D = W2S::WorldToScreen(head.GetPos(), viewMatrix);
                RBX::Vec2 torso2D = W2S::WorldToScreen(torso.GetPos(), viewMatrix);
                
                if (head2D.X != 0 && torso2D.X != 0) {
                    auto DrawBone = [&](RBX::RbxInstance a, RBX::RbxInstance b) {
                        RBX::Vec2 p1 = W2S::WorldToScreen(a.GetPos(), viewMatrix);
                        RBX::Vec2 p2 = W2S::WorldToScreen(b.GetPos(), viewMatrix);
                        if (p1.X != 0 && p2.X != 0) {
                            drawList->AddLine(ImVec2(p1.X, p1.Y), ImVec2(p2.X, p2.Y), IM_COL32(0, 0, 0, 255), Vars::ESP::skeletonThickness + 2.0f);
                            drawList->AddLine(ImVec2(p1.X, p1.Y), ImVec2(p2.X, p2.Y), skelCol, Vars::ESP::skeletonThickness);
                        }
                    };

                    DrawBone(head, torso);

                    if (isR6) {
                        if (leftArm.Addr != 0) DrawBone(torso, leftArm);
                        if (rightArm.Addr != 0) DrawBone(torso, rightArm);
                        if (leftLeg.Addr != 0) DrawBone(torso, leftLeg);
                        if (rightLeg.Addr != 0) DrawBone(torso, rightLeg);
                    }
                    else {
                        auto lowerTorso = character.FindChild("LowerTorso");
                        auto lUA = character.FindChild("LeftUpperArm");
                        auto lLA = character.FindChild("LeftLowerArm");
                        auto lH = character.FindChild("LeftHand");
                        auto rUA = character.FindChild("RightUpperArm");
                        auto rLA = character.FindChild("RightLowerArm");
                        auto rH = character.FindChild("RightHand");
                        auto lUL = character.FindChild("LeftUpperLeg");
                        auto lLL = character.FindChild("LeftLowerLeg");
                        auto lF = character.FindChild("LeftFoot");
                        auto rUL = character.FindChild("RightUpperLeg");
                        auto rLL = character.FindChild("RightLowerLeg");
                        auto rF = character.FindChild("RightFoot");

                        if (lowerTorso.Addr != 0) {
                            DrawBone(torso, lowerTorso);
                            if (lUL.Addr != 0) { DrawBone(lowerTorso, lUL); DrawBone(lUL, lLL); DrawBone(lLL, lF); }
                            if (rUL.Addr != 0) { DrawBone(lowerTorso, rUL); DrawBone(rUL, rLL); DrawBone(rLL, rF); }
                        }
                        if (lUA.Addr != 0) { DrawBone(torso, lUA); DrawBone(lUA, lLA); DrawBone(lLA, lH); }
                        if (rUA.Addr != 0) { DrawBone(torso, rUA); DrawBone(rUA, rLA); DrawBone(rLA, rH); }
                    }
                }
            }

            if (Vars::ESP::healthBar && plr.maxHealth > 0) {
                float healthPercent = static_cast<float>(plr.health) / static_cast<float>(plr.maxHealth);
                ImU32 healthColor = IM_COL32(255 * (1.0f - healthPercent), 255 * healthPercent, 0, 255);

                float barHeight = (maxY - minY) * healthPercent;

                drawList->AddRectFilled(ImVec2(minX - 6.0f, minY - 1.0f), ImVec2(minX - 2.0f, maxY + 1.0f), IM_COL32(0, 0, 0, 255));
                drawList->AddRectFilled(ImVec2(minX - 5.0f, maxY - barHeight), ImVec2(minX - 3.0f, maxY), healthColor);

                if (Vars::ESP::healthText) {
                    std::string hpStr = std::to_string(static_cast<int>(plr.health)) + " HP";
                    ImVec2 hpSize = ImGui::CalcTextSize(hpStr.c_str());
                    DrawOutlinedText(drawList, ImVec2(minX - 8.0f - hpSize.x, maxY - barHeight - 4.0f), hpStr, IM_COL32(255, 255, 255, 255), Vars::ESP::textBackground);
                }
            }

            if (Vars::ESP::names) {
                ImVec2 textSize = ImGui::CalcTextSize(plr.name.c_str());
                float textX = (minX + maxX) / 2.0f - textSize.x / 2.0f;
                float textY = minY - textSize.y - 2.0f;
                DrawOutlinedText(drawList, ImVec2(textX, textY), plr.name, IM_COL32(255, 255, 255, 255), Vars::ESP::textBackground);
            }

            float bottomTextOffset = 2.0f;
            if (Vars::ESP::distance) {
                std::string distText = std::to_string(static_cast<int>(plr.distance)) + "m";
                ImVec2 textSize = ImGui::CalcTextSize(distText.c_str());
                float textX = (minX + maxX) / 2.0f - textSize.x / 2.0f;
                DrawOutlinedText(drawList, ImVec2(textX, maxY + bottomTextOffset), distText, IM_COL32(200, 200, 200, 255), Vars::ESP::textBackground);
                bottomTextOffset += textSize.y + 2.0f;
            }

            if (Vars::ESP::weapon && !plr.equippedTool.empty()) {
                ImVec2 textSize = ImGui::CalcTextSize(plr.equippedTool.c_str());
                float textX = (minX + maxX) / 2.0f - textSize.x / 2.0f;
                DrawOutlinedText(drawList, ImVec2(textX, maxY + bottomTextOffset), plr.equippedTool, IM_COL32(150, 200, 255, 255), Vars::ESP::textBackground);
            }

            if (Vars::ESP::snaplines) {
                ImVec2 startPos;
                if (Vars::ESP::snaplinePos == 0) startPos = ImVec2(screenSize.x / 2.0f, screenSize.y);
                else if (Vars::ESP::snaplinePos == 1) startPos = ImVec2(screenSize.x / 2.0f, screenSize.y / 2.0f);
                else if (Vars::ESP::snaplinePos == 2) startPos = ImVec2(screenSize.x / 2.0f, 0.0f);

                drawList->AddLine(startPos, ImVec2((minX + maxX) / 2.0f, maxY), snapCol, 1.0f);
            }
        }

        if (Vars::ESP::items) {
            ImU32 itemColor = ImGui::ColorConvertFloat4ToU32(ImVec4(Vars::ESP::itemColor[0], Vars::ESP::itemColor[1], Vars::ESP::itemColor[2], Vars::ESP::itemColor[3]));

            for (auto& item : PlayerCache::items) {
                RBX::Vec2 screenPos = W2S::WorldToScreen(item.position, viewMatrix);
                if (screenPos.X != 0 && screenPos.Y != 0) {
                    std::string text = item.name + " [" + std::to_string((int)item.distance) + "m]";
                    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
                    DrawOutlinedText(drawList, ImVec2(screenPos.X - (textSize.x / 2), screenPos.Y), text, itemColor, Vars::ESP::textBackground);
                }
            }
        }
    }
}