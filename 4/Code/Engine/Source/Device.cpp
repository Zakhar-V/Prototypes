#include "../Device.hpp"
#include "../Graphics.hpp"
#include <SDL.h>

namespace Rx
{
	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	Device::Device(void) :
		m_opened(false),
		m_window(nullptr),
		m_size(0)
	{
		LOG_EVENT("Create Device");

		/*if (!gRenderSystem)
		{
			throw std::exception("Unable to create device : The render system must be created before it");
		}*/

		if (SDL_Init(SDL_INIT_VIDEO))
		{
			throw std::exception(SDL_GetError());
		}
		//m_window = gRenderSystem->GetSDLWindow();

		SDL_DisplayMode _dm;
		SDL_GetDesktopDisplayMode(0, &_dm);
		m_size.Set((_dm.w >> 2) * 3, (_dm.h >> 2) * 3);
		SDL_SetWindowSize(m_window, m_size.x, m_size.y);
		SDL_SetWindowPosition(m_window, _dm.w >> 3, _dm.h >> 3);

		m_opened = true;
	}
	//----------------------------------------------------------------------------//
	Device::~Device(void)
	{
		LOG_EVENT("Destroy Device");

	}
	//----------------------------------------------------------------------------//
	void Device::SetVisible(bool _state)
	{
		if (_state)
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
