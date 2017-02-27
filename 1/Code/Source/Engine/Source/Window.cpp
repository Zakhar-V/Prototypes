#include "Window.hpp"
#include "Render.hpp"
#include "Log.hpp"
#include <SDL.h>

namespace ge
{

	//----------------------------------------------------------------------------//
	// Window
	//----------------------------------------------------------------------------//
	

	//----------------------------------------------------------------------------//
	Window::Window(SDL_Window* _handle) :
		m_handle(_handle),
		m_size(0),
		m_isOpened(true)
	{
		if (!m_handle)
			LOG_FATAL("No window handle");
		ASSERT(m_handle != nullptr, "No window handle");

		SDL_SetWindowData(m_handle, "ge::Window", this);
	}
	//----------------------------------------------------------------------------//
	Window::~Window(void)
	{
		if (!Thread::IsMain())
			LOG_ERROR("Window can be destroyed only in main thread");
		
		SDL_DestroyWindow(m_handle);
	}
	//----------------------------------------------------------------------------//
	void Window::SetVisible(bool _visible)
	{
		if (_visible)
			SDL_ShowWindow(m_handle);
		else
			SDL_HideWindow(m_handle);
	}
	//----------------------------------------------------------------------------//
	bool Window::IsVisible(void)
	{
		return (SDL_GetWindowFlags(m_handle) & SDL_WINDOW_SHOWN) != 0;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// WindowSystem
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	WindowSystem::WindowSystem(void)
	{
	}
	//----------------------------------------------------------------------------//
	WindowSystem::~WindowSystem(void)
	{
		// ...
	}
	//----------------------------------------------------------------------------//
	WindowSystemPtr WindowSystem::Create(void)
	{
		if (!s_instance)
		{
			LOG_NODE("WindowSystem::Create");

			if (!Thread::IsMain())
			{
				LOG_ERROR("WindowSystem can be created only in main thread");
				return nullptr;
			}

			if (SDL_Init(SDL_INIT_VIDEO))
			{
				LOG_ERROR("Couldn't initialize SDL2 video : %s", SDL_GetError());
				return nullptr;
			}

			return new WindowSystem;
		}
		return s_instance;
	}
	//----------------------------------------------------------------------------//
	WindowPtr WindowSystem::CreateWindow(void* _externalWindow)
	{
		LOG_NODE_DEFERRED("WindowSystem::CreateWindow");

		if (!Thread::IsMain())
		{
			LOG_ERROR("Window can be created only in main thread");
			return nullptr;
		}

		if (!gRenderSystem)
		{
			LOG_ERROR("Couldn't create window: No RenderSystem");
			return nullptr;
		}

		return gRenderSystem ->CreateWindow(_externalWindow);
	}
	//----------------------------------------------------------------------------//
	Window* WindowSystem::GetWindow(uint _id)
	{
		SDL_Window* _handle = SDL_GetWindowFromID(_id);
		if (_handle)
			return reinterpret_cast<Window*>(SDL_GetWindowData(_handle, "ge::Window"));
		return nullptr;
	}
	//----------------------------------------------------------------------------//
	void WindowSystem::PollEvents(void)
	{
		SDL_Event _event;
		while (SDL_PollEvent(&_event))
		{
			// temp
			if (_event.type == SDL_WINDOWEVENT)
			{
				if (_event.window.event == SDL_WINDOWEVENT_CLOSE)
				{
					Window* _wnd = GetWindow(_event.window.windowID);
					if (_wnd)
						_wnd->m_isOpened = false;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
