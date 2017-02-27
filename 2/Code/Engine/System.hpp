#pragma once

#include "Core.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// System
	//----------------------------------------------------------------------------//

#define gSystem Engine::System::Get()

	class System : public Singleton<System>
	{
	public:

		static void CreateEngine(void);
		static void DestroyEngine(void);

		void BeginFrame(void);
		void EndFrame(void);
		float FrameTime(void) { return m_frameTime; }

	protected:

		System(void);
		~System(void);
		bool _Init(void);

		double m_startTime = 0;
		float m_frameTime = 0; // in microseconds
		double m_appTime = 0; // in microseconds
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
