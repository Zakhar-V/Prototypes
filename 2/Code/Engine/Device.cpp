#include "Device.hpp"
#include "GraphicsCore.hpp"
#include <SDL.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool Device::_Create(void)
	{
		LOG_EVENT("Create Device");

		new Device;
		if (!s_instance->_Init())
		{
			LOG_ERROR("Couldn't initialize Device");
			delete s_instance;
			return false;
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	void Device::_Destroy(void)
	{
		if (s_instance)
		{
			LOG_EVENT("Destroy Device");
			delete s_instance;
		}
	}
	//----------------------------------------------------------------------------//
	Device::Device(void)
	{
	}
	//----------------------------------------------------------------------------//
	Device::~Device(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool Device::_Init(void)
	{
		if (!gRenderContext)
		{
			LOG_ERROR("No RenderContext");
			return false;
		}

		m_window = gRenderContext->_GetSDLWindow();
		if (!m_window)
		{
			LOG_ERROR("No Window");
			return false;
		}

		SDL_DisplayMode _dm;
		SDL_GetDesktopDisplayMode(0, &_dm);
		m_size.Set((_dm.w >> 2) * 3, (_dm.h >> 2) * 3);
		SDL_SetWindowSize(m_window, m_size.x, m_size.y);
		SDL_SetWindowPosition(m_window, _dm.w >> 3, _dm.h >> 3);

		m_opened = true;

		return true;
	}
	//----------------------------------------------------------------------------//
	void Device::_BeginFrame(void)
	{
		PollEvents();
	}
	//----------------------------------------------------------------------------//
	void Device::_EndFrame(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Device::SetVisible(bool _visible)
	{
		if (_visible)
			SDL_ShowWindow(m_window);
		else
			SDL_HideWindow(m_window);
	}
	//----------------------------------------------------------------------------//
	bool Device::IsVisible(void)
	{
		return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_SHOWN) != 0;
	}
	//----------------------------------------------------------------------------//
	void Device::PollEvents(void)
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
					s_instance->m_opened = false;
				}
				else if (_event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					s_instance->m_size.Set(_event.window.data1, _event.window.data2);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
