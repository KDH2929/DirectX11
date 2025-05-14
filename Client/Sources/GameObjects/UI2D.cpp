#include "UI2D.h"
#include "Renderer.h"

UI2D::UI2D(XMFLOAT2 position, XMFLOAT2 size, std::shared_ptr<Texture> texture, XMFLOAT4 color)
    : position(position), size(size), texture(texture), color(color) {}

bool UI2D::Initialize(Renderer* renderer)
{
    GameObject::Initialize(renderer);

    auto shaderManager = renderer->GetShaderManager();

    shaderInfo.vertexShaderName = L"UI2DVertexShader";
    shaderInfo.pixelShaderName = L"UI2DPixelShader";

    vertexShader = shaderManager->GetVertexShader(shaderInfo.vertexShaderName);
    pixelShader = shaderManager->GetPixelShader(shaderInfo.pixelShaderName);

    materialData.diffuse = XMFLOAT3(color.x, color.y, color.z);  // UI ���� ����
    materialData.alpha = color.w;
    materialData.useTexture = (texture != nullptr) ? 1 : 0;


    quadMesh = Mesh::CreateQuad(renderer->GetDevice());

    if (!quadMesh) {
        return false;  // �޽� ���� ����
    }

    CreateDepthStencilState(renderer->GetDevice());
    CreateRasterizerState(renderer->GetDevice());

    return true;
}

void UI2D::Update(float deltaTime) {
    // UI ������Ʈ (�ִϸ��̼�, ���콺 ���� ��)
}

void UI2D::Render(Renderer* renderer) {
    if (!texture || !quadMesh) return;  // �ؽ�ó�� �޽ð� ������ ���������� ����


    auto context = renderer->GetDeviceContext();
    
    // DepthStencilState ����
    ID3D11DepthStencilState* prevDepthStencilState = nullptr;
    context->OMGetDepthStencilState(&prevDepthStencilState, nullptr);
    context->OMSetDepthStencilState(depthStencilState.Get(), 0);

    ID3D11RasterizerState* prevRasterizerState = nullptr;
    context->RSGetState(&prevRasterizerState);
    context->RSSetState(rasterizerState.Get());


    XMMATRIX view = XMMatrixIdentity(); // UI�� ī�޶� �̵�/ȸ�� ����
    XMMATRIX proj = XMMatrixOrthographicOffCenterLH(
        0.0f, static_cast<float>(renderer->GetViewportWidth()),
        static_cast<float>(renderer->GetViewportHeight()), 0.0f,
        0.0f, 1.0f
    );

    // UI ��Ҵ� ���� ȭ�鿡 ���ĵǱ� ������ billboarding�� �������� ����
    // �ܼ��� UI ��Ҹ� �׸��� ���� world ��ȯ ����� ���

    // UI�� ��ġ�� ũ�⸦ ����� ���� ��ȯ ���
    XMMATRIX world = XMMatrixScaling(size.x, size.y, 1.0f) *
        XMMatrixTranslation(position.x, position.y, 0.0f);

    // MVP ��� ���� ����
    CB_MVP mvpData;
    mvpData.model = XMMatrixTranspose(world);  // ���� ��ȯ ���
    mvpData.view = XMMatrixTranspose(view);    // ī�޶��� �� ��Ʈ����
    mvpData.projection = XMMatrixTranspose(proj);  // ī�޶��� ���� ��Ʈ����
    mvpData.modelInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));  // ���� �����

    context->UpdateSubresource(constantMVPBuffer.Get(), 0, nullptr, &mvpData, 0, 0);
    context->VSSetConstantBuffers(0, 1, constantMVPBuffer.GetAddressOf());  // ��� ���� ����

    // ��Ƽ���� ��� ���� ������Ʈ
    context->UpdateSubresource(constantMaterialBuffer.Get(), 0, nullptr, &materialData, 0, 0);
    context->PSSetConstantBuffers(2, 1, constantMaterialBuffer.GetAddressOf());  // ��Ƽ���� ���� ����

    // ���̴� ����
    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);

    // �ؽ�ó ���ε�
    if (texture) {
        ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
        if (srv) {
            ID3D11SamplerState* sampler = renderer->GetSamplerState();
            context->PSSetShaderResources(0, 1, &srv);  // ���̴��� �ؽ�ó ����
            context->PSSetSamplers(0, 1, &sampler);  // ���̴��� ���÷� ����
        }
    }

    // ����(Vertex) �� �ε���(Index) ���� ���ε�
    UINT stride = sizeof(MeshVertex);  // ������ ũ��
    UINT offset = 0;  // ������ (ù ��° �������� ����)

    // �޽��� ���� �� �ε��� ���� ���ε�
    ID3D11Buffer* vb = quadMesh->GetVertexBuffer();
    ID3D11Buffer* ib = quadMesh->GetIndexBuffer();

    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // UI2D �簢�� �׸���
    context->DrawIndexed(quadMesh->GetIndexCount(), 0, 0);  // �ε��� 6�� (�� �ﰢ������ ����)

    // ���� DepthStencilState ����
    context->OMSetDepthStencilState(prevDepthStencilState, 0);
    if (prevDepthStencilState) prevDepthStencilState->Release();

    context->RSSetState(prevRasterizerState);
    if (prevRasterizerState) prevRasterizerState->Release();
}

void UI2D::SetPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

XMFLOAT2 UI2D::GetPosition() const {
    return position;
}

void UI2D::SetSize(float width, float height) {
    size.x = width;
    size.y = height;
}

XMFLOAT2 UI2D::GetSize() const {
    return size;
}

void UI2D::SetColor(XMFLOAT4 newColor) {
    color = newColor;
}

XMFLOAT4 UI2D::GetColor() const {
    return color;
}

bool UI2D::CreateDepthStencilState(ID3D11Device* device)
{
    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = FALSE;              // ���� �׽�Ʈ ��Ȱ��ȭ
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_ALWAYS; // ���� �� �Լ� ���� (�׻� true)

    desc.StencilEnable = FALSE; // ���ٽ� ��Ȱ��ȭ

    return SUCCEEDED(device->CreateDepthStencilState(&desc, depthStencilState.GetAddressOf()));
}

bool UI2D::CreateRasterizerState(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;  // Culling ��Ȱ��ȭ
    rsDesc.FrontCounterClockwise = TRUE; // �ݽð������ �ո����� ���� (�ʿ信 ���� ����)
    rsDesc.DepthClipEnable = TRUE;

    return SUCCEEDED(device->CreateRasterizerState(&rsDesc, rasterizerState.GetAddressOf()));
}
