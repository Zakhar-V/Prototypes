#include "Graphics.hpp"
#include "Window.hpp"

extern "C"
{
	__declspec(dllexport) unsigned int NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Engine
{
	//----------------------------------------------------------------------------//
	// RenderSystem::StartupParams
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void RenderSystem::StartupParams::Serialize(Config& _cfg, bool _loading)
	{
		if (_loading)
		{
			Config* _val = _cfg.Search("r_RenderSystem");
			if (_val)
			{
				String _renderer = _val->AsString();
				if (_renderer.Equals("GL", true))
					renderer = RT_GL;
				else
					renderer = RT_Unknown;
			}

			_val = _cfg.Search("r_ShaderModel");
			if (_val)
			{
				String _sm = _val->AsString();
				if (_sm.Equals("SM4", true))
					shaderModel = SM4;
				else if (_sm.Equals("SM5", true))
					shaderModel = SM5;
				else
					shaderModel = SM_Unknown;
			}

			_val = _cfg.Search("r_Profile");
			if (_val)
			{
				String _profile = _val->AsString();
				if (_profile.Equals("Compatible", true))
					profile = RCP_Compatible;
				else if (_profile.Equals("Forward", true))
					profile = RCP_Forward;
				else
					profile = RCP_Core;
			}

			_val = _cfg.Search("r_DebugContext");
			if (_val)
				debugContext = _val->AsBool();
		}
		else
		{
			Config& _renderer = _cfg("r_RenderSystem");
			switch (renderer)
			{
			case RT_GL:
				_renderer = "GL";
				break;
			default:
				_renderer = "default";
				break;
			}

			Config& _sm = _cfg("r_ShaderModel");
			switch (shaderModel)
			{
			case SM4:
				_sm = "SM4";
				break;
			case SM5:
				_sm = "SM5";
				break;
			default:
				_sm = "default";
				break;
			}

			Config& _profile = _cfg("r_Profile");
			switch (profile)
			{
			case RCP_Forward:
				_profile = "Forward";
				break;
			case RCP_Compatible:
				_profile = "Compatible";
				break;
			default:
				_profile = "Core";
				break;
			}

			_cfg("r_DebugContext") = debugContext;
		}
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::StartupParams::SetDefaults(void)
	{
		renderer = RT_Unknown;
		shaderModel = SM_Unknown;
		profile = RCP_Core;
		debugContext = false;
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::StartupParams::Validate(void)
	{
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	int RenderSystem::s_moduleRefCount = 0;
	RenderSystem::StartupParams RenderSystem::s_startupParams;

	//----------------------------------------------------------------------------//
	RenderSystem::StartupParams& RenderSystem::GetCurrentStartupParams(void)
	{
		return s_startupParams;
	}
	//----------------------------------------------------------------------------//
	RenderSystem::RenderSystem(void)
	{
		ASSERT(Thread::IsMain());
	}
	//----------------------------------------------------------------------------//
	RenderSystem::~RenderSystem(void)
	{
		ASSERT(Thread::IsMain());

		//TODO: unqueue resources

		if (gWindow)
		{
			LOG_MSG(LL_Event, "Destroy Window");
			gWindow->SetVisible(false);
			delete gWindow;
		}

		if (gRenderDevice)
		{
			LOG_MSG(LL_Event, "Destroy GLRenderDevice");
			delete gRenderDevice;
		}
	}
	//----------------------------------------------------------------------------//
	bool RenderSystem::_Init(void)
	{
		s_startupParams.Validate();

		// select renderer here
		switch (s_startupParams.renderer)
		{
		case RT_Unknown:
			LOG_MSG(LL_Info, "Unknown type of renderer: Use OpenGL by default");
			LOG_MSG(LL_Event, "Create GLRenderDevice");
			new GLRenderDevice;
			break;

		case RT_GL:
			LOG_MSG(LL_Event, "Create GLRenderDevice");
			new GLRenderDevice;
			break;

		default:
			LOG_MSG(LL_Error, "Renderer %d is not supported", s_startupParams.renderer);
			return false;
		}

		if (!gRenderDevice->_Init(s_startupParams.shaderModel, s_startupParams.profile, s_startupParams.debugContext))
		{
			LOG_MSG(LL_Error, "Couldn't initialize RenderDevice");
			delete gRenderDevice;
			return false;
		}

		LOG_MSG(LL_Event, "Create Window");
		new Window(gRenderDevice->_GetSDLWindow());

		//TODO: register resource types 
		// ...

		return true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}