#pragma once

enum BULB_API KeyValue
{
	A		=	'A',
	B		=	'B',
	C		=	'C',
	D		=	'D',
	E		=	'E',
	F		=	'F',
	G		=	'G',
	H		=	'H',
	I		=	'I',
	J		=	'J',
	K		=	'K',
	L		=	'L',
	M		=	'M',
	N		=	'N',
	O		=	'O',
	P		=	'P',
	Q		=	'Q',
	R		=	'R',
	S		=	'S',
	T		=	'T',
	U		=	'U',
	V		=	'V',
	W		=	'W',
	X		=	'X',
	Y		=	'Y',
	Z		=	'Z',
	NUM_0	=	'0',
	NUM_1	=	'1',
	NUM_2	=	'2',
	NUM_3	=	'3',
	NUM_4	=	'4',
	NUM_5	=	'5',
	NUM_6	=	'6',
	NUM_7	=	'7',
	NUM_8	=	'8',
	NUM_9	=	'9',
	UP		=	VK_UP,
	DOWN	=	VK_DOWN,
	LEFT	=	VK_LEFT,
	RIGHT	=	VK_RIGHT,
	SPACE	=	VK_SPACE,
	ESC		=	VK_ESCAPE,
	F1		=	VK_F1,
	F2		=	VK_F2,
	F3		=	VK_F3,
	F4		=	VK_F4,
	F5		=	VK_F5,
	F6		=	VK_F6,
	F7		=	VK_F7,
	F8		=	VK_F8,
	F9		=	VK_F9,
	F10		=	VK_F10,
	F11		=	VK_F11,
	F12		=	VK_F12,
	CTRL	=	VK_CONTROL,
	SHIFT	=	VK_SHIFT,
	ALT		=	VK_MENU
};

enum BULB_API KeyState
{
	Idle,
	Down,
	Press,
	Up
};

struct BULB_API MouseStates
{
	KeyState LeftButton = KeyState::Idle;
	bool IsLeftButtonLazyUpdate = false;
	KeyState RightButton = KeyState::Idle;
	bool IsRightButtonLazyUpdate = false;
};

class BULB_API InputManager
{
	friend class BulbApplication;

private:
	InputManager() = default;
	~InputManager() = default;

	void Init();
	void Update();

public:
	InputManager(const InputManager& rhs) = delete;
	InputManager& operator=(const InputManager& rhs) = delete;

	static InputManager* GetInstance();
	static Bulb::ProcessResult Delete();

	bool IsKeyDown(KeyValue key);
	bool IsKeyPress(KeyValue key);

	bool IsMouseCenterFixed() { return _isMouseCenterFixed; }
	void SetMouseCenterFixMode(bool value) { _isMouseCenterFixed = value; }

	POINT GetMousePosition() { return _mousePosition; }
	Bulb::Vector2 GetMouseDelta() { return _mouseDelta; }

	bool IsMouseLeftButtonDown() {
#ifdef BULB_EDITOR
		if (ImGui::GetIO().WantCaptureMouse) return false;
#endif

		return _mouseStates.LeftButton == KeyState::Down;
	}
	bool IsMouseLeftButtonPress() {
#ifdef BULB_EDITOR
		if (ImGui::GetIO().WantCaptureMouse) return false;
#endif

		return _mouseStates.LeftButton == KeyState::Press;
	}
	bool IsMouseLeftButtonUp() {
#ifdef BULB_EDITOR
		if (ImGui::GetIO().WantCaptureMouse) return false;
#endif

		return _mouseStates.LeftButton == KeyState::Up;
	}

	bool IsMouseRightButtonDown() {
#ifdef BULB_EDITOR
		if (ImGui::GetIO().WantCaptureMouse) return false;
#endif

		return  _mouseStates.RightButton == KeyState::Down;
	}
	bool IsMouseRightButtonPress() {
#ifdef BULB_EDITOR
		if (ImGui::GetIO().WantCaptureMouse) return false;
#endif

		return _mouseStates.RightButton == KeyState::Press;
	}

	void OnMouseClick(USHORT buttonFlags);
	void OnMouseMove(int x, int y);

private:
	void MouseButtonUpdate();

private:
	static InputManager* s_instance;

	bool _isMouseMoving = false;
	bool _isMouseCenterFixed = false;
	MouseStates _mouseStates;
	POINT _mousePosition;
	Bulb::Vector2 _mouseDelta;
	unordered_map<KeyValue, KeyState> _keyStates;
};

