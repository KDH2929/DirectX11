#include "EnemyFlight.h"
#include "DebugManager.h"

EnemyFlight::EnemyFlight(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture)
    : Flight(mesh, texture)
{
    SetIsEnemy(true);
    SetHealth(100.0f);
}

bool EnemyFlight::Initialize(Renderer* renderer) {
    if (!Flight::Initialize(renderer))
        return false;

    SetAlive(true);

    XMFLOAT3 currentPosition = GetPosition();

    // Enemy �� �浹ü ����
    PxRigidDynamic* actor = PhysicsManager::GetInstance()->CreateDynamicBox(
        { currentPosition.x, currentPosition.y, currentPosition.z },
        { 4, 4, 4 }, { 0, 0, 0 }, CollisionLayer::Enemy);

    // ���� �ٸ� ������Ʈ�� �浹�� �� �ֵ��� ���� ���� (��: Default�� Block)
    PhysicsManager::GetInstance()->SetCollisionResponse(
        CollisionLayer::Enemy, CollisionLayer::Default, CollisionResponse::Block);

    actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
    actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

    // ���� ���� ����
    PxMaterial* material = PhysicsManager::GetInstance()->GetDefaultMaterial();
    material->setRestitution(0.0f);
    material->setStaticFriction(0.0f);
    material->setDynamicFriction(0.0f);

    PxShape* shape;
    actor->getShapes(&shape, 1);
    shape->setMaterials(&material, 1);

    BindPhysicsActor(actor);

    if (auto* dynamicActor = physicsActor->is<PxRigidDynamic>()) {
        dynamicActor->setLinearVelocity(PxVec3(0, 0, 0));
        dynamicActor->setAngularVelocity(PxVec3(0, 0, 0));
    }

    SetColliderOffset(XMFLOAT3(0.0f, 5.0f, 0.0f));
    SetCollisionLayer(CollisionLayer::Enemy);

    return true;
}

void EnemyFlight::Update(float deltaTime) {

    if (!IsAlive()) return;

    Flight::Update(deltaTime);

    SyncFromPhysics();
    UpdateWorldMatrix();
    DrawPhysicsColliderDebug();
}

void EnemyFlight::SetExplosionEffect(std::shared_ptr<BillboardExplosion> explosion)
{
    explosionEffect = explosion;
}

void EnemyFlight::OnDestroyed()
{
    DebugManager::GetInstance().AddOnScreenMessage("EnemyFlight Destroyed!", 10.0f);

    SetAlive(false);

    // ������ ����
    if (physicsActor) {
        PxScene* scene = PhysicsManager::GetInstance()->GetScene();

        PhysicsManager::GetInstance()->RemoveActor(physicsActor);

        physicsActor = nullptr;  // ���� ������ ����
    }


    if (explosionEffect)
    {
        explosionEffect->SetPosition(GetPosition());
        explosionEffect->Activate(); // ������/�ð� �ʱ�ȭ �� ���� 1.0 ��
    }
}
