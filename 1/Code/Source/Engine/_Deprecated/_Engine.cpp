#include "Internal.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Engine
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool Engine::Init(void)
	{
		if (!s_instance)
		{
			new Engine();
			if (!s_instance->_Init())
			{
				LOG_ERROR("Couldn't initalize Engine");
				delete s_instance;
				return false;
			}
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	void Engine::Destroy(void)
	{
		if (s_instance) delete s_instance;
	}
	//----------------------------------------------------------------------------//
	Engine::Engine(void)
	{
		s_instance = this;
		new Device;
		new RenderSystem;
	}
	//----------------------------------------------------------------------------//
	Engine::~Engine(void)
	{
		LOG_NODE("Destroying Engine");

		GRenderSystem->_Destroy();
		GDevice->_Destroy();

		delete GRenderSystem;
		delete GDevice;

		s_instance = 0;
	}
	//----------------------------------------------------------------------------//
	bool Engine::_Init(void)
	{
		LOG_NODE("Initializing Engine");

		if (!GDevice->_Init())
		{
			LOG_ERROR("Couldn't initialize Device");
			return false;
		}

		if (!GRenderSystem->_Init())
		{
			LOG_ERROR("Couldn't initialize RenderSystem");
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
