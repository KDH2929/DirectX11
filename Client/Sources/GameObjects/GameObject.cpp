#include "GameObject.h"
#include "DebugRenderer.h"
#include "Renderer.h"

GameObject::GameObject()
    : position(0.0f, 0.0f, 0.0f),
    scale(1.0f, 1.0f, 1.0f),
    rotationEuler(0.0f, 0.0f, 0.0f),
    rotationQuat(XMQuaternionIdentity())
{}

bool GameObject::Initialize(Renderer* renderer) {
    auto device = renderer->GetDevice();

    D3D11_BUFFER_DESC cbMVPDesc = {};
    cbMVPDesc.ByteWidth = sizeof(CB_MVP);
    cbMVPDesc.Usage = D3D11_USAGE_DEFAULT;
    cbMVPDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    HRESULT hr = device->CreateBuffer(&cbMVPDesc, nullptr, &constantMVPBuffer);
    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC cbMaterialDesc = {};
    cbMaterialDesc.ByteWidth = sizeof(CB_Material);
    cbMaterialDesc.Usage = D3D11_USAGE_DEFAULT;
    cbMaterialDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    hr = device->CreateBuffer(&cbMaterialDesc, nullptr, &constantMaterialBuffer);
    if (FAILED(hr)) return false;

    // textureManager �ʱ�ȭ
    textureManager.Initialize(device);

    return true;
}

void GameObject::SetPosition(const XMFLOAT3& newPosition) {
    position = newPosition;
}

void GameObject::SetScale(const XMFLOAT3& newScale) {
    scale = newScale;
}

void GameObject::SetRotationEuler(const XMFLOAT3& eulerAngles) {
    rotationEuler = eulerAngles;
    rotationQuat = XMQuaternionRotationRollPitchYaw(eulerAngles.x, eulerAngles.y, eulerAngles.z);
}

void GameObject::SetRotationQuat(const XMVECTOR& quat) {
    rotationQuat = XMQuaternionNormalize(quat);

    XMFLOAT4 q;
    XMStoreFloat4(&q, rotationQuat);
    rotationEuler = {
        asinf(2.0f * (q.w * q.x - q.y * q.z)),
        atan2f(2.0f * (q.w * q.y + q.x * q.z), 1.0f - 2.0f * (q.y * q.y + q.x * q.x)),
        atan2f(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.z * q.z + q.x * q.x))
    };
}

XMFLOAT3 GameObject::GetPosition() const {
    return position;
}

XMFLOAT3 GameObject::GetScale() const {
    return scale;
}

XMFLOAT3 GameObject::GetRotationEuler() const {
    return rotationEuler;
}

XMVECTOR GameObject::GetRotationQuat() const {
    return rotationQuat;
}


void GameObject::AddForce(const XMFLOAT3& force)
{
    if (!physicsActor) return;

    if (auto* dynamicActor = physicsActor->is<physx::PxRigidDynamic>()) {
        PxVec3 pxForce(force.x, force.y, force.z);
        dynamicActor->addForce(pxForce, physx::PxForceMode::eIMPULSE);
    }
}

void GameObject::SetColliderOffset(const XMFLOAT3& offset)
{
    colliderOffset = offset;
}

XMFLOAT3 GameObject::GetColliderOffset() const
{
    return colliderOffset;
}

void GameObject::BindPhysicsActor(physx::PxRigidActor* actor)
{
    physicsActor = actor;
    actor->userData = this;
}

void GameObject::SyncFromPhysics()
{
    if (!physicsActor) return;

    using namespace physx;
    PxTransform t = physicsActor->getGlobalPose();
    SetPosition({ t.p.x - colliderOffset.x, t.p.y - colliderOffset.y, t.p.z - colliderOffset.z});

    XMVECTOR q = XMVectorSet(t.q.x, t.q.y, t.q.z, t.q.w);
    SetRotationQuat(q);
}

void GameObject::ApplyTransformToPhysics()
{
    if (!physicsActor) return;

    // ���� �������� ����� ��ġ�� ȸ�� ���
    PxVec3 pxPos(position.x + colliderOffset.x, position.y + colliderOffset.y, position.z + colliderOffset.z);
    XMFLOAT4 quat;
    XMStoreFloat4(&quat, rotationQuat);
    PxQuat pxRot(quat.x, quat.y, quat.z, quat.w);

    // ���� ��ü�� ��쿡�� ���� �����Ͽ� �̵�
    if (auto* dynamicActor = physicsActor->is<PxRigidDynamic>()) {

        // ���� ������ �浹�� �ݿ��� �̵�
        // PxVec3 velocity = dynamicActor->getLinearVelocity();  // ���� �ӵ� ��������
        // �ӵ��� �����ϰų� ������ ���� �߰��Ͽ� �̵�
        // ����: dynamicActor->addForce(PxVec3(0, 0, 1) * forceMagnitude, PxForceMode::eIMPULSE);


        // �浹 �� ƨ�ܳ����� ���� �����ϱ� ���� �ӵ��� ȸ�� ����
        dynamicActor->setLinearVelocity(PxVec3(0, 0, 0));  // �ӵ� ����
        dynamicActor->setAngularVelocity(PxVec3(0, 0, 0)); // ȸ�� �ӵ� ����

        // ������ ��ġ�� ����
        dynamicActor->setGlobalPose(PxTransform(pxPos, pxRot));

        // dyn->clearForce(); // �̹� ����� ���� �����
        // dyn->clearTorque(); // ȸ���µ� �����
    }
}


void GameObject::DrawPhysicsColliderDebug()
{
    if (!physicsActor) return;

    auto draw = [](PxRigidActor* actor) {
        PxTransform transform = actor->getGlobalPose();
        PxShape* shape = nullptr;
        actor->getShapes(&shape, 1);

        PxGeometryHolder holder = shape->getGeometry();
        switch (holder.getType()) {
        case PxGeometryType::eBOX: {
            const PxBoxGeometry& geom = holder.box();
            DebugRenderer::GetInstance().DrawBox(transform.p, geom.halfExtents, transform.q);
            break;
        }
        case PxGeometryType::eSPHERE: {
            const PxSphereGeometry& geom = holder.sphere();
            DebugRenderer::GetInstance().DrawSphere(transform.p, geom.radius);
            break;
        }
        default:
            break;
        }
        };

    if (auto* dynamicActor = physicsActor->is<PxRigidDynamic>())
        draw(dynamicActor);
    else if (auto* staticActor = physicsActor->is<PxRigidStatic>())
        draw(staticActor);
}

physx::PxRigidActor* GameObject::GetPhysicsActor() const
{
    return physicsActor;
}

void GameObject::SetCollisionLayer(CollisionLayer layer)
{
    if (!physicsActor) return;

    uint32_t word0 = static_cast<uint32_t>(layer);

    PxShape* shape = nullptr;
    physicsActor->getShapes(&shape, 1);
    if (shape)
    {
        shape->setSimulationFilterData(PxFilterData(word0, 0, 0, 0));
        shape->setQueryFilterData(PxFilterData(word0, 0, 0, 0));
    }
}

void GameObject::UpdateWorldMatrix() {
    XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
    XMMATRIX R = XMMatrixRotationQuaternion(rotationQuat);
    XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);

    worldMatrix = S * R * T;
}
