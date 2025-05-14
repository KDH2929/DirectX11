#pragma once

#include "physx/PxPhysicsAPI.h"
#include <vector>
#include <unordered_map>
#include <functional>

using namespace physx;

// ���� ���
constexpr const char* PHYSX_PVD_HOST = "127.0.0.1";
constexpr int         PHYSX_PVD_PORT = 5425;
constexpr int         PHYSX_PVD_TIMEOUT = 10;
constexpr int         PHYSX_NUM_THREADS = 2;
constexpr float       PHYSX_GRAVITY_Y = -9.81f;

// �浹 ���� Ÿ��
enum class CollisionResponse
{
    Ignore,
    Overlap,
    Block
};

// �ݸ��� ���̾� ����
enum class CollisionLayer : uint32_t
{
    Default = 1,
    Player = 2,
    Enemy = 3,
    Projectile = 4,
    EnemyProjectile = 5,
    PlayerProjectile = 6,
};


struct CollisionEventHandler
{
    // ���͸� ���� (��: Ư�� ���̾� ��, ������Ʈ Ÿ�� ��)
    std::function<bool(PxActor*, PxActor*)> Matches;

    // ó���� �ݹ�
    std::function<void(PxActor*, PxActor*)> Callback;
};

class PhysicsManager : public PxSimulationEventCallback
{
public:
    // ���� �ν��Ͻ� ����
    static PhysicsManager* GetInstance();
    static void SetInstance(PhysicsManager* instance);

    // ���� & �Ҹ�
    bool Init();
    void Cleanup();

    void Simulate(float deltaTime);
    void FetchResults();

    // ������
    PxPhysics* GetPhysics() const { return physics; }
    PxScene* GetScene() const { return scene; }
    PxMaterial* GetDefaultMaterial() const { return defaultMaterial; }

    // ������Ʈ ����
    PxRigidStatic* CreateStaticBox(const PxVec3& pos, const PxVec3& halfExtents, CollisionLayer layer = CollisionLayer::Default);
    PxRigidDynamic* CreateDynamicBox(const PxVec3& pos, const PxVec3& halfExtents, const PxVec3& velocity, CollisionLayer layer = CollisionLayer::Default);
    PxRigidDynamic* CreateSphere(const PxVec3& pos, float radius, const PxVec3& velocity, CollisionLayer layer = CollisionLayer::Default);
    PxRigidDynamic* CreateCapsule(const PxVec3& pos, float radius, float halfHeight, const PxVec3& velocity, CollisionLayer layer = CollisionLayer::Default);
    PxRigidDynamic* CreateProjectile(const PxVec3& pos, const PxVec3& dir, float speed, float radius, float mass = 1.0f, CollisionLayer layer = CollisionLayer::Projectile);

    // �浹 ���� ���� (Block, Overlap, Ignore)
    void SetCollisionResponse(CollisionLayer a, CollisionLayer b, CollisionResponse response);


    void RegisterHitHandler(CollisionEventHandler handler);
    void RegisterOverlapHandler(CollisionEventHandler handler);

    
    // ���Ű����Լ�
    void RemoveActor(PxActor* actor);
    void ApplyActorRemovals();

private:
    // ���� ��ƿ
    void SetShapeCollisionFilter(PxShape* shape, CollisionLayer layer);

    // �浹 ���� ���̴�
    static PxFilterFlags CustomFilterShader(
        PxFilterObjectAttributes attributes0, PxFilterData filterData0,
        PxFilterObjectAttributes attributes1, PxFilterData filterData1,
        PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize
    );

    // PxSimulationEventCallback ����
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override;
    void onTrigger(PxTriggerPair* pairs, PxU32 count) override;
    void onConstraintBreak(PxConstraintInfo*, PxU32) override {}
    void onWake(PxActor**, PxU32) override {}
    void onSleep(PxActor**, PxU32) override {}
    void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) override {}

private:
    // ���� �ν��Ͻ�
    static PhysicsManager* s_instance;

    // PhysX ���� ���
    PxDefaultAllocator      allocator;
    PxDefaultErrorCallback  errorCallback;
    PxFoundation* foundation = nullptr;
    PxPhysics* physics = nullptr;
    PxDefaultCpuDispatcher* dispatcher = nullptr;
    PxScene* scene = nullptr;
    PxMaterial* defaultMaterial = nullptr;
    PxPvd* pvd = nullptr;


    std::vector<PxActor*> actorsToRemove;


    // �浹 ���� ��Ʈ����
    CollisionResponse collisionMatrix[32][32] = {};

    // ���ҽ� ����
    std::vector<PxActor*> managedActors;

    // �̺�Ʈ �ڵ鷯�� ����
    std::vector<CollisionEventHandler> hitHandlers;
    std::vector<CollisionEventHandler> overlapHandlers;

};
