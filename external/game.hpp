#pragma once
#include "memory.hpp"
#include <cmath>
#include <vector>

struct ImVec2;

namespace game
{
    namespace offsets
    {
        constexpr uintptr_t ClientEngine = 0x65F7AD0;
        constexpr uintptr_t EntityList = 0x6E4D0D8;
    }

    inline uintptr_t camera = 0;
    inline uintptr_t localActor = 0;

    struct XMFLOAT3X4
    {
        float _11, _12, _13, _14;
        float _21, _22, _23, _24;
        float _31, _32, _33, _34;
    };

    struct Vec3 { float x, y, z; };
    struct Vec2 { float x, y; };

    void MessiahMatrixAdd(XMFLOAT3X4 bone, XMFLOAT3X4 pos, Vec3& out)
    {
        out.x = (pos._11 * bone._32) + (pos._14 * bone._33) + (pos._23 * bone._34) + pos._32;
        out.y = (pos._12 * bone._32) + (pos._21 * bone._33) + (pos._24 * bone._34) + pos._33;
        out.z = (pos._13 * bone._32) + (pos._22 * bone._33) + (pos._31 * bone._34) + pos._34;
    }

    bool W2S(uintptr_t cam, const Vec3& world, Vec2& out)
    {
        if (!cam) return false;

        float relX = world.x - mem::Read<float>(cam + 124);
        float relY = world.y - mem::Read<float>(cam + 128);
        float relZ = world.z - mem::Read<float>(cam + 132);

        float px = relX * mem::Read<float>(cam + 772)
            + relY * mem::Read<float>(cam + 784)
            + relZ * mem::Read<float>(cam + 796);

        float py = relX * mem::Read<float>(cam + 776)
            + relY * mem::Read<float>(cam + 788)
            + relZ * mem::Read<float>(cam + 800);

        float pzOrig = relX * mem::Read<float>(cam + 780)
            + relY * mem::Read<float>(cam + 792)
            + relZ * mem::Read<float>(cam + 804);

        if (pzOrig >= -0.01f) return false;

        float pz = -pzOrig;
        float fov = mem::Read<float>(cam + 824);
        float f = 1.0f / tanf((fov * 0.017453292f) * 0.5f);

        float screenW = (float)mem::Read<uint16_t>(cam + 752);
        float screenH = (float)mem::Read<uint16_t>(cam + 754);

        float invZ = 1.0f / fmaxf(fabsf(pz), 0.000001f);

        out.x = roundf(((px * invZ) * f * screenH + screenW) * 0.5f * 10.0f) * 0.1f;
        out.y = roundf(((screenH - ((py * invZ) * f * screenH)) * 0.5f) * 10.0f) * 0.1f;

        return true;
    }

    struct EntityData
    {
        uintptr_t address;
        uintptr_t IEntity;
        uintptr_t actorInstance;
        Vec3 worldPos;
        Vec2 screenPos;
        float distance;
        bool valid;
    };

    bool ReadEntityList(std::vector<EntityData>& entities)
    {
        entities.clear();

        uintptr_t ClientEngine = mem::Read<uintptr_t>(mem::base + offsets::ClientEngine);
        if (!ClientEngine) return false;

        uintptr_t IGameplay = mem::Read<uintptr_t>(ClientEngine + 0x58);
        if (!IGameplay) return false;

        uintptr_t ClientPlayer = mem::Read<uintptr_t>(IGameplay + 0x58);
        if (!ClientPlayer) return false;

        camera = mem::Read<uintptr_t>(ClientPlayer + 0x238);
        localActor = mem::Read<uintptr_t>(ClientPlayer + 0x288);
        if (!camera || !localActor) return false;

        Vec3 localPos = mem::Read<Vec3>(localActor + 0x58 + 36);

        uintptr_t entityListStart = mem::Read<uintptr_t>(mem::base + offsets::EntityList);
        if (!entityListStart) return false;

        uintptr_t head = mem::Read<uintptr_t>(entityListStart + 0x8);
        if (!head) return false;

        uintptr_t currentActor = mem::Read<uintptr_t>(head);
        if (!currentActor) return false;

        int iterations = 0;
        do
        {
            if (iterations++ >= 1024) break;
            if (currentActor == head) break;

            uintptr_t next = mem::Read<uintptr_t>(currentActor);

            uintptr_t actorInstance = mem::Read<uintptr_t>(currentActor + 0x18);
            if (!actorInstance) { currentActor = next; continue; }
            uintptr_t actorProps = mem::Read<uintptr_t>(actorInstance + 0x278);
            if (!actorProps) { currentActor = next; continue; }
            uintptr_t actorComponent = mem::Read<uintptr_t>(actorProps + 0x18);
            if (!actorComponent) { currentActor = next; continue; }
            uintptr_t IEntity = mem::Read<uintptr_t>(actorComponent + 0x40);
            if (!IEntity) { currentActor = next; continue; }
            uintptr_t entityMask = mem::Read<uint64_t>(IEntity + 0x2e0);
            if (entityMask != 2) { currentActor = next; continue; }
            if (IEntity == localActor) { currentActor = next; continue; }
            uintptr_t IArea = mem::Read<uintptr_t>(IEntity + 0x88);
            if (IArea == 0) { currentActor = next; continue; }

            EntityData ent{};
            ent.address = currentActor;
            ent.IEntity = IEntity;
            ent.actorInstance = actorInstance;

            float trans[12];
            ReadProcessMemory(mem::hProcess, (LPCVOID)(IEntity + 0x58), trans, sizeof(trans), nullptr);
            ent.worldPos.x = trans[9];
            ent.worldPos.y = trans[10];
            ent.worldPos.z = trans[11];

            float dx = ent.worldPos.x - localPos.x;
            float dy = ent.worldPos.y - localPos.y;
            float dz = ent.worldPos.z - localPos.z;
            ent.distance = sqrtf(dx * dx + dy * dy + dz * dz);

            Vec2 sPos;
            if (W2S(camera, ent.worldPos, sPos))
            {
                ent.screenPos = sPos;
                ent.valid = true;
            }
            else
            {
                ent.valid = false;
            }

            entities.push_back(ent);
            currentActor = next;
        } while (currentActor != head);

        return true;
    }

    bool ReadBone(uintptr_t BipedPose, int boneIdx, XMFLOAT3X4 dxTrans, Vec3& out)
    {
        uintptr_t boneStart = mem::Read<uintptr_t>((boneIdx * 0x8) + BipedPose);
        if (!boneStart) return false;
        XMFLOAT3X4 boneMat = mem::Read<XMFLOAT3X4>(boneStart + 0x30);
        MessiahMatrixAdd(boneMat, dxTrans, out);
        return true;
    }

    struct BoneData
    {
        Vec2 neck, spine1, spine2, spine3, pelvis;
        Vec2 buttCheekL, buttCheekR;
        Vec2 kneeL, kneeR, footL, footR;
        Vec2 sholL, elbowL, wristL;
        Vec2 sholR, elbowR, wristR;
        bool valid = false;
    };

    bool ReadBones(uintptr_t actorInstance, uintptr_t IEntity, BoneData& bones)
    {
        uintptr_t pose = mem::Read<uintptr_t>(actorInstance + 0x18);
        if (!pose) return false;
        uintptr_t BipedPose = mem::Read<uintptr_t>(pose + 0x90);
        if (!BipedPose) return false;
        BipedPose += 0x8;

        XMFLOAT3X4 dxTrans = mem::Read<XMFLOAT3X4>(IEntity + 0x58);

        Vec3 _neck, _spine1, _spine2, _spine3, _pelvis;
        Vec3 _bCL, _bCR, _kL, _kR, _fL, _fR;
        Vec3 _sL, _eL, _wL, _sR, _eR, _wR;

        ReadBone(BipedPose, 7, dxTrans, _neck);
        ReadBone(BipedPose, 6, dxTrans, _spine1);
        ReadBone(BipedPose, 5, dxTrans, _spine2);
        ReadBone(BipedPose, 4, dxTrans, _spine3);
        ReadBone(BipedPose, 3, dxTrans, _pelvis);
        ReadBone(BipedPose, 22, dxTrans, _bCL);
        ReadBone(BipedPose, 18, dxTrans, _bCR);
        ReadBone(BipedPose, 23, dxTrans, _kL);
        ReadBone(BipedPose, 19, dxTrans, _kR);
        ReadBone(BipedPose, 24, dxTrans, _fL);
        ReadBone(BipedPose, 20, dxTrans, _fR);
        ReadBone(BipedPose, 14, dxTrans, _sL);
        ReadBone(BipedPose, 9, dxTrans, _sR);
        ReadBone(BipedPose, 15, dxTrans, _eL);
        ReadBone(BipedPose, 10, dxTrans, _eR);
        ReadBone(BipedPose, 16, dxTrans, _wL);
        ReadBone(BipedPose, 11, dxTrans, _wR);

        W2S(camera, _neck, bones.neck);
        W2S(camera, _spine1, bones.spine1);
        W2S(camera, _spine2, bones.spine2);
        W2S(camera, _spine3, bones.spine3);
        W2S(camera, _pelvis, bones.pelvis);
        W2S(camera, _bCL, bones.buttCheekL);
        W2S(camera, _bCR, bones.buttCheekR);
        W2S(camera, _kL, bones.kneeL);
        W2S(camera, _kR, bones.kneeR);
        W2S(camera, _fL, bones.footL);
        W2S(camera, _fR, bones.footR);
        W2S(camera, _sL, bones.sholL);
        W2S(camera, _sR, bones.sholR);
        W2S(camera, _eL, bones.elbowL);
        W2S(camera, _eR, bones.elbowR);
        W2S(camera, _wL, bones.wristL);
        W2S(camera, _wR, bones.wristR);

        bones.valid = true;
        return true;
    }

    bool GetAimTarget(float screenCX, float screenCY, Vec2& outTarget, float fovRadius)
    {
        if (!camera || !localActor) return false;

        Vec3 localPos = mem::Read<Vec3>(localActor + 0x58 + 36);

        uintptr_t entityListStart = mem::Read<uintptr_t>(mem::base + offsets::EntityList);
        if (!entityListStart) return false;
        uintptr_t head = mem::Read<uintptr_t>(entityListStart + 0x8);
        if (!head) return false;
        uintptr_t currentActor = mem::Read<uintptr_t>(head);
        if (!currentActor) return false;

        float closestDist = fovRadius;
        bool found = false;

        int iterations = 0;
        do
        {
            if (iterations++ >= 1024) break;
            if (currentActor == head) break;

            uintptr_t next = mem::Read<uintptr_t>(currentActor);

            uintptr_t actorInstance = mem::Read<uintptr_t>(currentActor + 0x18);
            if (!actorInstance) { currentActor = next; continue; }
            uintptr_t actorProps = mem::Read<uintptr_t>(actorInstance + 0x278);
            if (!actorProps) { currentActor = next; continue; }
            uintptr_t actorComponent = mem::Read<uintptr_t>(actorProps + 0x18);
            if (!actorComponent) { currentActor = next; continue; }
            uintptr_t IEntity = mem::Read<uintptr_t>(actorComponent + 0x40);
            if (!IEntity) { currentActor = next; continue; }
            uintptr_t entityMask = mem::Read<uint64_t>(IEntity + 0x2e0);
            if (entityMask != 2) { currentActor = next; continue; }
            if (IEntity == localActor) { currentActor = next; continue; }
            uintptr_t IArea = mem::Read<uintptr_t>(IEntity + 0x88);
            if (IArea == 0) { currentActor = next; continue; }

            uintptr_t actorInst = mem::Read<uintptr_t>(currentActor + 0x18);
            if (!actorInst) { currentActor = next; continue; }
            uintptr_t pose = mem::Read<uintptr_t>(actorInst + 0x18);
            if (!pose) { currentActor = next; continue; }
            uintptr_t BipedPose = mem::Read<uintptr_t>(pose + 0x90);
            if (!BipedPose) { currentActor = next; continue; }
            BipedPose += 0x8;

            XMFLOAT3X4 dxTrans = mem::Read<XMFLOAT3X4>(IEntity + 0x58);
            Vec3 headPos;
            if (!ReadBone(BipedPose, 7, dxTrans, headPos)) { currentActor = next; continue; }

            Vec2 bone2D;
            if (W2S(camera, headPos, bone2D))
            {
                float ddx = bone2D.x - screenCX;
                float ddy = bone2D.y - screenCY;
                float dist = sqrtf(ddx * ddx + ddy * ddy);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    outTarget = bone2D;
                    found = true;
                }
            }

            currentActor = next;
        } while (currentActor != head);

        return found;
    }
}
