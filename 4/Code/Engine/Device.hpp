#pragma once

#include "Base.hpp"
#include "Object.hpp"
#include "Math.hpp"

struct SDL_Window;

namespace Rx
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	/*class Window : public Object
	{

	};*/

	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//

#define gDevice Rx::Device::Get()

	class Device final : public Singleton<Device>
	{
	public:
		Device(void);
		~Device(void);

		void SetVisible(bool _state = true);
		bool IsVisible(void);
		bool IsOpened(void) { return m_opened; }
		void PollEvents(void);
		const Vec2ui GetSize(void) { return m_size; }

	protected:

		bool m_opened;
		SDL_Window* m_window;
		Vec2ui m_size;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
