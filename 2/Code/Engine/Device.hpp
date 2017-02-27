#pragma once

#include "Core.hpp"

struct SDL_Window;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class InputReceiver
	{
	public:

		virtual void Begin(void) { }
		virtual void End(void) { }
		virtual void Focus(bool _focus) { }
		virtual void KeyDown() { }
		virtual void KeyUp(void) { }
		virtual void KeyRepeat(void) { }
		virtual void OnChar(char _ch) { }


	protected:
		friend class Device;

		bool m_active;
	};

	//----------------------------------------------------------------------------//
	// Device
	//----------------------------------------------------------------------------//

#define gDevice Engine::Device::Get()

	class Device final : public Singleton<Device>
	{
	public:

		void SetVisible(bool _visible = true);
		bool IsVisible(void);
		bool IsOpened(void) { return m_opened; }

		void PollEvents(void);

	protected:
		friend class System;

		static bool _Create(void);
		static void _Destroy(void);

		Device(void);
		~Device(void);
		bool _Init(void);

		void _BeginFrame(void);
		void _EndFrame(void);

		SDL_Window* m_window = nullptr; // created by RenderContext
		Vec2ui m_size = 0;
		bool m_opened = false;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
