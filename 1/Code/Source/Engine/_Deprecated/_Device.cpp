#include "Device.hpp"
#include <GLFW/glfw3.h>
#ifdef WIN32
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#	include <GLFW/glfw3native.h>
#endif
COMPILER_MESSAGE("linker", "glfw.lib");
#pragma comment(lib, "glfw.lib")

#define USE_GLFW_CONTEXTS_HACK // при переключении в полноэкранный режим, активный контекст opengl не изменяется

namespace ge
{
	//----------------------------------------------------------------------------//
	// Keyboard
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Keyboard::Keyboard(void)
	{

	}
	//----------------------------------------------------------------------------//
	Keyboard::~Keyboard(void)
	{

	}
	//----------------------------------------------------------------------------//
	bool Keyboard::_Init(void)
	{
		LOG_NODE("Initializing Keyboard");

		return true;
	}
	//----------------------------------------------------------------------------//
	void Keyboard::_Destroy(void)
	{
		LOG_NODE("Destroying Keyboard");
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Mouse
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Mouse::Mouse(void)
	{

	}
	//----------------------------------------------------------------------------//
	Mouse::~Mouse(void)
	{

	}
	//----------------------------------------------------------------------------//
	bool Mouse::_Init(void)
	{
		LOG_NODE("Initializing Mouse");

		return true;
	}
	//----------------------------------------------------------------------------//
	void Mouse::_Destroy(void)
	{
		LOG_NODE("Destroying Mouse");
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// InputSystem
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	InputSystem::InputSystem(void)
	{
		new Keyboard;
		new Mouse;
	}
	//----------------------------------------------------------------------------//
	InputSystem::~InputSystem(void)
	{
		delete GMouse;
		delete GKeyboard;
	}
	//----------------------------------------------------------------------------//
	bool InputSystem::_Init(void)
	{
		LOG_NODE("Initializing InputSystem");

		if (!GKeyboard->_Init())
		{
			LOG_ERROR("Couldn't initialize Keyboard");
			return false;
		}

		if (!GMouse->_Init())
		{
			LOG_ERROR("Couldn't initialize Mouse");
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void InputSystem::_Destroy(void)
	{
		LOG_NODE("Destroying InputSystem");

		GMouse->_Destroy();
		GKeyboard->_Destroy();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

	namespace
	{
		int _TranslateKey(int _glfw)
		{
			return 0;
		}

		int _TranslateKeyMods(int _glfw)
		{
			return 0;
		}

		GLFWmonitor* _GetMonitorByPoint(int _px, int _py)
		{
			int _count = 0;
			GLFWmonitor** _monitors = glfwGetMonitors(&_count);
			for (int i = 0; i < _count; ++i)
			{
				int _x, _y;
				glfwGetMonitorPos(_monitors[i], &_x, &_y);
				const GLFWvidmode* _mode = glfwGetVideoMode(_monitors[i]);
				if (_mode && _px >= _x && _px <= _x + _mode->width && _py >= _y && _py <= _y + _mode->height) return _monitors[i];
			}
			return 0;
		}

		GLFWmonitor* _GetMonitorForWindow(GLFWwindow* _wnd)
		{
			int _x, _y, _w, _h;
			glfwGetWindowPos(_wnd, &_x, &_y);
			glfwGetWindowSize(_wnd, &_w, &_h);
			return _GetMonitorByPoint(_x + (_w >> 1), _y + (_h >> 1));
		}
	}

	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//

	double Device::_Time() { return glfwGetTime(); }

	//----------------------------------------------------------------------------//
	Device::Device(void) :
		m_active(false),
		m_visible(false),
		m_fullscreen(false),
		m_shouldClose(false),
		m_size(Vec2::ZERO),
		m_sizei(Vec2ui::ZERO),
		m_videoMode(Vec3ui::ZERO),
		m_focus(false),
		m_currentWindow(0),
		m_mainWindow(0),
		m_fullscreenWindow(0),
		m_fullscreenDC(0),
		m_monitor(0)
	{
		new InputSystem;
	}
	//----------------------------------------------------------------------------//
	Device::~Device(void)
	{
		delete GInputSystem;
	}
	//----------------------------------------------------------------------------//
	bool Device::_Init(void)
	{
		LOG_NODE("Initializing Device");

		// glfw
		LOG_INFO("GLFW %s", glfwGetVersionString());
		glfwSetErrorCallback(_ErrorCallback);
		if (!glfwInit())
		{
			LOG_ERROR("Couldn't initialize of GLFW");
			return false;
		}

		// monitor
		glfwSetMonitorCallback(_MonitorCallback);
		GLFWmonitor* _monitor = glfwGetPrimaryMonitor();
		if (!_monitor)
		{
			LOG_ERROR("No primary monitor");
			return false;
		}

		// videomode
		const GLFWvidmode* _videoMode = glfwGetVideoMode(_monitor);
		if (!_videoMode)
		{
			LOG_ERROR("No video modes");
			return false;
		}
		m_videoMode.x = _videoMode->width;
		m_videoMode.y = _videoMode->height;
		m_videoMode.z = _videoMode->refreshRate;

		// hints
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_VISIBLE, 0);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

		// main window
		m_mainWindow = glfwCreateWindow((_videoMode->width * 3) / 4, (_videoMode->height * 3) / 4, "Game", 0, 0);
		if (!m_mainWindow)
		{
			LOG_ERROR("Couldn't create main window");
			return false;
		}
		_SetupCallbacks(m_mainWindow);
		m_currentWindow = m_mainWindow;
		int _w, _h; 
		glfwGetWindowSize(m_currentWindow, &_w, &_h); 
		m_sizei.Set(_w, _h);
		m_size = m_sizei.Cast<Vec2i>().Cast<Vec2>();
		glfwMakeContextCurrent(m_currentWindow);

		if (!GInputSystem->_Init())
		{
			LOG_ERROR("Couldn't initialize InputSystem");
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void Device::_Destroy(void)
	{
		LOG_NODE("Destroying Device");

		GInputSystem->_Destroy();

		m_currentWindow = 0;
		if (m_fullscreenWindow)
		{
#ifdef USE_GLFW_CONTEXTS_HACK
			ReleaseDC(glfwGetWin32Window(m_fullscreenWindow), (HDC)m_fullscreenDC);
#endif
			glfwMakeContextCurrent(m_fullscreenWindow);
			glfwIconifyWindow(m_fullscreenWindow);
			glfwDestroyWindow(m_fullscreenWindow);
		}
		if (m_mainWindow) glfwDestroyWindow(m_mainWindow);
	}
	//----------------------------------------------------------------------------//
	void Device::_BeforeChangingContexts(void)
	{
		TODO("Уведомлять систему рендеринга");
		//BeforeChangingContexts();
	}
	//----------------------------------------------------------------------------//
	void Device::_AfterChangingContexts(void)
	{
		TODO("Уведомлять систему рендеринга");
		//AfterChangingContexts();
	}
	//----------------------------------------------------------------------------//
	void Device::_SwapBuffers(void)
	{
		glfwSwapBuffers(m_currentWindow);
	}
	//----------------------------------------------------------------------------//
	void Device::_CheckWindowState(void)
	{
		int _width, _height;
		glfwGetWindowSize(m_currentWindow, &_width, &_height);
		if (_width != m_sizei.x || _height != m_sizei.y)
		{
			_WindowSizeCallback(m_currentWindow, _width, _height);
			_WindowFramebufferSizeCallback(m_currentWindow, _width, _height);
		}

		bool _focus = glfwGetWindowAttrib(m_currentWindow, GLFW_FOCUSED) != 0;
		if (_focus != m_active) _WindowFocusCallback(m_currentWindow, _focus);
	}
	//----------------------------------------------------------------------------//
	void Device::_ChooseFullscreenParams(void)
	{
		m_monitor = _GetMonitorForWindow(m_mainWindow);
		if (m_monitor)
		{
			///\todo выбирать монитор по айди
			///\todo подбирать режим под конкретный монитор
			///\todo устанавливать частоту обновления
			const GLFWvidmode* _videoMode = glfwGetVideoMode(m_monitor);
			if (_videoMode)
			{
				m_videoMode.x = _videoMode->width;
				m_videoMode.y = _videoMode->height;
				m_videoMode.z = _videoMode->refreshRate;
			}
			else
			{
				m_monitor = 0;
				LOG_ERROR("Device : Couldn't choose video mode");
			}
		}
		else
		{
			m_monitor = glfwGetPrimaryMonitor();
			LOG_WARNING("Device : Couldn't choose monitor; use primary");
		}
	}
	//----------------------------------------------------------------------------//
	void Device::_ErrorCallback(int _code, const char* _err)
	{
		LOG_ERROR("GLFW : %s", _err);
	}
	//----------------------------------------------------------------------------//
	void Device::_MonitorCallback(GLFWmonitor* _monitor, int _event)
	{
		if (_event == GLFW_DISCONNECTED)
		{
			LOG_INFO("Device : Disconnect monitor");
			if (s_instance->m_fullscreenWindow)
			{
				if (_monitor == s_instance->m_monitor)
				{
					s_instance->SetFullscreen(false);
				}
				else
				{
					glfwIconifyWindow(s_instance->m_fullscreenWindow);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void Device::_WindowPosCallback(GLFWwindow* _wnd, int _x, int _y)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_WindowSizeCallback(GLFWwindow* _wnd, int _w, int _h)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_WindowCloseCallback(GLFWwindow* _wnd)
	{
		s_instance->m_shouldClose = true;
	}
	//----------------------------------------------------------------------------//
	void Device::_WindowRefreshCallback(GLFWwindow* _wnd)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_WindowFocusCallback(GLFWwindow* _wnd, int _focused)
	{
		if (_focused)
		{
			s_instance->m_active = true;
			s_instance->m_focus = true;
		}
		else
		{
			s_instance->m_focus = false;
			s_instance->m_active = false;
		}
	}
	//----------------------------------------------------------------------------//
	void Device::_WindowIconifyCallback(GLFWwindow* _wnd, int _iconfied)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_WindowFramebufferSizeCallback(GLFWwindow* _wnd, int _w, int _h)
	{
		s_instance->m_size = s_instance->m_sizei.Set(Max(_w, 0), Max(_h, 0)).Cast<Vec2>();
	}
	//----------------------------------------------------------------------------//
	void Device::_ButtonCallback(GLFWwindow* _wnd, int _button, int _action, int _mods)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_CursorPosCallback(GLFWwindow* _wnd, double _x, double _y)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_CursorEnterCallback(GLFWwindow* _wnd, int _entered)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_ScrollCallback(GLFWwindow* _wnd, double _x, double _y)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_KeyCallback(GLFWwindow* _wnd, int _key, int _scancode, int _action, int _mods)
	{
		if (_action == 0 && _key == KK_ENTER && (_mods & GLFW_MOD_ALT))
		{
			GDevice->SetFullscreen(!GDevice->IsFullscreen());
		}
	}
	//----------------------------------------------------------------------------//
	void Device::_CharCallback(GLFWwindow* _wnd, unsigned int _char)
	{

	}
	//----------------------------------------------------------------------------//
	void Device::_SetupCallbacks(GLFWwindow* _wnd)
	{
		glfwSetWindowPosCallback(_wnd, _WindowPosCallback);
		glfwSetWindowSizeCallback(_wnd, _WindowSizeCallback);
		glfwSetWindowCloseCallback(_wnd, _WindowCloseCallback);
		glfwSetWindowRefreshCallback(_wnd, _WindowRefreshCallback);
		glfwSetWindowFocusCallback(_wnd, _WindowFocusCallback);
		glfwSetWindowIconifyCallback(_wnd, _WindowIconifyCallback);
		glfwSetFramebufferSizeCallback(_wnd, _WindowFramebufferSizeCallback);
		glfwSetMouseButtonCallback(_wnd, _ButtonCallback);
		glfwSetCursorPosCallback(_wnd, _CursorPosCallback);
		glfwSetCursorEnterCallback(_wnd, _CursorEnterCallback);
		glfwSetScrollCallback(_wnd, _ScrollCallback);
		glfwSetKeyCallback(_wnd, _KeyCallback);
		glfwSetCharCallback(_wnd, _CharCallback);
	}
	//----------------------------------------------------------------------------//
	void Device::SetVisible(bool _state)
	{
		if (m_visible != _state)
		{
			_state ? glfwShowWindow(m_currentWindow) : glfwHideWindow(m_currentWindow);
			m_visible = glfwGetWindowAttrib(m_currentWindow, GLFW_VISIBLE) != 0;
		}
	}
	//----------------------------------------------------------------------------//
	void Device::SetFullscreen(bool _state)
	{
		if (_state)
		{
			if (m_currentWindow != m_fullscreenWindow)
			{
				LOG_DEBUG("Change to fullscreen mode");

				_ChooseFullscreenParams();
				if (m_monitor)
				{
					m_fullscreenWindow = glfwCreateWindow(m_videoMode.x, m_videoMode.y, "Game", m_monitor, m_mainWindow);
					if (m_fullscreenWindow)
					{
						_SetupCallbacks(m_fullscreenWindow);
						glfwHideWindow(m_currentWindow);
						m_currentWindow = m_fullscreenWindow;
						m_fullscreen = true;
						m_visible = glfwGetWindowAttrib(m_currentWindow, GLFW_VISIBLE) != 0;
						_BeforeChangingContexts();
						glfwMakeContextCurrent(m_fullscreenWindow);
#ifdef USE_GLFW_CONTEXTS_HACK
						m_fullscreenDC = GetDC(glfwGetWin32Window(m_fullscreenWindow));
						wglMakeCurrent((HDC)m_fullscreenDC, glfwGetWGLContext(m_mainWindow)); // hack
#endif
						_AfterChangingContexts();
						_CheckWindowState();
					}
					else
					{
						LOG_ERROR("Device : Couldn't create of fullscreen window");
					}
				}
			}
		}
		else if (m_currentWindow == m_fullscreenWindow)
		{
			LOG_DEBUG("Change to windowed mode");

			_BeforeChangingContexts();
#ifdef USE_GLFW_CONTEXTS_HACK
			ReleaseDC(glfwGetWin32Window(m_fullscreenWindow), (HDC)m_fullscreenDC);
#endif
			glfwMakeContextCurrent(m_fullscreenWindow);
			glfwIconifyWindow(m_fullscreenWindow);
			glfwDestroyWindow(m_fullscreenWindow);
			m_fullscreenWindow = 0;
			m_fullscreen = false;
			m_monitor = 0;
			m_currentWindow = m_mainWindow;
			glfwMakeContextCurrent(m_currentWindow);
			_AfterChangingContexts();
			_CheckWindowState();
			m_visible ? glfwShowWindow(m_currentWindow) : glfwHideWindow(m_currentWindow);
		}
	}
	//----------------------------------------------------------------------------//
	bool Device::SetVideoMode(uint _width, uint _height, uint _freq)
	{
		return false;
	}
	//----------------------------------------------------------------------------//
	bool Device::PollEvents(void)
	{
		glfwPollEvents();
		return !m_shouldClose;
	}
	//----------------------------------------------------------------------------//
	void Device::Close(void)
	{
		m_shouldClose = true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
