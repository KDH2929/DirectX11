#pragma once

#include "Flight.h"
#include "InputManager.h"
#include "Camera.h"
#include "CameraController.h"
#include "BillboardMuzzleFlash.h"
#include "BillboardMuzzleSmoke.h"
#include "Bullet.h"

class PlayerFlight : public Flight {
public:
    PlayerFlight(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture);
    ~PlayerFlight() override = default;

    bool Initialize(Renderer* renderer) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;


private:
    void Fire();


private:
    // �ӵ� �� �߷� ����
    float speed = 0.0f;              // ���� �ӵ�
    float maxSpeed = 300.0f;         // �ִ� �ӵ�
    float cruiseSpeed = 150.0f;      // ���� �ӵ�

    float maxThrustAccel = 200.0f;   // �ִ� �߷� ���ӵ�
    float dragCoefficient = 0.01f;   // �׷� ��� (�ӵ� ���� ���)

    float lastFireTime = 0.0f;       // ������ �߻� �ð�
    float fireCooldown = 0.1f;       // �ּ� �߻� ���� (�� ����)

    XMVECTOR localForward = XMVectorSet(0, 0, 1, 0);
    XMVECTOR localRight = XMVectorSet(1, 0, 0, 0);
    XMVECTOR localUp = XMVectorSet(0, 1, 0, 0);


    // ī�޶�
    std::shared_ptr<Camera> camera;
    CameraController cameraController;
    XMVECTOR cameraQuat = XMQuaternionIdentity();

    std::vector<std::shared_ptr<BillboardMuzzleFlash>> muzzleFlashPoolLeft;
    std::vector<std::shared_ptr<BillboardMuzzleFlash>> muzzleFlashPoolRight;

    std::vector<std::shared_ptr<BillboardMuzzleSmoke>> muzzleSmokePoolLeft;
    std::vector<std::shared_ptr<BillboardMuzzleSmoke>> muzzleSmokePoolRight;


    std::shared_ptr<Mesh> bulletMesh;
    std::shared_ptr<Texture> bulletTexture;
    std::vector<std::shared_ptr<Bullet>> bulletPool;

};
