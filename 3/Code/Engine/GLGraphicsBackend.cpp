#include "GLGraphicsBackend.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// gDEBugger
	//----------------------------------------------------------------------------//

	GL_FUNCINIT(glStringMarkerGREMEDY);
	GL_FUNCINIT(glFrameTerminatorGREMEDY);

	//----------------------------------------------------------------------------//
	bool LoadGremedyExtensions(void)
	{
		uint _numFails = 0;
		GL_FUNCLOAD(glStringMarkerGREMEDY);
		GL_FUNCLOAD(glFrameTerminatorGREMEDY);
		return _numFails == 0;
	}
	//----------------------------------------------------------------------------//
	void GLDebugString(const char* _str)
	{
		if (_str && glStringMarkerGREMEDY)
			glStringMarkerGREMEDY((GLsizei)strlen(_str), _str);
	}
	//----------------------------------------------------------------------------//
	void GLDebugBreak(void)
	{
		if (glFrameTerminatorGREMEDY)
			glFrameTerminatorGREMEDY();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GL_NV_command_list
	//----------------------------------------------------------------------------//

	GL_FUNCINIT(glCreateStatesNV);
	GL_FUNCINIT(glDeleteStatesNV);
	GL_FUNCINIT(glIsStateNV);
	GL_FUNCINIT(glStateCaptureNV);
	GL_FUNCINIT(glGetCommandHeaderNV);
	GL_FUNCINIT(glGetStageIndexNV);
	GL_FUNCINIT(glDrawCommandsNV);
	GL_FUNCINIT(glDrawCommandsAddressNV);
	GL_FUNCINIT(glDrawCommandsStatesNV);
	GL_FUNCINIT(glDrawCommandsStatesAddressNV);
	GL_FUNCINIT(glglCreateCommandListsNV);
	GL_FUNCINIT(glDeleteCommandListsNV);
	GL_FUNCINIT(glIsCommandListNV);
	GL_FUNCINIT(glListDrawCommandsStatesClientNV);
	GL_FUNCINIT(glCommandListSegmentsNV);
	GL_FUNCINIT(glCompileCommandListNV);
	GL_FUNCINIT(glCallCommandListNV);

	//----------------------------------------------------------------------------//
	bool LoadNVCommandListExtension(void)
	{
		uint _numFails = 0;
		GL_FUNCLOAD(glCreateStatesNV);
		GL_FUNCLOAD(glDeleteStatesNV);
		GL_FUNCLOAD(glIsStateNV);
		GL_FUNCLOAD(glStateCaptureNV);
		GL_FUNCLOAD(glGetCommandHeaderNV);
		GL_FUNCLOAD(glGetStageIndexNV);
		GL_FUNCLOAD(glDrawCommandsNV);
		GL_FUNCLOAD(glDrawCommandsAddressNV);
		GL_FUNCLOAD(glDrawCommandsStatesNV);
		GL_FUNCLOAD(glDrawCommandsStatesAddressNV);
		GL_FUNCLOAD(glglCreateCommandListsNV);
		GL_FUNCLOAD(glDeleteCommandListsNV);
		GL_FUNCLOAD(glIsCommandListNV);
		GL_FUNCLOAD(glListDrawCommandsStatesClientNV);
		GL_FUNCLOAD(glCommandListSegmentsNV);
		GL_FUNCLOAD(glCompileCommandListNV);
		GL_FUNCLOAD(glCallCommandListNV);
		return _numFails == 0;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLCommandQueue
	//----------------------------------------------------------------------------//

	GLCommandQueue GLCommandQueue::RQ;
	GLCommandQueue GLCommandQueue::DQ;

	Condition GLCommandQueue::s_onPush;

	//----------------------------------------------------------------------------//
	GLCommandQueue::GLCommandQueue(void)
	{
		m_end = m_data + SIZE;
		m_readPos = m_end;
		m_writePos = m_data;

		in = 0;
		out = 0;
	}
	//----------------------------------------------------------------------------//
	GLCommandQueue::~GLCommandQueue(void)
	{
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::PushRaw(const void* _data, size_t _size)
	{
		ASSERT(_size <= SIZE);

		if (_size > 0)
		{
			ASSERT(_data != nullptr);
			m_mutex.Lock();
			_Wait(_size + sizeof(void*) * 2);
			_Write(_data, _size);
			m_mutex.Unlock();
			s_onPush.Signal();
		}
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::Push(GLCommandFunc _func, const void* _data, uint _size)
	{
		ASSERT(_func != nullptr);
		ASSERT(_size == 0 || _data != nullptr);
		ASSERT(_size <= SIZE);

		++in;

		Command _cmd;
		_cmd.func = _func;
		_cmd.size = _size;

		m_mutex.Lock();
		_Wait(sizeof(Command) + _size + sizeof(void*) * 2);
#if 0
		if (_highPriority)
		{
			if (_size > 0)
				_Write(_data, _size, true);
			_Write(&_cmd, sizeof(Command), true);
		}
		else
#endif
		{
			_Write(&_cmd, sizeof(Command));
			if (_size > 0)
				_Write(_data, _size);
		}
		m_mutex.Unlock();
		s_onPush.Signal();
	}
	//----------------------------------------------------------------------------//
	bool GLCommandQueue::Pop(GLCommandFunc& _func, void* _data, bool _wait)
	{
		ASSERT(_data != nullptr);

		++out;

		m_mutex.Lock();

		if (_wait)
		{
			while (_GetSize() == 0) // wait
			{
				m_mutex.Unlock();
				s_onPush.Wait();
				m_mutex.Lock();
			}
		}
		else if (_GetSize() == 0)
		{
			m_mutex.Unlock();
			return false;
		}

		Command _cmd;
		_Read(&_cmd, sizeof(Command));
		if (_cmd.size > 0)
			_Read(_data, _cmd.size);
		_func = _cmd.func;

		m_mutex.Unlock();
		return true;
	}
	//----------------------------------------------------------------------------//
	uint GLCommandQueue::GetSize(void)
	{
		SCOPE_LOCK(m_mutex);
		return (uint)_GetSize();
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::Wait(uint _timeout)
	{
		s_onPush.Wait(_timeout);
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::_Wait(size_t _size)
	{
		bool _overflow = false;
		while (SIZE - _GetSize() < _size)
		{
			_overflow = true;
			m_mutex.Unlock();
			Thread::Pause(0);
			m_mutex.Lock();
		}

		if (_overflow)
		{
			LOG_MSG(LL_Warning, "Command queue overflow");
		}
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::_Write(const void* _data, size_t _size)
	{
#if 0
		if (_highPriority) // write to position of read
		{
			if (m_writePos >= m_readPos) // |---R===W---|
			{
				size_t _ls = m_readPos - m_data;
				if (_ls >= _size)
				{
					m_readPos -= _size;
					memcpy(m_readPos, _data, _size);
					ASSERT(m_readPos >= m_data);
				}
				else
				{
					size_t _rs = _size - _ls;
					memcpy(m_data, ((uint8*)_data) + _rs, _ls);
					m_readPos = m_end - _rs;
					memcpy(m_readPos, _data, _rs);
					ASSERT(m_readPos >= m_writePos);
				}
			}
			else // |===W---R===|
			{
				m_readPos -= _size;
				memcpy(m_readPos, _data, _size);
				ASSERT(m_readPos >= m_writePos);
			}
		}
		else
#endif
		{
			if (m_writePos >= m_readPos)	// |---R===W---|
			{
				size_t _rs = m_end - m_writePos;
				if (_rs >= _size)
				{
					memcpy(m_writePos, _data, _size);
					m_writePos += _size;
					ASSERT(m_writePos <= m_end);
				}
				else
				{
					size_t _ls = _size - _rs;
					memcpy(m_writePos, _data, _rs);
					memcpy(m_data, ((const uint8*)_data) + _rs, _ls);
					m_writePos = m_data + _ls;
					ASSERT(m_writePos <= m_readPos);
				}
			}
			else // |===W---R===|
			{
				memcpy(m_writePos, _data, _size);
				m_writePos += _size;
				ASSERT(m_writePos <= m_readPos);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLCommandQueue::_Read(void* _data, size_t _size)
	{
		if (m_writePos >= m_readPos)	// |---R===W---|
		{
			memcpy(_data, m_readPos, _size);
			m_readPos += _size;
			ASSERT(m_readPos <= m_writePos);
		}
		else // |===W---R===|
		{
			size_t _rs = m_end - m_readPos;
			if (_rs >= _size)
			{
				memcpy(_data, m_readPos, _size);
				m_readPos += _size;
				ASSERT(m_readPos <= m_end);
			}
			else
			{
				size_t _ls = _size - _rs;
				memcpy(_data, m_readPos, _rs);
				memcpy(((uint8*)_data) + _rs, m_data, _ls);
				m_readPos = m_data + _ls;
				ASSERT(m_readPos <= m_writePos);
			}
		}
	}
	//----------------------------------------------------------------------------//
	size_t GLCommandQueue::_GetSize(void)
	{
		uint8* _r = m_readPos;
		uint8* _w = m_writePos;
		return _w >= _r ? (_w - _r) : (SIZE - (_r - _w));
	}
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	// GLDebugPoint
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLDebugPoint::GLDebugPoint(const char* _func, const char* _file, int _line) :
		prev(GLDebugHelper::s_top),
		func(_func),
		file(_file),
		line(_line)
	{
		if (!IsDriverThread())
		{
			LogMsg(LL_Assert, _func, _file, _line, "This function should be called from driver thread only");
		}

		GLDebugHelper::s_top = this;

		// clear errors
		if (gGLDebugHelper->GetType() == GLDebugHelper::OT_CheckErrors)
		{
			if (prev)
				gGLDebugHelper->LogErrors(prev->func, prev->file, prev->line);
			else
				gGLDebugHelper->LogErrors("External code", 0, 0);
		}
	}
	//----------------------------------------------------------------------------//
	GLDebugPoint::~GLDebugPoint(void)
	{
		if (gGLDebugHelper->GetType() == GLDebugHelper::OT_CheckErrors)
			gGLDebugHelper->LogErrors(func, file, line);

		GLDebugHelper::s_top = prev;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLDebugHelper
	//----------------------------------------------------------------------------//
	
	GLDebugPoint* GLDebugHelper::s_top;
	GLDebugHelper GLDebugHelper::s_instance;

	//----------------------------------------------------------------------------//
	GLDebugHelper::GLDebugHelper(void) :
		m_type(OT_None),
		m_level(DOL_Disabled)
	{
	}
	//----------------------------------------------------------------------------//
	GLDebugHelper::~GLDebugHelper(void)
	{
	}
	//----------------------------------------------------------------------------//
	DebugOutputLevel GLDebugHelper::SetLevel(DebugOutputLevel _level)
	{
		DebugOutputLevel _prev = m_level;

		if (m_level != _level)
		{
			switch (m_type)
			{
			case OT_GL43:
			{
				m_level = _level;
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, m_level >= DOL_Low);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, m_level >= DOL_Normal);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, m_level >= DOL_High);

			} break;

			case OT_ARB:
			{
				m_level = _level;
				glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, nullptr, m_level >= DOL_Low);
				glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM_ARB, 0, nullptr, m_level >= DOL_Normal);
				glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_ARB, 0, nullptr, m_level >= DOL_High);

			} break;

			case OT_AMD:
			{
				m_level = _level;
				glDebugMessageEnableAMD(0, GL_DEBUG_SEVERITY_LOW_AMD, 0, nullptr, m_level >= DOL_Low);
				glDebugMessageEnableAMD(0, GL_DEBUG_SEVERITY_MEDIUM_AMD, 0, nullptr, m_level >= DOL_Normal);
				glDebugMessageEnableAMD(0, GL_DEBUG_SEVERITY_HIGH_AMD, 0, nullptr, m_level >= DOL_High);

			} break;

			case OT_CheckErrors:
			{
				m_level = _level;
			} break;
			}
		}

		return _prev;
	}
	//----------------------------------------------------------------------------//
	void GLDebugHelper::LogErrors(const char* _func, const char* _file, int _line)
	{
		if (m_level >= DOL_High)
		{
			for (uint _err; (_err = glGetError());)
			{
				String _errStr;
				switch (_err)
				{
				case GL_INVALID_ENUM:
					_errStr = "GL_INVALID_ENUM error generated";
					break;
				case GL_INVALID_VALUE:
					_errStr = "GL_INVALID_VALUE error generated";
					break;
				case GL_INVALID_OPERATION:
					_errStr = "GL_INVALID_OPERATION error generated";
					break;
				case 0x0504: // deprecated
					_errStr = "GL_STACK_UNDERFLOW error generated";
					break;
				case 0x0503: // deprecated
					_errStr = "GL_STACK_OVERFLOW error generated";
					break;
				case GL_OUT_OF_MEMORY:
					_errStr = "GL_OUT_OF_MEMORY error generated";
					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					_errStr = "GL_INVALID_FRAMEBUFFER_OPERATION error generated";
					break;
				case 0x0507: // OpenGL 4.5
					_errStr = "GL_CONTEXT_LOST error generated";
					break;
				case 0x8031: // GL_ARB_imaging
					_errStr = "GL_TABLE_TOO_LARGE1 error generated";
					break;

				default:
					_errStr = String::Format("glGetError returned 0x04x", _err);
					break;
				};
				_Message(_func, _file, _line, "OpenGL", "Error", "high", _errStr);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLDebugHelper::Message(const char* _func, const char* _file, int _line, const char* _src, const char* _type, const char* _level, const char* _msg)
	{
		_Message(_func, _file, _line, _src, _type, _level, _msg);
	}
	//----------------------------------------------------------------------------//
	void GLDebugHelper::_Init(bool _debugContext)
	{
		if (!_debugContext)
		{
			m_type = OT_None;
			m_level = DOL_Disabled;
			LOG_MSG(LL_Info, "Debug output is disabled");
		}
		else if (ogl_IsVersionGEQ(4, 3))
		{
			m_type = OT_GL43;
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallbackARB(&_DebugProcARB, nullptr);
			LOG_MSG(LL_Info, "OpenGL 4.3 is used for debug output");
		}
		else if (ogl_ext_ARB_debug_output)
		{
			m_type = OT_ARB;
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallbackARB(&_DebugProcARB, nullptr);
			LOG_MSG(LL_Info, "GL_ARB_debug_output extension is used for debug output");
		}
		else if (ogl_ext_AMD_debug_output)
		{
			m_type = OT_AMD;
			glDebugMessageCallbackAMD(&_DebugProcAMD, nullptr);
			LOG_MSG(LL_Info, "GL_AMD_debug_output extension is used for debug output");
		}
		else
		{
			m_type = OT_CheckErrors;
			LOG_MSG(LL_Info, "glGetError is used for debug output");
		}

		SetLevel(DOL_Normal);
	}
	//----------------------------------------------------------------------------//
	void GLDebugHelper::_Message(const char* _func, const char* _file, int _line, const char* _src, const char* _type, const char* _level, const char* _msg)
	{
		// ...
		LogMsg(LL_Debug, _func, _file, _line, String::Format("%s: %s(%s): %s", _src, _type, _level, _msg));
	}
	//----------------------------------------------------------------------------//
	void __stdcall GLDebugHelper::_DebugProcARB(uint _source, uint _type, uint _id, uint _severity, int _length, const char* _message, const void* _ud)
	{
		const char* _src;
		const char* _typeStr;
		const char* _levelStr;
		DebugOutputLevel _level;
		const char* _func = nullptr;
		const char* _file = nullptr;
		int _line = 0;

		switch (_source)
		{
		case GL_DEBUG_SOURCE_API_ARB:
			_src = "OpenGL";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			_src = "Window system";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			_src = "Shader compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			_src = "Third party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			_src = "Application";
			break;
		case GL_DEBUG_SOURCE_OTHER_ARB:
			_src = "Other";
			break;
		default:
			return;
		}

		switch (_type)
		{
		case GL_DEBUG_TYPE_ERROR_ARB:
			_typeStr = "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			_typeStr = "Deprecated behavior";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			_typeStr = "Undefined behavior";
			break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			_typeStr = "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			_typeStr = "Performance";
			break;
		case GL_DEBUG_TYPE_OTHER_ARB:
			_typeStr = "Message";
			break;
#if 0
		case GL_DEBUG_TYPE_MARKER:
			_dtype = "Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			_dtype = "Push group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			_dtype = "Pop group";
			break;
#endif
		default:
			return;
		}

		switch (_severity)
		{
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			_levelStr = "high";
			_level = DOL_High;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			_levelStr = "medium";
			_level = DOL_Normal;
			break;
		case GL_DEBUG_SEVERITY_LOW_ARB:
			_levelStr = "low";
			_level = DOL_Low;
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			_levelStr = "notification";
			_level = DOL_Low;
			break;
		default:
			return;
		}

		if (s_top)
		{
			_func = s_top->func;
			_file = s_top->file;
			_line = s_top->line;
		}

		if (_level <= s_instance.m_level)
		{
			s_instance._Message(_func, _file, _line, _src, _typeStr, _levelStr, (const char*)_message);
		}
	}
	//----------------------------------------------------------------------------//
	void __stdcall GLDebugHelper::_DebugProcAMD(uint _id, uint _category, uint _severity, int _length, const char* _message, void* _ud)
	{
		const char* _src;
		const char* _type;
		const char* _levelStr;
		DebugOutputLevel _level = DOL_Low;
		const char* _func = nullptr;
		const char* _file = nullptr;
		int _line = 0;

		switch (_category)
		{
		case GL_DEBUG_CATEGORY_API_ERROR_AMD:
			_src = "OpenGL";
			_type = "Error";
			break;
		case GL_DEBUG_CATEGORY_APPLICATION_AMD:
			_src = "Application";
			_type = "Message";
			break;
		case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
			_src = "OpenGL";
			_type = "Deprecated behavior";
			break;
		case GL_DEBUG_CATEGORY_OTHER_AMD:
			_src = "Other";
			_type = "Message";
			break;
		case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
			_src = "OpenGL";
			_type = "Perfomance";
			break;
		case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
			_src = "Shader compiler";
			_type = "Output";
			break;
		case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
			_src = "OpenGL";
			_type = "Undefined behavior";
			break;
		case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
			_src = "Window system";
			_type = "Message";
			break;

		default:
			return;
		}

		switch (_severity)
		{
		case GL_DEBUG_SEVERITY_HIGH_AMD:
			_levelStr = "high";
			_level = DOL_High;
			break;
		case GL_DEBUG_SEVERITY_MEDIUM_AMD:
			_levelStr = "medium";
			_level = DOL_Normal;
			break;
		case GL_DEBUG_SEVERITY_LOW_AMD:
			_levelStr = "low";
			_level = DOL_Low;
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			_levelStr = "notification";
			_level = DOL_Low;
			break;
		default:
			return;
		}

		if (s_top)
		{
			_func = s_top->func;
			_file = s_top->file;
			_line = s_top->line;
		}

		if (_level >= s_instance.m_level)
		{
			s_instance._Message(_func, _file, _line, _src, _type, _levelStr, (const char*)_message);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLGpuInfo
	//----------------------------------------------------------------------------//

	GLGpuInfo GLGpuInfo::s_instance;

	//----------------------------------------------------------------------------//
	uint GLGpuInfo::GetAvailableVideoMemory(void)
	{
		if (m_GL_NVX_gpu_memory_info)
		{
			//#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
			int	_size;
			glGetIntegerv(0x9049, &_size);
			return _size / 1024;
		}
#if _WIN32
		else
		{
			// use WMI or DX
		}
#endif
		return 0;
	}
	//----------------------------------------------------------------------------//
	void GLGpuInfo::_Init(void)
	{
		if (SDL_GL_ExtensionSupported("GL_NVX_gpu_memory_info"))
		{
			//#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
			m_GL_NVX_gpu_memory_info = true;
			glGetIntegerv(0x9048, (int*)&m_totalVRam);
			m_totalVRam /= 1024;
			uint _availableMemory = GetAvailableVideoMemory();
			LOG_MSG(LL_Info, "Video memory: %u mb (used %u mb, available %u mb)", m_totalVRam, (m_totalVRam - _availableMemory), _availableMemory);
		}
		else
		{
			LOG_MSG(LL_Warning, "Unable to get size of video memory");
		}

		//NVIDIA: GL_NVX_gpu_memory_info
		//AMD: GL_ATI_meminfo, WGL_AMD_gpu_association, GLX_AMD_gpu_association
		//Win32: WMI, DirectX

	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Types
	//----------------------------------------------------------------------------//

	const uint GLMappingModes[] =
	{
		GL_MAP_READ_BIT, // MM_Read
		GL_MAP_WRITE_BIT, // MM_Write
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT, // MM_Discard
	};

	const uint GLBufferUsages[] =
	{
		GL_STATIC_DRAW, // HBU_Default
		GL_DYNAMIC_DRAW, // HBU_Dynamic
		GL_DYNAMIC_READ, // HBU_Readback
	};

	//----------------------------------------------------------------------------//
	// GLBufferHandle
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLBufferHandle::~GLBufferHandle(void)
	{
		VERIFY_GL_SCOPE();

		glDeleteBuffers(1, &glhandle);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Destroy(void)
	{
		if (IsDriverThread())
		{
			delete this;
		}
		else
		{
			Cmd_Destroy _cmd(this);
			gGLResourceQueue.Push(&_cmd);
		}
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Create(uint _size, const void* _data)
	{
		VERIFY_GL_SCOPE();

		glGenBuffers(1, &glhandle);
		glBindBuffer(GL_ARRAY_BUFFER, glhandle);
		glBufferData(GL_ARRAY_BUFFER, _size, _data, glusage);
		size = _size;
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Create(GLCommandQueue& _queue, HwBufferUsage _usage, uint _size, const void* _data)
	{
		glusage = GLBufferUsages[_usage];

		Atomic<bool> _done = false;
		Cmd_Create _cmd = { this, _size, _data, &_done };
		_queue.Push(&GLBufferHandle::Create, &_cmd);

		while (!_done)
			Thread::Pause(0);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Create(Cmd_Create* _cmd)
	{
		_cmd->handle->Create(_cmd->size, _cmd->data);
		*_cmd->done = true;
	}
	//----------------------------------------------------------------------------//
	uint8* GLBufferHandle::Map(MappingMode _mode, uint _offset, uint _size)
	{
		VERIFY_GL_SCOPE();

		return (uint8*)glMapNamedBufferRangeEXT(glhandle, _offset, _size, GLMappingModes[_mode]);
	}
	//----------------------------------------------------------------------------//
	uint8* GLBufferHandle::Map(GLCommandQueue& _queue, MappingMode _mode, uint _offset, uint _size)
	{
		Atomic<uint8*> _mp;
		Cmd_Map _cmd = { this, _mode, _offset, _size, &_mp };
		_queue.Push(&GLBufferHandle::Map, &_cmd);

		uint8* _ptr;
		while (!(_ptr = _mp))
			Thread::Pause(0);

		return (_ptr != (uint8*)-1) ? _ptr : nullptr;
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Map(Cmd_Map* _cmd)
	{
		uint8* _ptr = _cmd->handle->Map(_cmd->mode, _cmd->offset, _cmd->size);
		*_cmd->ptr = _ptr ? _ptr : (uint8*)-1;
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Unmap(void)
	{
		VERIFY_GL_SCOPE();

		glUnmapNamedBufferEXT(glhandle);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Unmap(GLCommandQueue& _queue)
	{
		Cmd_Unmap _cmd = { this };
		_queue.Push(&GLBufferHandle::Unmap, &_cmd);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Unmap(Cmd_Unmap* _cmd)
	{
		_cmd->handle->Unmap();
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Copy(GLBufferHandle* _src, uint _srcOffset, uint _dstOffset, uint _size)
	{
		VERIFY_GL_SCOPE();

		glNamedCopyBufferSubDataEXT(_src->glhandle, glhandle, _srcOffset, _dstOffset, _size);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Copy(GLCommandQueue& _queue, GLBufferHandle* _src, uint _srcOffset, uint _dstOffset, uint _size)
	{
		Cmd_Copy _cmd = { this, _src, _dstOffset, _srcOffset, _size };
		_queue.Push(&GLBufferHandle::Copy, &_cmd);
	}
	//----------------------------------------------------------------------------//
	void GLBufferHandle::Copy(Cmd_Copy* _cmd)
	{
		_cmd->dst->Copy(_cmd->src, _cmd->srcOffset, _cmd->dstOffset, _cmd->size);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
