#pragma once

#include "GameObject.h"

class Flight : public GameObject {
public:
    Flight(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture);
    ~Flight() override = default;

    bool Initialize(Renderer* renderer) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;

    // ü�°���
    void SetHealth(float hp);
    float GetHealth() const;
    void TakeDamage(float damage);
    virtual void OnDestroyed();

    // ���� ���� ����
    void SetAlive(bool alive);
    bool IsAlive() const;


    // ��/�Ʊ� �÷���
    void SetIsEnemy(bool enemy);
    bool IsEnemy() const;


private:

    std::weak_ptr<Mesh> flightMesh;

    // ü��
    float health = 100.0f;
    float maxHealth = 100.0f;

    // �� ����
    bool isEnemy = false;
    bool isAlive = true;

};
