#include "Window.hpp"
#include <SDL.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Window
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Window::Window(SDL_Window* _window) :
		m_window(_window),
		m_width(0),
		m_height(0),
		m_isOpened(true)
	{
		ASSERT(_window != nullptr);

		SDL_DisplayMode _dm;
		SDL_GetDesktopDisplayMode(0, &_dm);
		SDL_SetWindowSize(m_window, (_dm.w >> 2) * 3, (_dm.h >> 2) * 3);
		SDL_SetWindowPosition(m_window, _dm.w >> 3, _dm.h >> 3);
	}
	//----------------------------------------------------------------------------//
	Window::~Window(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Window::SetVisible(bool _visible)
	{
		if (_visible)
			SDL_ShowWindow(m_window);
		else
			SDL_HideWindow(m_window);
	}
	//----------------------------------------------------------------------------//
	bool Window::IsVisible(void)
	{
		return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_SHOWN) != 0;
	}
	//----------------------------------------------------------------------------//
	void Window::PollEvents(void)
	{
		SDL_Event _event;
		while (SDL_PollEvent(&_event))
		{
			if (_event.window.windowID != SDL_GetWindowID(m_window))
				continue;

			// temp
			if (_event.type == SDL_WINDOWEVENT)
			{
				if (_event.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					s_instance->m_isOpened = false;
				}
				else if (_event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					s_instance->m_width = _event.window.data1;
					s_instance->m_height = _event.window.data2;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
