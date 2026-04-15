#pragma once
#include "../Offsets/Offsets.h"
#include "../../Memory/Communication.h"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <thread>
#include <chrono>

namespace RBX {

    struct Vec2 {
        float X{ 0 };
        float Y{ 0 };
    };

    struct Vec3 {
        float X{ 0 };
        float Y{ 0 };

        float Z{ 0 };
    };

    struct Vec4 {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
        float W{ 0 };
    };


    struct Mat4 {
        float data[16];
    };

    struct CFrame {
        float data[12];

        Vec3 GetRightVector() const {
            return { data[0], data[3], data[6] };
        }

        Vec3 GetUpVector() const {
            return { data[1], data[4], data[7] };
        }

        Vec3 GetLookVector() const {
            return { data[2], data[5], data[8] };
        }

        Vec3 GetPosition() const {
            return { data[9], data[10], data[11] };
        }
    };

    class RbxInstance {
    public:
        uintptr_t Addr;

        RbxInstance(uintptr_t addr) : Addr(addr) {}

        std::string GetName() {
            if (Addr == 0) return "";
            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(Addr + offsets::Name);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        std::string GetClass() {
            if (Addr == 0) return "";
            uintptr_t classDesc = Coms->ReadMemory<uintptr_t>(Addr + offsets::ClassDescriptor);
            if (classDesc == 0) return "";

            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(classDesc + offsets::ClassDescriptorToClassName);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        RbxInstance GetParent() {
            if (Addr == 0) return RbxInstance(0);
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + offsets::Parent));
        }

        std::vector<RbxInstance> GetChildList() {
            std::vector<RbxInstance> childList;
            if (Addr == 0) return childList;
            
            uintptr_t childStart = Coms->ReadMemory<uintptr_t>(Addr + offsets::Children);
            if (childStart == 0) return childList;

            uintptr_t childEnd = Coms->ReadMemory<uintptr_t>(childStart + offsets::ChildrenEnd);
            uintptr_t currentPtr = Coms->ReadMemory<uintptr_t>(childStart);

            if (currentPtr == 0 || childEnd == 0 || currentPtr == childEnd) return childList;
            
            // Safety: Limit child count to prevent infinite loops if memory is corrupted
            int safetyLimit = 0;
            for (uintptr_t ptr = currentPtr; ptr != childEnd && safetyLimit < 5000; ptr += 0x10) {
                uintptr_t childAddr = Coms->ReadMemory<uintptr_t>(ptr);
                if (childAddr != 0) {
                    childList.emplace_back(childAddr);
                }
                safetyLimit++;
            }
            
            return childList;
        }

        RbxInstance FindChild(const std::string& targetName) {
            if (Addr == 0) return RbxInstance(0);
            auto children = GetChildList();
            for (auto& child : children) {
                if (child.GetName() == targetName) {
                    return child;
                }
            }
            return RbxInstance(0);
        }

        RbxInstance FindChildByClass(const std::string& targetClass) {
            if (Addr == 0) return RbxInstance(0);
            auto children = GetChildList();
            for (auto& child : children) {
                if (child.GetClass() == targetClass) {
                    return child;
                }
            }
            return RbxInstance(0);
        }

        RbxInstance WaitChild(const std::string& targetName) {
            if (Addr == 0) return RbxInstance(0);
            int safety = 0;
            while (safety < 100) {
                auto children = GetChildList();
                for (auto& child : children) {
                    if (child.GetName() == targetName) {
                        return child;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                safety++;
            }
            return RbxInstance(0);
        }

        uintptr_t GetPrimitivePtr() {
            if (Addr == 0) return 0;
            return Coms->ReadMemory<uintptr_t>(Addr + offsets::Primitive);
        }

        Vec3 GetPos() {
            uintptr_t prim = GetPrimitivePtr();
            if (prim == 0) return { 0, 0, 0 };
            return Coms->ReadMemory<Vec3>(prim + offsets::Position);
        }

        CFrame GetCFrame() {
            uintptr_t prim = GetPrimitivePtr();
            if (prim == 0) return CFrame();
            return Coms->ReadMemory<CFrame>(prim + offsets::CFrame);
        }

        RbxInstance GetModelRef() {
            if (Addr == 0) return RbxInstance(0);
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + offsets::ModelInstance));
        }

        float CalcDistance(const Vec3& targetPos) {
            Vec3 currentPos = GetPos();
            float dx = currentPos.X - targetPos.X;
            float dy = currentPos.Y - targetPos.Y;
            float dz = currentPos.Z - targetPos.Z;
            return sqrtf(dx * dx + dy * dy + dz * dz);
        }
    };


    class RenderEngine : public RbxInstance {
    public:
        RenderEngine(uintptr_t addr) : RbxInstance(addr) {}

        Mat4 GetViewMat() {
            return Coms->ReadMemory<Mat4>(Addr + offsets::viewmatrix);
        }


        Vec2 WorldToViewport(const Vec3& worldPos) {
            Vec4 quat;
            
            Vec2 screenDims{ static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), 
                            static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) };

            Mat4 viewMat = GetViewMat();
            
            quat.X = (worldPos.X * viewMat.data[0]) + (worldPos.Y * viewMat.data[1]) + (worldPos.Z * viewMat.data[2]) + viewMat.data[3];
            quat.Y = (worldPos.X * viewMat.data[4]) + (worldPos.Y * viewMat.data[5]) + (worldPos.Z * viewMat.data[6]) + viewMat.data[7];

            quat.Z = (worldPos.X * viewMat.data[8]) + (worldPos.Y * viewMat.data[9]) + (worldPos.Z * viewMat.data[10]) + viewMat.data[11];
            quat.W = (worldPos.X * viewMat.data[12]) + (worldPos.Y * viewMat.data[13]) + (worldPos.Z * viewMat.data[14]) + viewMat.data[15];
            
            Vec2 screenPos;
            
            if (quat.W < 0.1f) {

                return screenPos;
            }

            
            Vec3 ndc;
            ndc.X = quat.X / quat.W;
            ndc.Y = quat.Y / quat.W;
            ndc.Z = quat.Z / quat.W;
            
            screenPos.X = (screenDims.X / 2.0f * ndc.X) + (screenDims.X / 2.0f);
            screenPos.Y = -(screenDims.Y / 2.0f * ndc.Y) + (screenDims.Y / 2.0f);

            
            return screenPos;
        }
    };

    inline void ModifyWalkSpeed(RbxInstance humanoid, float newSpeed) {
        Coms->WriteMemory(humanoid.Addr + offsets::WalkSpeed, newSpeed);
        Coms->WriteMemory(humanoid.Addr + offsets::WalkSpeedCheck, newSpeed);

    }



    inline void ModifyJumpPower(RbxInstance humanoid, float newPower) {
        Coms->WriteMemory(humanoid.Addr + offsets::JumpPower, newPower);
        Coms->WriteMemory(humanoid.Addr + 0x1AC, newPower);
    }

    inline void SetCanCollide(RbxInstance model, bool canCollide, int depth = 0) {
        if (model.Addr == 0 || depth > 10) return;
        auto children = model.GetChildList();
        for (auto& child : children) {
            if (child.Addr == 0) continue;
            uintptr_t prim = child.GetPrimitivePtr();
            if (prim != 0) {
                uint8_t current = Coms->ReadMemory<uint8_t>(child.Addr + offsets::CanCollide);
                if (canCollide) current |= offsets::CanCollideMask;
                else current &= ~offsets::CanCollideMask;
                Coms->WriteMemory<uint8_t>(child.Addr + offsets::CanCollide, current);
            }
            SetCanCollide(child, canCollide, depth + 1);
        }
    }

    inline bool IsModerator(const std::string& name) {
        static std::vector<std::string> keywords = { "admin", "mod", "staff", "roblox", "helper" };
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        for (auto& kw : keywords) {
            if (lowerName.find(kw) != std::string::npos) return true;
        }
        return false;
    }

}
