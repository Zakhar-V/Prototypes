#include "Logger.hpp"
#include "Thread.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Engine
{
	TODO_EX("Logger", "Добавить буферизацию сообщений");
	TODO_EX("Logger", "Перенести функции работы с консолью в Console.hpp (или Platform.hpp)");

	//----------------------------------------------------------------------------//
	// Logger utils
	//----------------------------------------------------------------------------//

	namespace
	{
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

		CriticalSection s_logMutex;
	}

	//----------------------------------------------------------------------------//
	// defined in Base.hpp
	void LogMsg(int _level, const char* _func, const char* _file, int _line, const char* _msg)
	{
		Logger::Get()->Message(_level, _func, _file, _line, _msg);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Logger
	//----------------------------------------------------------------------------//

	Logger Logger::s_instance;

	//----------------------------------------------------------------------------//
	Logger::Logger(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Logger::Message(int _level, const char* _func, const char* _file, int _line, const char* _msg)
	{
		LogMessage _lm;
		time(&_lm.time);
		_lm.threadId = Thread::GetCurrentId();
		_lm.level = _level;
		_lm.func = _func;
		_lm.file = _file;
		_lm.line = _line;
		_lm.msg = _msg;

		// ...
		//TODO: do store message
		// ...

		//[time] thread> file(line): func:
		//    level: msg

		String _str, _info = String::Format("%d> ", _lm.threadId);

		//if (_lm.level != LL_Info && _lm.level != LL_Event)
		{
			if (_lm.file.NonEmpty())
			{
				_info += _lm.file;
				_info += String::Format("(%d): ", _lm.line);
			}
			if (_lm.func.NonEmpty())
			{
				_info += _lm.func;
				_info += ": ";
			}

			String _threadName = Thread::GetName(_lm.threadId);
			if (_threadName != "Unnamed")
			{
				_info += "(";
				_info += _threadName;
				if (!String::Find(_threadName, "thread", true))
					_info += " thread";
				_info += "): ";
			}
		}

		_info += "\n\t";

		switch (_level)
		{
		case LL_Assert:
			_str += "Assertion failed: ";
			break;
		case LL_Fatal:
			_str += "Fatal error: ";
			break;
		case LL_Error:
			_str += "Error: ";
			break;
		case LL_Warning:
			_str += "Warning: ";
			break;
		case LL_Event:
			_str += "Event: ";
			break;
		case LL_Info:
			_str += "Info: ";
			break;
		case LL_Debug:
			_str += "Debug: ";
			break;
		}

		_str += _msg;
		_str += "\n";

		// print message
		{
			struct tm _tm = *localtime(&_lm.time);
			uint8 _cc = 0;
			SCOPE_LOCK(s_logMutex);

			switch (_level)
			{
			case LL_Error:
			case LL_Fatal:
			case LL_Assert:
				_cc = _SetCColors(0x0c);
				break;

			case LL_Warning:
				_cc = _SetCColors(0x0e);
				break;

			case LL_Debug:
				_cc = _SetCColors(0x08);
				break;
			}

			if(m_writeInfo)
				printf("[%02d:%02d:%02d] %s%s", _tm.tm_hour, _tm.tm_min, _tm.tm_sec, *_info, *_str);
			else
				printf("%s", *_str);

			if (_cc)
				_SetCColors(_cc);
		}

#	ifdef _WIN32
		if (IsDebuggerPresent())
		{
			OutputDebugStringA(_info + _str);
			if (_lm.level < LL_Warning)
				DebugBreak();
		}
		else if (_lm.level == LL_Fatal || _lm.level == LL_Assert)
		{
			_str = _msg;
			_str += "\n";

			if (_lm.file.NonEmpty())
			{
				_str += "File: " + _lm.file;
				_str += String::Format("\nLine: %d\n", _lm.line);
			}

			if (_lm.func.NonEmpty())
				_str += "Function: " + _lm.func + "\n";

			_str += String::Format("Thread: %d (%s)\n", _lm.threadId, *Thread::GetName(_lm.threadId));
			_str += "\n See log for more details.";

			MessageBoxA(0, _str, _lm.level == LL_Fatal ? "Fatal error" : "Assertion failed", MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST | MB_DEFAULT_DESKTOP_ONLY);
		}
#	endif

		if (_lm.level == LL_Assert)
		{
			exit(-1);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
