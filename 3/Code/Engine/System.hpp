#pragma once

#include "Base.hpp"

namespace Engine
{

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gSystem	Engine::System::Get()

	//----------------------------------------------------------------------------//
	// Engine
	//----------------------------------------------------------------------------//

	class System final : public Singleton<System>
	{
	public:

		static bool Create(void);
		static void Destroy(void);

	protected:

		System(void);
		~System(void);
		bool _Init(void);
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}