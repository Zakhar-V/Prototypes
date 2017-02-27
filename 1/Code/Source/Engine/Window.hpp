#pragma once

#include "Object.hpp"
#include "Math.hpp"

struct SDL_Window;
union SDL_Event;

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gWindowSystem ge::WindowSystem::Get()

	class Texture;
	typedef Ptr<class Window> WindowPtr;
	typedef Ptr<class WindowSystem> WindowSystemPtr;

	//----------------------------------------------------------------------------//
	// Window
	//----------------------------------------------------------------------------//

	class ENGINE_API Window : public Object
	{
	public:
		OBJECT(Window);

		friend class WindowSystem; // temp
		void SetVisible(bool _visible = true);
		bool IsVisible(void);
		bool IsOpened(void) { return m_isOpened; }
		
		virtual Texture* GetColorBuffer(void) = 0;
		virtual Texture* GetDepthStencilBuffer(void) = 0;
		///\note Can be used only in render thread.
		virtual void SwapBuffers(bool _vsync = true) = 0;

		SDL_Window* GetHandle(void) { return m_handle; }

	protected:

		Window(SDL_Window* _handle);
		~Window(void);

		SDL_Window* m_handle;
		Vec2ui m_size;
		bool m_isOpened;
	};

	//----------------------------------------------------------------------------//
	// WindowSystem
	//----------------------------------------------------------------------------//

	class ENGINE_API WindowSystem : public Object, public Singleton<WindowSystem>
	{
	public:
		OBJECT(WindowSystem);

		static WindowSystemPtr Create(void);

		WindowPtr CreateWindow(void* _externalWindow = nullptr);
		Window* GetWindow(uint _id);

		void PollEvents(void);

	protected:

		WindowSystem(void);
		~WindowSystem(void);
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
