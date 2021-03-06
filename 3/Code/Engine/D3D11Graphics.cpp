#include "Graphics.hpp"

#pragma warning(disable : 4005)
#include <d3d11.h>

namespace ge
{
	//----------------------------------------------------------------------------//
	// Buffer::Impl
	//----------------------------------------------------------------------------//

	struct Buffer::Impl
	{
		CriticalSection mutex;
		ID3D11Buffer* buffer;
	};

	//----------------------------------------------------------------------------//
	// RenderCommandList::Impl
	//----------------------------------------------------------------------------//

	struct RenderCommandList::Impl
	{

	};

	//----------------------------------------------------------------------------//
	// RenderSystem::Impl
	//----------------------------------------------------------------------------//

	struct RenderSystem::Impl
	{

	};

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	RenderSystem::RenderSystem(void)
	{

	}
	//----------------------------------------------------------------------------//
	RenderSystem::~RenderSystem(void)
	{

	}
	//----------------------------------------------------------------------------//
	bool RenderSystem::Init(void)
	{
		return 0;
	}
	//----------------------------------------------------------------------------//
	RenderCommandList* RenderSystem::CreateCommandList(void)
	{
		return nullptr;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

