#include "Render.hpp"
#include "Log.hpp"
#include "SDL.h"

namespace ge
{
	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	RenderSystem::RenderSystem(void) :
		m_type(RST_Null),
		m_shaderModel(SM_Null),
		m_maxRenderTargets(0)
	{

	}
	//----------------------------------------------------------------------------//
	RenderSystem::~RenderSystem(void)
	{

	}
	//----------------------------------------------------------------------------//

	RenderSystemPtr _CreateGLRenderSystem(ShaderModel _shaderModel, uint _flags); // in GLRenderSystem.cpp
	RenderSystemPtr _CreateGLESRenderSystem(ShaderModel _shaderModel, uint _flags); // in GLESRenderSystem.cpp
	RenderSystemPtr _CreateD3D11RenderSystem(ShaderModel _shaderModel, uint _flags); // in D3D11RenderSystem.cpp

	RenderSystemPtr RenderSystem::Create(RenderSystemType _type, ShaderModel _shaderModel, uint _flags)
	{
		LOG_NODE("RenderSystem::Create");

		if (!Thread::IsMain())
		{
			LOG_ERROR("RenderSystem can be created only in main thread");
			return nullptr;
		}

		if (SDL_Init(SDL_INIT_VIDEO))
		{
			LOG_ERROR("Couldn't initialize SDL2 video : %s", SDL_GetError());
			return nullptr;
		}

		RenderSystemPtr _rs;
		int _types[3] = { RST_GL, RST_D3D11, RST_GLES };
		for (int i = 0, _test = _type > 0 ? _type - 1 : _type; i < 3; ++i, _test = (_test + 1) % 3)
		{
			switch(_types[_test])
			{
			case RST_GL:
			{
#ifdef USE_GL
				_rs = _CreateGLRenderSystem(_shaderModel, _flags);
#else
				LOG_ERROR("OpenGL render system is not available");
#endif
			} break;				

			case RST_GLES:
			{
#ifdef USE_GLES
				//_rs = _CreateGLESRenderSystem(_shaderModel, _flags);
#else
				LOG_ERROR("OpenGLES render system is not available");
#endif
			} break;
				
			case RST_D3D11:
			{
#ifdef USE_D3D11
				//_rs = _CreateD3D11RenderSystem(_shaderModel, _flags);
#else
				LOG_ERROR("Direct3D11 render system is not available");
#endif
			} break;
			}

			if ((!_rs && (_flags & RSF_AnyType) == 0) || _rs)
				break;
		}

		if (!_rs)
			LOG_FATAL("No available render system");

		return _rs;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
