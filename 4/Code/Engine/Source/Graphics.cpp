#include "../Graphics.hpp"
#include <SDL.h>

namespace Rx
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HardwareBuffer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	HardwareBuffer::HardwareBuffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize) :
		m_type(_type),
		m_usage(_usage),
		m_size(_size),
		m_elementSize(_elementSize)
	{
	}
	//----------------------------------------------------------------------------//
	HardwareBuffer::~HardwareBuffer(void)
	{
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// VertexFormat
	//----------------------------------------------------------------------------//
	
	const VertexFormatDesc VertexFormatDesc::Empty;

	//----------------------------------------------------------------------------//
	VertexFormat::VertexFormat(const VertexFormatDesc& _desc) :
		m_desc(_desc)
	{
	}
	//----------------------------------------------------------------------------//
	VertexFormat::~VertexFormat(void)
	{
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HardwareShaderStage
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	HardwareShaderStage::HardwareShaderStage(ShaderStageType _type)	:
		m_type(_type)
	{
	}
	//----------------------------------------------------------------------------//
	HardwareShaderStage::~HardwareShaderStage(void)
	{
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	void _CreateD3D11RenderSystem(void); // in D3D11RenderSystem.cpp

	//----------------------------------------------------------------------------//
	bool RenderSystem::Create(void)
	{
		if (s_instance)
			return true;

		LOG_EVENT("Create RenderSystem");

		_CreateD3D11RenderSystem();
		if (s_instance->_Init())
			return true;

		LOG_ERROR("Couldn't create RenderSystem");

		s_instance->_Destroy();

		return false;
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::Destroy(void)
	{
		if (s_instance)
		{
			LOG_EVENT("Destroy RenderSystem");
			s_instance->_Destroy();
			delete s_instance;
		}
	}
	//----------------------------------------------------------------------------//
	RenderSystem::RenderSystem(void) :
		m_window(nullptr)
	{
	}
	//----------------------------------------------------------------------------//
	RenderSystem::~RenderSystem(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool RenderSystem::_Init(void)
	{
		if (SDL_Init(SDL_INIT_VIDEO))
		{
			LOG_ERROR("Couldn't initialize SDL video : %s", SDL_GetError());
			return false;
		}

		if (!_InitDriver())
		{
			LOG_ERROR("Couldn't initialize driver");
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::_Destroy(void)
	{
		_DestroyDriver();

		if (m_window)
		{
			SDL_DestroyWindow(m_window);
		}
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::BeginFrame(void)
	{
		
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::EndFrame(void)
	{

	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
