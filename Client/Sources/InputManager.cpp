#include "InputManager.h"
#include <Windows.h>
#include <algorithm>

void InputManager::Initialize(HWND hwnd)
{
    this->hwnd = hwnd;

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // �Ϲ� ����ũž
    rid.usUsage = 0x02;     // ���콺
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void InputManager::OnKeyDown(UINT nChar) {
    if (nChar < 256)
        keyStates[nChar] = true;
}

void InputManager::OnKeyUp(UINT nChar) {
    if (nChar < 256)
        keyStates[nChar] = false;
}
bool InputManager::IsKeyHeld(UINT nChar) const {
    return (nChar < 256) ? keyStates[nChar] : false;
}

bool InputManager::IsKeyJustPressed(UINT nChar) const {
    if (nChar >= 256)
        return false;

    // ���Ӱ� ������ �ľ�
    return keyStates[nChar] && !prevKeyStates[nChar];
}

void InputManager::Update(float deltaTime) {

    // ���� �����ӿ��� �ʱ�ȭó��
    // ���� ĳ���� Update -> InputMangaer Update ������ ó������ �����ϸ� ��

    mouseDeltaX = rawMouseDeltaX;
    mouseDeltaY = rawMouseDeltaY;

    rawMouseDeltaX = 0;
    rawMouseDeltaY = 0;


    leftKeyDoublePressed = false;
    rightKeyDoublePressed = false;


    // ���� Ű(VK_LEFT) ���� �Է� Ȯ��
    if (IsKeyJustPressed(VK_LEFT)) {

        if (leftKeyComboInputActive && leftKeyTimer < DOUBLE_PRESS_THRESHOLD)
        {
            leftKeyComboInputActive = false;
            leftKeyDoublePressed = true;
        }

        else
        {
            leftKeyDoublePressed = false;
        }

        leftKeyComboInputActive = true;
        leftKeyTimer = 0.0f;
    }


    else {
        leftKeyTimer += deltaTime;

        if (leftKeyTimer >= DOUBLE_PRESS_THRESHOLD)
        {
            leftKeyComboInputActive = false;
            leftKeyDoublePressed = false;
        }
    }


    // ���� Ű(VK_RIGHT) ���� �Է� Ȯ��
    if (IsKeyJustPressed(VK_RIGHT)) {

        if (rightKeyComboInputActive && rightKeyTimer < DOUBLE_PRESS_THRESHOLD)
        {
            rightKeyComboInputActive = false;
            rightKeyDoublePressed = true;
        }

        else
        {
            rightKeyDoublePressed = false;
        }

        rightKeyComboInputActive = true;
        rightKeyTimer = 0.0f;
    }


    else {
        rightKeyTimer += deltaTime;

        if (rightKeyTimer >= DOUBLE_PRESS_THRESHOLD)
        {
            rightKeyComboInputActive = false;
            rightKeyDoublePressed = false;
        }
    }




    // ���� �������� Ű ���� ������Ʈ
    for (int i = 0; i < 256; ++i)
    {
        prevKeyStates[i] = keyStates[i];
    }

    // ���� �������� ���콺 ���� ����
    for (int i = 0; i < 3; ++i)
    {
        prevMouseButtonStates[i] = mouseButtonStates[i];
    }

}


bool InputManager::IsDoublePressedLeft() const {
    return leftKeyDoublePressed;
}

bool InputManager::IsDoublePressedRight() const {
    return rightKeyDoublePressed;
}

void InputManager::OnMouseMove(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

void InputManager::OnMouseDown(MouseButton button)
{
    int index = static_cast<int>(button);
    if (index < 3) {
        mouseButtonStates[index] = true;
    }
}

void InputManager::OnMouseUp(MouseButton button)
{
    int index = static_cast<int>(button);
    if (index < 3) {
        mouseButtonStates[index] = false;
    }
}

void InputManager::OnRawMouseMove(int dx, int dy)
{
    rawMouseDeltaX += dx;
    rawMouseDeltaY += dy;
}

int InputManager::GetMouseX() const
{
    return mouseX;
}

int InputManager::GetMouseY() const {
    return mouseY;
}

bool InputManager::IsMouseButtonDown(MouseButton button) const {
    int index = static_cast<int>(button);
    return (index < 3) ? mouseButtonStates[index] : false;
}

bool InputManager::IsMouseButtonUp(MouseButton button) const
{
    int index = static_cast<int>(button);
    return (index < 3) ? !mouseButtonStates[index] : false;
}

bool InputManager::IsMouseButtonReleased(MouseButton button) const
{
    int index = static_cast<int>(button);
    return (index < 3) ? (!mouseButtonStates[index] && prevMouseButtonStates[index]) : false;
}

int InputManager::GetMouseDeltaX() const
{
    return mouseDeltaX;
}

int InputManager::GetMouseDeltaY() const
{
    return mouseDeltaY;
}

XMFLOAT2 InputManager::GetMouseDirectionFromCenter()
{
    if (!hwnd) return XMFLOAT2(0, 0);

    RECT rect;
    GetClientRect(hwnd, &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    POINT mousePos = { 0 };
    GetCursorPos(&mousePos);               // ���콺 ��ġ�� ��ũ�� ��ǥ ��������
    ScreenToClient(hwnd, &mousePos);       // �� Ŭ���̾�Ʈ(������ ����) ��ǥ�� ��ȯ

    float centerX = screenWidth * 0.5f;
    float centerY = screenHeight * 0.5f;

    float normX = (mousePos.x - centerX) / centerX;
    float normY = (centerY - mousePos.y) / centerY;  // Y ����

    // Ŭ���� (�ִ� ���� �ʰ� ����)
    normX = std::clamp(normX, -1.0f, 1.0f);
    normY = std::clamp(normY, -1.0f, 1.0f);

    return XMFLOAT2(normX, normY);
}

void InputManager::SetCursorHidden(bool hide)
{
    ShowCursor(!hide);
}

void InputManager::SetCursorCentor()
{
    if (!hwnd) return;

    RECT rect;
    GetClientRect(hwnd, &rect);
    POINT center;
    center.x = (rect.right - rect.left) / 2;
    center.y = (rect.bottom - rect.top) / 2;

    ClientToScreen(hwnd, &center);
    SetCursorPos(center.x, center.y);
}
