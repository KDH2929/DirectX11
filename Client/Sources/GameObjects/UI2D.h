#pragma once

#include "GameObject.h"
#include "Texture.h"
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

class UI2D : public GameObject {
public:
    UI2D(XMFLOAT2 position, XMFLOAT2 size, std::shared_ptr<Texture> texture = nullptr, XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

    bool Initialize(Renderer* renderer) override;
    void Update(float deltaTime) override;
    void Render(Renderer* renderer) override;

    void SetPosition(float x, float y);
    XMFLOAT2 GetPosition() const;

    void SetSize(float width, float height);
    XMFLOAT2 GetSize() const;

    void SetColor(XMFLOAT4 color);
    XMFLOAT4 GetColor() const;

private:
    bool CreateDepthStencilState(ID3D11Device* device);
    bool CreateRasterizerState(ID3D11Device* device);

private:
    XMFLOAT2 position;  // ��ġ (ȭ�� ��ǥ��)
    XMFLOAT2 size;      // ũ��
    XMFLOAT4 color;     // ����
    std::shared_ptr<Texture> texture;  // �ؽ�ó

    std::shared_ptr<Mesh> quadMesh;
};
