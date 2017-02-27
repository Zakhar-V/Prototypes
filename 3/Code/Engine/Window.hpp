#pragma once

#include "Base.hpp"

struct SDL_Window;
union SDL_Event;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gWindow Engine::Window::Get()

	//----------------------------------------------------------------------------//
	// Window
	//----------------------------------------------------------------------------//

	/// The main window. Created by ge::RenderSystem
	class Window final : public Singleton<Window>
	{
	public:

		void SetVisible(bool _visible = true);
		bool IsVisible(void);
		bool IsOpened(void) { return m_isOpened; }

		uint GetWidth(void) { return m_width; }
		uint GetHeight(void) { return m_height; }

		void PollEvents(void);

	protected:
		friend class RenderSystem;

		Window(SDL_Window* _window);
		~Window(void);

		SDL_Window* m_window;
		uint m_width;
		uint m_height;
		bool m_isOpened;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

