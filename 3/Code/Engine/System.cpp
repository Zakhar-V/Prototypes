#include "System.hpp"
#include "Resource.hpp"
#include "Graphics.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// System
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool System::Create(void)
	{
		if (!s_instance)
		{
			LOG_MSG(LL_Event, "Create Engine");
			if (!Thread::IsMain())
			{
				LOG_MSG(LL_Fatal, "Engine should be created in main thread");
				return false;
			}
			new System;
			if (!s_instance->_Init())
			{
				LOG_MSG(LL_Error, "Couldn't initialize Engine");
				delete s_instance;
				return false;
			}
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	void System::Destroy(void)
	{
		if (s_instance)
		{
			ASSERT(Thread::IsMain());
			LOG_MSG(LL_Event, "Destroy Engine");
			delete s_instance;
		}
	}
	//----------------------------------------------------------------------------//
	System::System(void)
	{
	}
	//----------------------------------------------------------------------------//
	System::~System(void)
	{
		// ...

		if (gRenderSystem)
		{
			LOG_MSG(LL_Event, "Destroy RenderSystem");
			delete gRenderSystem;
		}

		if (gResourceCache)
		{
			LOG_MSG(LL_Event, "Destroy ResourceCache");
			delete gResourceCache;
		}
	}
	//----------------------------------------------------------------------------//
	bool System::_Init(void)
	{
		LOG_MSG(LL_Event, "Create ResourceCache");
		new ResourceCache;

		LOG_MSG(LL_Event, "Create RenderSystem");
		new RenderSystem;
		if (!gRenderSystem->_Init())
		{
			LOG_MSG(LL_Fatal, "Couldn't initialize RenderSystem");
			return false;
		}

		// ...

		return true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
