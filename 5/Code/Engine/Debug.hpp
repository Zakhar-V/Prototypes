#pragma once

#include "Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gLogSystem Engine::LogSystem::Get()

	//----------------------------------------------------------------------------//
	// LogMessage
	//----------------------------------------------------------------------------//

	struct LogMessage
	{
		int level;
		uint threadId;
		time_t time;
		String func;
		String file;
		int line;
		String msg;
	};

	//----------------------------------------------------------------------------//
	// LogSystem
	//----------------------------------------------------------------------------//

	class LogSystem final : public NonCopyable
	{
	public:
		static LogSystem* Get(void) { return &s_instance; }

		void SetWriteInfo(bool _enabled = true) { m_writeInfo = _enabled; }
		void Message(int _level, const char* _func, const char* _file, int _line, const char* _msg);

	private:
		LogSystem(void);

		bool m_writeInfo;
		static LogSystem s_instance;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
