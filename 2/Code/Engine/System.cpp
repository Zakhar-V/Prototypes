#include "System.hpp"
#include "GraphicsCore.hpp"
#include "Device.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// System
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void System::CreateEngine(void)
	{
		if (!s_instance)
		{
			LOG_EVENT("Create System");

			new System;
			if (!s_instance->_Init())
			{
				LOG_ERROR("Couldn't initialize System");
				delete s_instance;
				throw std::runtime_error("Couldn't create Engine");
			}
		}
	}
	//----------------------------------------------------------------------------//
	void System::DestroyEngine(void)
	{
		if (s_instance)
		{
			LOG_EVENT("Destroy System");
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
		if (gResourceCache)
			gResourceCache->_Shutdown();

		Device::_Destroy();
		RenderContext::_Destroy();

		if (gResourceCache)
			delete gResourceCache;
	}
	//----------------------------------------------------------------------------//
	bool System::_Init(void)
	{
		new ResourceCache();

		if (!RenderContext::_Create())
			return false;

		if (!Device::_Create())
			return false;

		return true;
	}
	//----------------------------------------------------------------------------//
	void System::BeginFrame(void)
	{
		gDevice->_BeginFrame();
		m_startTime = TimeMs();
		gRenderContext->_BeginFrame();
	}
	//----------------------------------------------------------------------------//
	void System::EndFrame(void)
	{
		gRenderContext->_EndFrame();
		gDevice->_EndFrame(); // ??

		m_frameTime = (float)(TimeMs() - m_startTime);
		//if(m_frameTime > 1) m_frameTime = 0;
		m_appTime += m_frameTime;

		//printf("%f %f\n", m_appTime, m_frameTime);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
