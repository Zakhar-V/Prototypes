#pragma once

#include "Core.hpp"

//----------------------------------------------------------------------------//
// External types
//----------------------------------------------------------------------------//

struct GLFWwindow;
struct GLFWmonitor;

namespace ge
{
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	enum KeyboardKey : uint16
	{
		KK_UNKNOWN = 0, // -1

		// printable
		KK_SPACE = 32,
		KK_APOSTROPHE = 39, // '
		KK_COMMA = 44, // ,
		KK_MINUS, // -
		KK_PERIOD, // .
		KK_SLASH, // /
		KK_0,
		KK_1,
		KK_2,
		KK_3,
		KK_4,
		KK_5,
		KK_6,
		KK_7,
		KK_8,
		KK_9,
		KK_SEMICOLON = 59, // ;
		KK_EQUAL = 61, // =
		KK_A = 65,
		KK_B,
		KK_C,
		KK_D,
		KK_E,
		KK_F,
		KK_G,
		KK_H,
		KK_I,
		KK_J,
		KK_K,
		KK_L,
		KK_M,
		KK_N,
		KK_O,
		KK_P,
		KK_Q,
		KK_R,
		KK_S,
		KK_T,
		KK_U,
		KK_V,
		KK_W,
		KK_X,
		KK_Y,
		KK_Z,
		KK_LEFT_BRACKET = 91,  // [
		KK_BACKSLASH,  // '\'
		KK_RIGHT_BRACKET,  // ]
		KK_TILDE = 96,  // `
		KK_OEM_1 = 161, // non-US #1
		KK_OEM_2, // non-US #2

		// function
		KK_ESCAPE = 256,
		KK_ENTER,
		KK_TAB,
		KK_BACKSPACE,
		KK_INSERT,
		KK_DELETE,
		KK_RIGHT,
		KK_LEFT,
		KK_DOWN,
		KK_UP,
		KK_PAGE_UP,
		KK_PAGE_DOWN,
		KK_HOME,
		KK_END,
		KK_CAPS_LOCK = 280,
		KK_SCROLL_LOCK,
		KK_NUM_LOCK,
		KK_PRINT_SCREEN,
		KK_PAUSE,
		KK_F1 = 290,
		KK_F2,
		KK_F3,
		KK_F4,
		KK_F5,
		KK_F6,
		KK_F7,
		KK_F8,
		KK_F9,
		KK_F10,
		KK_F11,
		KK_F12,
		KK_F13,
		KK_F14,
		KK_F15,
		KK_F16,
		KK_F17,
		KK_F18,
		KK_F19,
		KK_F20,
		KK_F21,
		KK_F22,
		KK_F23,
		KK_F24,
		KK_F25,
		KK_NUMPAD_0 = 320,
		KK_NUMPAD_1,
		KK_NUMPAD_2,
		KK_NUMPAD_3,
		KK_NUMPAD_4,
		KK_NUMPAD_5,
		KK_NUMPAD_6,
		KK_NUMPAD_7,
		KK_NUMPAD_8,
		KK_NUMPAD_9,
		KK_NUMPAD_DECIMAL,
		KK_NUMPAD_DIVIDE,
		KK_NUMPAD_MULTIPLY,
		KK_NUMPAD_SUBTRACT,
		KK_NUMPAD_ADD,
		KK_NUMPAD_ENTER,
		KK_NUMPAD_EQUAL,
		KK_LEFT_SHIFT = 340,
		KK_LEFT_CONTROL,
		KK_LEFT_ALT,
		KK_LEFT_SUPER,
		KK_RIGHT_SHIFT,
		KK_RIGHT_CONTROL,
		KK_RIGHT_ALT,
		KK_RIGHT_SUPER,
		KK_MENU,
		MAX_KEYBOARD_KEYS = 349,
	};

	//----------------------------------------------------------------------------//
	// Keyboard
	//----------------------------------------------------------------------------//

#define GKeyboard Keyboard::Get()

	class Keyboard : public Singleton < Keyboard >
	{
	public:

	protected:
		friend class InputSystem;

		Keyboard(void);
		~Keyboard(void);
		bool _Init(void);
		void _Destroy(void);
	};

	//----------------------------------------------------------------------------//
	// Mouse
	//----------------------------------------------------------------------------//

#define GMouse Mouse::Get()

	class Mouse : public Singleton < Mouse >
	{
	public:

	protected:
		friend class InputSystem;

		Mouse(void);
		~Mouse(void);
		bool _Init(void);
		void _Destroy(void);
	};

	//----------------------------------------------------------------------------//
	// InputSystem
	//----------------------------------------------------------------------------//

#define GInputSystem InputSystem::Get()

	class InputSystem : public Singleton < InputSystem >
	{
	public:

	protected:
		friend class Device;

		InputSystem(void);
		~InputSystem(void);
		bool _Init(void);
		void _Destroy(void);
	};

	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//

#define GDevice Device::Get()

	class Device : public Singleton < Device >
	{
	public:
		void SetVisible(bool _state = true);
		bool IsVisible(void) { return m_visible; }
		void SetFullscreen(bool _state = true);
		bool IsFullscreen(void) { return m_fullscreen; }
		bool SetVideoMode(uint _width, uint _height, uint _freq = 0);
		bool SetVideoMode(const Vec3ui& _mode) { return SetVideoMode(_mode.x, _mode.y, _mode.z); }
		const Vec3ui& GetVideoMode(void) { return m_videoMode; }
		const Vec2& GetSize(void) { return m_size; }
		const Vec2ui& GetSizei(void) { return m_sizei; }
		bool IsActive(void) { return m_active; }
		bool PollEvents(void);
		void Close(void);
		bool ShouldClose(void) { return m_shouldClose; }

		double _Time();	///\todo remove

	protected:
		friend class Engine;
		friend class RenderSystem;

		Device(void);
		~Device(void);
		bool _Init(void);
		void _Destroy(void);

		void _BeforeChangingContexts(void);
		void _AfterChangingContexts(void);
		void _SwapBuffers(void);
		void _CheckWindowState(void);
		void _ChooseFullscreenParams(void);
		static void _ErrorCallback(int _code, const char* _err);
		static void _MonitorCallback(GLFWmonitor* _monitor, int _event);
		static void _WindowPosCallback(GLFWwindow* _wnd, int _x, int _y);
		static void _WindowSizeCallback(GLFWwindow* _wnd, int _w, int _h);
		static void _WindowCloseCallback(GLFWwindow* _wnd);
		static void _WindowRefreshCallback(GLFWwindow* _wnd);
		static void _WindowFocusCallback(GLFWwindow* _wnd, int _focused);
		static void _WindowIconifyCallback(GLFWwindow* _wnd, int _iconfied);
		static void _WindowFramebufferSizeCallback(GLFWwindow* _wnd, int _w, int _h);
		static void _ButtonCallback(GLFWwindow* _wnd, int _button, int _action, int _mods);
		static void _CursorPosCallback(GLFWwindow* _wnd, double _x, double _y);
		static void _CursorEnterCallback(GLFWwindow* _wnd, int _entered);
		static void _ScrollCallback(GLFWwindow* _wnd, double _x, double _y);
		static void _KeyCallback(GLFWwindow* _wnd, int _key, int _scancode, int _action, int _mods);
		static void _CharCallback(GLFWwindow* _wnd, unsigned int _char);
		static void _SetupCallbacks(GLFWwindow* _wnd);

		GLFWwindow* m_currentWindow;
		GLFWwindow* m_mainWindow;
		GLFWwindow* m_fullscreenWindow;
		void* m_fullscreenDC;
		GLFWmonitor* m_monitor;
		bool m_active;
		bool m_visible;
		bool m_fullscreen;
		bool m_shouldClose;
		Vec3ui m_videoMode;
		Vec2 m_size;
		Vec2ui m_sizei;
		bool m_focus;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
