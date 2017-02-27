#pragma once

#include "Object.hpp"
#include "Math.hpp"

struct SDL_Window;

namespace Rx
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	enum ShaderModel : uint
	{
		SM_Unknown = 0,
		SM2 = 2,
		SM3 = 3,
		SM4 = 4,
		SM5 = 5,
	};

	/*
	OpenGL es 2.0 (SM2)
	OpenGL 2.x (2.0 + extensions 3.x) (SM2/SM3)
	OpenGL 3.x (3.0 + extensions 4.x) (SM4/SM5)
	OpenGL 4.5 (4.5, without extensions) (SM5)
	Direct3D9 (SM2/SM3)
	Direct3D11 (SM3/SM4/SM5)

	*/

	enum GraphicsSystemType
	{
		GST_OpenGLES,
		GST_OpenGL,
		GST_DirectX9,
		GST_DirectX11,
	};

	enum GpuFeatures : uint
	{
		GF_ShaderModel,
		GF_NumFeatures,
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


	class GraphicsSystem : public Singleton<GraphicsSystem>, public Object
	{
	public:
		OBJECT("GraphicsSystem", Object);




	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
