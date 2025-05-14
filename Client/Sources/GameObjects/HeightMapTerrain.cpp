#include "HeightMapTerrain.h"
#include "Renderer.h"
#include "DebugManager.h"

#include <fstream>

HeightMapTerrain::HeightMapTerrain(const std::wstring& path, int width, int height, float scale, float vertexDist)
    : heightMapPath(path), mapWidth(width), mapHeight(height), heightScale(scale), vertexDistance(vertexDist)
{}

bool HeightMapTerrain::Initialize(Renderer* renderer)
{
    GameObject::Initialize(renderer);

    LoadHeightMap();
    CreateMeshFromHeightMap();

    auto device = renderer->GetDevice();

    // Vertex Buffer
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = UINT(sizeof(MeshVertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices.data();

    device->CreateBuffer(&vbd, &vinitData, vertexBuffer.GetAddressOf());

    // Index Buffer
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = UINT(sizeof(UINT) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();

    device->CreateBuffer(&ibd, &iinitData, indexBuffer.GetAddressOf());

    // Shader ����
    auto shaderManager = renderer->GetShaderManager();
    shaderInfo.vertexShaderName = L"PhongVertexShader";
    shaderInfo.pixelShaderName = L"PhongPixelShader";

    vertexShader = shaderManager->GetVertexShader(shaderInfo.vertexShaderName);
    pixelShader = shaderManager->GetPixelShader(shaderInfo.pixelShaderName);
    inputLayout = shaderManager->GetInputLayout(shaderInfo.vertexShaderName);

    // Rasterizer ����
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.CullMode = shaderInfo.cullMode;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthClipEnable = true;

    device->CreateRasterizerState(&rsDesc, rasterizerState.GetAddressOf());

    // Material �⺻��
    materialData.ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
    materialData.diffuse = XMFLOAT3(0.6f, 0.6f, 0.6f);
    materialData.specular = XMFLOAT3(0.1f, 0.1f, 0.1f);
    materialData.useTexture = 0;


    // �ݶ��̴� ũ�� ����
    PxVec3 halfExtents = { 30.0f, 5.0f, 30.0f };  // �ʱⰪ

    // ���� �޽��� ���� ���̸� ����Ͽ� �ݶ��̴� ũ�� ����
    float terrainWidth = static_cast<float>(mapWidth) * vertexDistance;
    float terrainHeight = static_cast<float>(mapHeight) * vertexDistance;

    halfExtents.x = terrainWidth / 2.0f;
    halfExtents.z = terrainHeight / 2.0f;

    XMFLOAT3 terrainPos = GetPosition();
    PxVec3 colliderCenter = PxVec3(
        terrainPos.x + halfExtents.x,
        terrainPos.y - halfExtents.y,
        terrainPos.z + halfExtents.z
    );

    // �ݶ��̴� ����
    PxRigidStatic* actor = PhysicsManager::GetInstance()->CreateStaticBox(
        colliderCenter,
        halfExtents,
        CollisionLayer::Default
    );

    BindPhysicsActor(actor);


    return true;
}

void HeightMapTerrain::Update(float deltaTime)
{
    UpdateWorldMatrix();
}

void HeightMapTerrain::Render(Renderer* renderer)
{
    DrawPhysicsColliderDebug();

    auto context = renderer->GetDeviceContext();

    // ���̴� & �Է� ���̾ƿ�
    context->IASetInputLayout(inputLayout.Get());
    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->RSSetState(rasterizerState.Get());

    // ����/�ε��� ���� ����
    UINT stride = sizeof(MeshVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // CB_MVP (b0) ����
    CB_MVP mvpData;
    XMMATRIX view = renderer->GetCamera()->GetViewMatrix();
    XMMATRIX proj = renderer->GetCamera()->GetProjectionMatrix();

    mvpData.model = XMMatrixTranspose(worldMatrix);
    mvpData.view = XMMatrixTranspose(view);
    mvpData.projection = XMMatrixTranspose(proj);
    mvpData.modelInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

    context->UpdateSubresource(constantMVPBuffer.Get(), 0, nullptr, &mvpData, 0, 0);
    context->VSSetConstantBuffers(0, 1, constantMVPBuffer.GetAddressOf());  // b0

    // ���� (b1)
    ID3D11Buffer* lightCB = renderer->GetLightingConstantBuffer();
    context->PSSetConstantBuffers(1, 1, &lightCB);

    // ��Ƽ���� (b2)
    context->UpdateSubresource(constantMaterialBuffer.Get(), 0, nullptr, &materialData, 0, 0);
    context->PSSetConstantBuffers(2, 1, constantMaterialBuffer.GetAddressOf());

    // ��ο�
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}

void HeightMapTerrain::SetVertexDistance(float vertexDist)
{
    vertexDistance = vertexDist;
}

void HeightMapTerrain::LoadHeightMap()
{
    FILE* file = nullptr;
    _wfopen_s(&file, heightMapPath.c_str(), L"rb");

    if (!file) {
        return;
    }

    heightData.resize(mapWidth * mapHeight);
    fread(heightData.data(), 1, heightData.size(), file);
    fclose(file);

    if (heightData.empty()) {
        throw std::runtime_error("Failed to load heightmap data.");
    }
}

void HeightMapTerrain::CreateMeshFromHeightMap()
{
    vertices.clear();
    indices.clear();

    // vertices ����
    for (int z = 0; z < mapHeight; ++z)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            if (!heightData.empty()) {
                // ���� ���
                float height = heightData[z * mapWidth + x] / 255.0f * heightScale;

                float posX = (float)x * vertexDistance;
                float posZ = (float)z * vertexDistance;
                XMFLOAT3 pos = XMFLOAT3(posX, height, posZ);

                // ��� ���
                XMFLOAT3 normal = CalculateNormal(x, z);

                // UV ���
                XMFLOAT2 uv = XMFLOAT2((float)x / mapWidth, (float)z / mapHeight);
                vertices.push_back({ pos, normal, uv });
            }
        }
    }

    // indices ����
    for (int z = 0; z < mapHeight - 1; ++z)
    {
        for (int x = 0; x < mapWidth - 1; ++x)
        {
            int start = z * mapWidth + x;

            // �ε��� ���� ���
            if (start + mapWidth + 1 < vertices.size()) {
                indices.push_back(start);
                indices.push_back(start + mapWidth);
                indices.push_back(start + 1);

                indices.push_back(start + 1);
                indices.push_back(start + mapWidth);
                indices.push_back(start + mapWidth + 1);
            }
        }
    }
}

// �ֺ� ������κ��� ���� ���� ���
XMFLOAT3 HeightMapTerrain::CalculateNormal(int x, int z)
{
    // ��� �˻縦 ���� �ε��� ���� �ʰ� ����
    float heightL = (x > 0) ? heightData[z * mapWidth + (x - 1)] : heightData[z * mapWidth + x]; // ����
    float heightR = (x < mapWidth - 1) ? heightData[z * mapWidth + (x + 1)] : heightData[z * mapWidth + x]; // ������
    float heightD = (z > 0) ? heightData[(z - 1) * mapWidth + x] : heightData[z * mapWidth + x]; // �Ʒ�
    float heightU = (z < mapHeight - 1) ? heightData[(z + 1) * mapWidth + x] : heightData[z * mapWidth + x]; // ��

    // ��� ��� (�̿����� ���� ����)
    float dx = (heightR - heightL) * heightScale;
    float dz = (heightU - heightD) * heightScale;

    // ���� ���� ���
    XMFLOAT3 normal = { dx, 1.0f, dz };
    XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&normal)));  // ���� ����ȭ

    return normal;
}
