#pragma once
#include <windows.h>
#include <DirectXMath.h>

using namespace DirectX;

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2
};

class InputManager {
public:
    static InputManager& GetInstance() {
        static InputManager instance;
        return instance;
    }

    void Initialize(HWND hwnd);

    // Ű�Է� ����
    void OnKeyDown(UINT nChar);
    void OnKeyUp(UINT nChar);

    bool IsKeyHeld(UINT nChar) const;
    bool IsKeyJustPressed(UINT nChar) const;

    // �¿�Ű�� ���� double press ���� ��ȯ
    bool IsDoublePressedLeft() const;
    bool IsDoublePressedRight() const;

    // ���콺 �Է� ó��
    void OnMouseMove(int x, int y);
    void OnMouseDown(MouseButton button);   // button: 0-��Ŭ��, 1-��Ŭ��, 2-��
    void OnMouseUp(MouseButton button);

    void OnRawMouseMove(int dx, int dy);

    // ���콺 ���� Ȯ�� �޼���
    int GetMouseX() const;
    int GetMouseY() const;
    bool IsMouseButtonDown(MouseButton button) const;
    bool IsMouseButtonUp(MouseButton button) const;
    bool IsMouseButtonReleased(MouseButton button) const;       // "������ ��������" Ȯ���ϴ� �Լ�

    int GetMouseDeltaX() const;
    int GetMouseDeltaY() const;

    XMFLOAT2 GetMouseDirectionFromCenter();

    void SetCursorHidden(bool hide);
    void SetCursorCentor();

    void Update(float deltaTime);

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    HWND hwnd = nullptr;

    // Ű �Է� ����
    bool keyStates[256] = { false };
    bool prevKeyStates[256] = { false };

    // ���콺 �Է� ����
    int mouseX = 0;
    int mouseY = 0;
    bool mouseButtonStates[3] = { false, false, false };
    bool prevMouseButtonStates[3] = { false, false, false };

    int mouseDeltaX = 0;
    int mouseDeltaY = 0;

    // ���콺 �̵������� ���� Raw Input ���� ����
    int rawMouseDeltaX = 0;
    int rawMouseDeltaY = 0;

    bool lockMouseToCenter = false;
    bool ignoreNextMouseDelta = false;


    // �¿� Ű�� ���� double press Ÿ�̸� (ms ����)
    float leftKeyTimer = 0.0f;
    float rightKeyTimer = 0.0f;


    bool leftKeyComboInputActive = false;
    bool rightKeyComboInputActive = false;

    bool leftKeyDoublePressed = false;
    bool rightKeyDoublePressed = false;


    // �����Է� �Ӱ�ġ (ms ����)
    static constexpr float DOUBLE_PRESS_THRESHOLD = 200.0f;
};
