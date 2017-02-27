#include "Log.hpp"
#include "Thread.hpp"
#include "PlatformIncludes.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// LogNode
	//----------------------------------------------------------------------------//

	namespace
	{
		enum : uint
		{
			INFO_LENGTH = 20,
			TAB_SIZE = 4,
		};
		static CriticalSection s_logMutex;
		static THREAD_LOCAL LogNode* s_logNode = nullptr; // top
		static THREAD_LOCAL int s_logDepth = 0;
#ifdef _DEBUG
		static THREAD_LOCAL uint s_logMask = LL_All;
#else
		static THREAD_LOCAL uint s_logMask = LL_All & ~(LL_Debug | LL_RenderDebugOutput);
#endif

		//----------------------------------------------------------------------------//
		uint8 _SetCColors(uint8 _color)
		{
#		ifdef _WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
			{
				uint8 _pc = csbi.wAttributes & 0xff;
				SetConsoleTextAttribute(hStdOut, _color);
				return _pc;
			}
#		else
			NOT_IMPLEMENTED_YET();
#		endif
			return 0;
		}
		//----------------------------------------------------------------------------//
		void _LogWrite(const char* _msg, ...)
		{
			va_list _args;
			va_start(_args, _msg);
			vprintf(_msg, _args);
			va_end(_args);
		}
		//----------------------------------------------------------------------------//
		void _WriteOffset(uint _offset)
		{
			for (uint i = 0; i < _offset; ++i)
				_LogWrite(" ");
		}
		//----------------------------------------------------------------------------//
		void _WriteInfo(void)
		{
			time_t _t; time(&_t);
			struct tm _tm = *localtime(&_t);
			_LogWrite("[%02d:%02d:%02d][%04x]    ", _tm.tm_hour, _tm.tm_min, _tm.tm_sec, Thread::GetCurrentId()); // 20 symbols
		}
		//----------------------------------------------------------------------------//
	}

	//----------------------------------------------------------------------------//
	LogNode::LogNode(bool _writeNow, const char* _func, const char* _title, uint _mask) :
		m_prev(s_logNode),
		m_func(_func),
		m_title(_title ? _title : ""),
		m_mask(_mask & GetMask()),
		m_depth(s_logDepth++),
		m_written(false)
	{
		s_logNode = this;
		if (_writeNow)
			_Write();
	}
	//----------------------------------------------------------------------------//
	LogNode::~LogNode(void)
	{
		if (m_written)
		{
			SCOPE_LOCK(s_logMutex);
			_WriteInfo();
			_WriteOffset(m_depth * TAB_SIZE);
			_LogWrite("}\n");
		}
		s_logNode = m_prev;
		--s_logDepth;
	}
	//----------------------------------------------------------------------------//
	void LogNode::_Write(void)
	{
		if (!m_written)
		{
			SCOPE_LOCK(s_logMutex);

			m_written = true;
			if (m_prev)
				m_prev->_Write();
			_WriteInfo();
			_WriteOffset(m_depth * TAB_SIZE);
			_LogWrite("%s\n", m_title);
			_WriteOffset(INFO_LENGTH + m_depth * TAB_SIZE);
			_LogWrite("{\n");
		}
	}
	//----------------------------------------------------------------------------//
	void LogNode::Message(int _level, const char* _msg, ...)
	{
		SCOPE_LOCK(s_logMutex);

		if ((_level & GetMask()) == 0)
			return;

		if (s_logNode)
			s_logNode->_Write();

		_WriteInfo();
		_WriteOffset(s_logDepth * TAB_SIZE);

		uint8 _cc = 0;
		switch (_level)
		{
		case LL_Info:
			_LogWrite("- ");
			break;

		case LL_Warning:
			_LogWrite("[WARNING] ");
			_cc = _SetCColors(0x0e);
			break;

		case LL_Error:
			_LogWrite("[ERROR] ");
			_cc = _SetCColors(0x0c);
			break;

		case LL_Fatal:
			_LogWrite("[FATAL] ");
			_cc = _SetCColors(0x0c);
			break;

		case LL_Debug:
			_LogWrite("[DEBUG] ");
			_cc = _SetCColors(0x08);
			break;

		case LL_RenderDebugOutput:
			_LogWrite("[DRIVER] ");
			_cc = _SetCColors(0x08);
			break;
		}

		va_list _args;
		va_start(_args, _msg);
		String _text = String::FormatV(_msg, _args);
		va_end(_args);
		_LogWrite("%s\n", _text.c_str());

		if (_cc)
			_SetCColors(_cc);
	}
	//----------------------------------------------------------------------------//
	LogNode* LogNode::GetTop(void)
	{
		return s_logNode;
	}
	//----------------------------------------------------------------------------//
	void LogNode::SetMask(uint _mask)
	{
		if (s_logNode)
			s_logNode->m_mask = _mask & (s_logNode->m_prev ? s_logNode->m_prev->m_mask : s_logMask);
		else
			s_logMask = _mask;
	}
	//----------------------------------------------------------------------------//
	uint LogNode::GetMask(void)
	{
		return s_logNode ? s_logNode->m_mask : s_logMask;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}