#pragma once

#include "Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gLogger Engine::Logger::Get()

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
	// Logger
	//----------------------------------------------------------------------------//

	class Logger final: public NonCopyable
	{
	public:
		static Logger* Get(void) { return &s_instance; }

		void SetWriteInfo(bool _enabled = true) { m_writeInfo = _enabled; }
		void Message(int _level, const char* _func, const char* _file, int _line, const char* _msg);

	private:
		Logger(void);

		bool m_writeInfo;
		static Logger s_instance;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
