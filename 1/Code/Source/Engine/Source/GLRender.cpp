#include "GLRender.hpp"
#include "Log.hpp"

#ifdef USE_GL

namespace ge
{
#ifdef DEBUG_RC

	//----------------------------------------------------------------------------//
	// gDEBugger
	//----------------------------------------------------------------------------//
	
	typedef void (APIENTRY* PFNGLSTRINGMARKERGREMEDYPROC) (GLsizei len, const GLvoid *string); // GL_GREMEDY_string_marker
	typedef void (APIENTRY* PFNGLFRAMETERMINATORGREMEDYPROC) (void); // GL_GREMEDY_frame_terminator

	void APIENTRY _glStringMarkerGREMEDY(GLsizei len, const GLvoid *string) { }
	void APIENTRY _glFrameTerminatorGREMEDY(void) { }

	PFNGLSTRINGMARKERGREMEDYPROC glStringMarkerGREMEDY = _glStringMarkerGREMEDY;
	PFNGLFRAMETERMINATORGREMEDYPROC glFrameTerminatorGREMEDY = _glFrameTerminatorGREMEDY;

	//----------------------------------------------------------------------------//
	void GLLoadGremedyExtensions(void)
	{
#if defined(_WIN32)
		glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)wglGetProcAddress("glStringMarkerGREMEDY");
		glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)wglGetProcAddress("glFrameTerminatorGREMEDY");
#elif defined (__APPLE__)
		glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)getMacOSXExtensionFunctionAddress("glStringMarkerGREMEDY");
		glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)getMacOSXExtensionFunctionAddress("glFrameTerminatorGREMEDY");
#elif defined(__linux__)
		glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)glXGetProcAddress((const GLubyte*)"glStringMarkerGREMEDY");
		glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)glXGetProcAddress((const GLubyte*)"glFrameTerminatorGREMEDY");
#endif

		if (glStringMarkerGREMEDY || glFrameTerminatorGREMEDY)
			LOG_INFO("gDEBugger is presented");

		if (!glStringMarkerGREMEDY)
			glStringMarkerGREMEDY = _glStringMarkerGREMEDY;

		if (!glFrameTerminatorGREMEDY)
			glFrameTerminatorGREMEDY = _glFrameTerminatorGREMEDY;
	}
	//----------------------------------------------------------------------------//
	void GLDebugPoint(const char* _title)
	{
		if (!_title)
			_title = "debug point";
		glStringMarkerGREMEDY(strlen(_title), _title);
		glFrameTerminatorGREMEDY();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GL debug output
	//----------------------------------------------------------------------------//

	bool GLDebugOutputEnabled = false; // ?

	namespace
	{
		THREAD_LOCAL bool tls_GLDebugOutputEnabled = true; // ?
	}

	//----------------------------------------------------------------------------//
	void APIENTRY GLDebugOutputARB(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const GLchar* _message, const GLvoid* _ud)
	{
		bool _enabled = (LogNode::GetMask() & LL_RenderDebugOutput) != 0;
		LogNode* _top = LogNode::GetTop();
		const char* _func = __FUNCTION__;
		if (_top)
			_func = _top->GetFunc();

		if (_enabled && tls_GLDebugOutputEnabled)
		{
			const char* _dsource;
			const char* _dtype;
			const char* _dseverity;
			int _priority = 0;

			switch (_source)
			{
			case GL_DEBUG_SOURCE_API_ARB:
				_dsource = "OpenGL";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
				_dsource = "Windows";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
				_dsource = "Shader Compiler";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
				_dsource = "Third Party";
				break;
			case GL_DEBUG_SOURCE_APPLICATION_ARB:
				_dsource = "Application";
				break;
			case GL_DEBUG_SOURCE_OTHER_ARB:
				_dsource = "Other";
				break;
			default:
				_dsource = "Unknown";
				break;
			}

			switch (_type)
			{
			case GL_DEBUG_TYPE_ERROR_ARB:
				_dtype = "Error";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
				_dtype = "Deprecated behavior";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
				_dtype = "Undefined behavior";
				break;
			case GL_DEBUG_TYPE_PORTABILITY_ARB:
				_dtype = "Portability";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE_ARB:
				_dtype = "Performance";
				break;
			case GL_DEBUG_TYPE_OTHER_ARB:
				_dtype = "Message";
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
				_dtype = "unknown";
				break;
			}

			switch (_severity)
			{
			case GL_DEBUG_SEVERITY_HIGH_ARB:
				_dseverity = "high";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM_ARB:
				_dseverity = "medium";
				break;
			case GL_DEBUG_SEVERITY_LOW_ARB:
				_dseverity = "low";
				return;
				break;

#if 1 // ignore notifications
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				_dseverity = "notification";
				return;
#endif
			default:
				_dseverity = "?";
				break;
			}

			LogNode::Message(LL_RenderDebugOutput, "%s: %s: %s(%s) %d : %s", _func, _dsource, _dtype, _dseverity, _id, _message);
		}
	}
	//----------------------------------------------------------------------------//
	void GLEnableDebugOutput(bool _enabled = true)
	{
		if (_enabled)
		{
			if (ogl_ext_ARB_debug_output)
			{
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
				glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
				glDebugMessageCallbackARB(&GLDebugOutputARB, 0);
				GLDebugOutputEnabled = true;
				LOG_INFO("Debug-output is enabled");
			}
			else if (ogl_IsVersionGEQ(4, 3))
			{
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
				glDebugMessageCallback(&GLDebugOutputARB, 0);
				GLDebugOutputEnabled = true;
				LOG_INFO("Debug-output is enabled");
			}
			else
			{
				GLDebugOutputEnabled = false;
				LOG_WARNING("Debug-output extension is not found");
			}
		}
		else if (ogl_ext_ARB_debug_output || ogl_IsVersionGEQ(4, 3))
		{
			glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			GLDebugOutputEnabled = false;
			LOG_INFO("Debug-output is disabled");
		}
	}
	//----------------------------------------------------------------------------//
	void GLDebugMsg(int _type, int _prio, const char* _msg, ...)
	{
		if (GLDebugOutputEnabled)
		{
			va_list _args;
			va_start(_args, _msg);
			String _str = String::FormatV(_msg, _args);
			va_end(_args);

			if (ogl_ext_ARB_debug_output)
			{
				glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, _type, 0, _prio, _str.Length(), _str);
			}
			else if (ogl_IsVersionGEQ(4, 3))
			{
				glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION_ARB, _type, 0, _prio, _str.Length(), _str);
			}
			else
			{
				GLDebugOutputARB(GL_DEBUG_SOURCE_APPLICATION_ARB, _type, 0, _prio, _str.Length(), _str, nullptr);
			}
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Debug
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void GLInitDebug(bool _enableDebugOutput)
	{
		GLEnableDebugOutput(_enableDebugOutput);
		GLLoadGremedyExtensions();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#endif

	//----------------------------------------------------------------------------//
	// Common
	//----------------------------------------------------------------------------//

	uint GLAccessMode(uint _mode)
	{
		if ((_mode & AM_ReadWrite) == AM_ReadWrite)
			return GL_READ_WRITE;
		else if (_mode & AM_Read)
			return GL_READ_ONLY;
		else if (_mode & AM_Write)
			return GL_READ_WRITE;

		return GL_NONE;
	}

	struct GLPrimitiveInfo
	{
		const char* name;
		uint type;
		uint minVersion;
		uint vertices;
		uint step;
		uint primitivesAdd;
		bool isPatch;
		bool isStrip;
		bool isAdjacency;
		bool usePritiveRestart;
	};

	GLPrimitiveInfo GLPrimitiveTypes[] =
	{
		{ "Points", GL_POINTS, 21, 1, 1, 0, false, false, false, false }, // PT_Points
		{ "Lines", GL_LINES, 21, 2, 2, 0, false, false, false, false }, // PT_Lines
		{ "LineStrip", GL_LINE_STRIP, 21, 2, 1, 0, false, true, false, false }, // PT_LineStrip
		{ "LineStrips", GL_LINE_STRIP, 33, 2, 1, 0, false, true, false, true }, // PT_LineStrips
		{ "LineLoop", GL_LINE_LOOP, 21, 2, 2, 1, false, false, false, false }, // PT_LineLoop
		{ "LinesAdjacency", GL_LINES_ADJACENCY, 33, 4, 4, 0, false, false, true, false }, // PT_LinesAdjacency
		{ "LineStripAdjacency", GL_LINE_STRIP_ADJACENCY, 33, 4, 1, 0, false, true, true, false }, // PT_LineStripAdjacency
		{ "LineStripsAdjacency", GL_LINE_STRIP_ADJACENCY, 33, 4, 1, 0, false, true, true, true }, // PT_LineStripsAdjacency
		{ "LinePatches", GL_PATCHES, 43, 2, 2, 0, true, false, false, false }, // PT_LinePatches
		{ "Triangles", GL_TRIANGLES, 21, 3, 3, 0, false, false, false, false }, // PT_Triangles
		{ "TriangleStrip", GL_TRIANGLE_STRIP, 21, 3, 1, 0, false, true, false, false }, // PT_TriangleStrip
		{ "TriangleStrips", GL_TRIANGLE_STRIP, 33, 3, 1, 0, false, true, false, true }, // PT_TriangleStrips
		{ "TrianglesAdjacency", GL_TRIANGLES_ADJACENCY, 33, 6, 6, 0, false, false, true, false }, // PT_TrianglesAdjacency
		{ "TriangleStripAdjacency", GL_TRIANGLE_STRIP_ADJACENCY, 33, 6, 2, 0, false, true, true, false }, // PT_TriangleStripAdjacency
		{ "TriangleStripsAdjacency", GL_TRIANGLE_STRIP_ADJACENCY, 33, 6, 2, 0, false, true, true, true }, // PT_TriangleStripsAdjacency
		{ "TrianglePatches", GL_PATCHES, 43, 3, 3, 0, true, false, false, false }, // PT_TrianglePatches
		{ "QuadPatches", GL_PATCHES, 43, 4, 4, 0, true, false, false, false }, // PT_QuadPatches
		{ "Patches5", GL_PATCHES, 43, 5, 5, 0, true, false, false, false }, // PT_Patches5
		{ "Patches6", GL_PATCHES, 43, 6, 6, 0, true, false, false, false }, // PT_Patches6
		{ "Patches7", GL_PATCHES, 43, 7, 7, 0, true, false, false, false }, // PT_Patches7
		{ "Patches8", GL_PATCHES, 43, 8, 8, 0, true, false, false, false }, // PT_Patches8
	};

	uint NumPrimitives(PrimitiveType _type, uint _elements)
	{
		const GLPrimitiveInfo& _info = GLPrimitiveTypes[_type];
		if (_elements > _info.vertices)
			return (_elements - (_info.vertices - _info.step)) / _info.step + _info.primitivesAdd;
		return 0;
	}

	//----------------------------------------------------------------------------//
	// GLBuffer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	uint GLBufferUsage(HardwareBufferUsage _usage, AccessMode _cpuAccess)
	{
		switch (_usage)
		{
		case HBU_Default:
			return GL_STATIC_DRAW;

		case HBU_Dynamic:
			return GL_DYNAMIC_DRAW;

		case HBU_Staging:
			return (_cpuAccess & AM_Read) ? GL_DYNAMIC_READ : GL_DYNAMIC_COPY; // ???
		}
		ASSERT(false, "Unknown enum");
		return 0;
	}
	//----------------------------------------------------------------------------//
	uint GLBufferStorageFlags(HardwareBufferUsage _usage, AccessMode _cpuAccess)
	{
		uint _flags = 0;
		if (_cpuAccess & AM_Read)
			_flags |= GL_MAP_READ_BIT;
		if (_cpuAccess & AM_Write)
			_flags |= GL_MAP_WRITE_BIT;

		switch (_usage)
		{
		case HBU_Default:
		case HBU_Dynamic:
			return _flags | GL_DYNAMIC_STORAGE_BIT;

		case HBU_Staging:
			return _flags | GL_CLIENT_STORAGE_BIT;
		}
		ASSERT(false, "Unknown enum");
		return 0;
	}
	//----------------------------------------------------------------------------//
	uint GLMappingMode(MappingMode _mode)
	{
		switch (_mode)
		{
		case MM_None:
			return 0;
		case MM_ReadOnly:
			return GL_MAP_READ_BIT;
		case MM_ReadWrite:
			return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
		case MM_WriteDiscard:
			return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
		case MM_WriteNoOverwrite:
			return GL_MAP_WRITE_BIT;
		}
		ASSERT(false, "Unknown enum");
		return 0;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLBuffer::GLBuffer(HardwareBufferUsage _usage, AccessMode _cpuAccess, uint _size, const void* _data) :
		HardwareBuffer(_usage, _cpuAccess, _size),
		m_mappedData(nullptr),
		m_isMapped(false),
		m_handle(0)
	{
		R_DEBUG_NODE();

		if (GLVersion >= 45)
		{
			glCreateBuffers(1, &m_handle);
			glNamedBufferStorage(m_handle, _size ? _size : 4, _data, GLBufferStorageFlags(m_usage, m_cpuAccess));
			glGetNamedBufferParameteriv(m_handle, GL_BUFFER_SIZE, (int*)&m_size);
			if (m_size != _size && _size > 0)
			{
				LOG_ERROR("Couldn't allocate hardware buffer %d: No enough of memory for %u bytes", m_handle, _size);
			}
		}
		else 
		{
			if (GLVersion > 21 || m_usage != HBU_Staging)
			{
				glGenBuffers(1, &m_handle);
				glBindBuffer(GL_ARRAY_BUFFER, m_handle);
				GLState->currentBuffer = this;
				glBufferData(GL_ARRAY_BUFFER, _size ? _size : 4, _data, GLBufferUsage(m_usage, m_cpuAccess));
				glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, (int*)&m_size);
				if (m_size != _size && _size > 0)
				{
					LOG_ERROR("Couldn't allocate hardware buffer %d: No enough of memory for %u bytes", m_handle, _size);
				}
			}

			if (GLVersion == 21)
			{
				m_mappedData = new MappedData21;
				if (m_usage == HBU_Dynamic || m_usage == HBU_Staging)
				{
					m_mappedData->storage = new(std::nothrow) uint8[_size ? _size : 4];
					if (!m_mappedData->storage)
					{
						LOG_ERROR("Couldn't allocate staging buffer %d: No enough of memory for %u bytes", m_handle, _size);
						m_size = 0;
						m_mappedData->storage = new(std::nothrow) uint8[4];
					}
					
					if (_data)
						memcpy(m_mappedData->storage, _data, m_size);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	GLBuffer::~GLBuffer(void)
	{
		R_DEBUG_NODE();

		if (GLState->currentBuffer == this)
			GLState->currentBuffer = nullptr;

		if(m_handle)
			glDeleteBuffers(1, &m_handle);

		if (m_mappedData)
		{
			if (m_mappedData->tempBuffer)
				delete[] m_mappedData->tempBuffer;
			if(m_mappedData->storage)
				delete[] m_mappedData->storage;
			delete m_mappedData;
		}
	}
	//----------------------------------------------------------------------------//
	uint8* GLBuffer::Map(MappingMode _mode)
	{
		return GLBuffer::Map(_mode, 0, m_size);
	}
	//----------------------------------------------------------------------------//
	uint8* GLBuffer::Map(MappingMode _mode, uint _offset, uint _size)
	{
		if (_mode == AM_None || !_size)
			return nullptr;

		R_DEBUG_NODE();

		uint _access = _mode & AM_ReadWrite;
		if ((m_cpuAccess & _access) != _access)
		{
			R_DEBUG_ERROR("Error: Couldn't map buffer %d: No access", m_handle);
			return nullptr;
		}
		if (m_isMapped)
		{
			R_DEBUG_ERROR("Error: Couldn't map buffer %d: Buffer already mapped", m_handle);
			return nullptr;
		}
		if (_offset + _size > m_size)
		{
			R_DEBUG_ERROR("Error: Couldn't map buffer %d: Range [%u .. %u] is out of buffer with size %d", m_handle, _offset, _offset + _size, m_size);
			return nullptr;
		}

		void* _mappedData = nullptr;

		if (GLVersion >= 45)
		{
			_mappedData = glMapNamedBufferRange(m_handle, _offset, _size, GLMappingMode(_mode));
		}
		else if (GLVersion >= 33)
		{
			if (ogl_ext_EXT_direct_state_access)
			{
				_mappedData = glMapNamedBufferRangeEXT(m_handle, _offset, _size, GLMappingMode(_mode));
			}
			else
			{
				_Bind();
				_mappedData = glMapBufferRange(GL_ARRAY_BUFFER, _offset, _size, GLMappingMode(_mode));
			}
		}
		else // 2.1
		{
			// sync of data
			if (m_mappedData->storage) // use local copy
			{
				m_mappedData->ptr = m_mappedData->storage + _offset;
			}
			else if (_offset == 0 && _size == m_size) // map all buffer
			{
				if (ogl_ext_EXT_direct_state_access)
				{
					m_mappedData->ptr = (uint8*)glMapNamedBufferEXT(m_handle, GLAccessMode(_mode));
				}
				else
				{
					_Bind();
					m_mappedData->ptr = (uint8*)glMapBuffer(GL_ARRAY_BUFFER, GLAccessMode(_mode));
				}
			}
			else // use temp buffer
			{
				ASSERT(m_mappedData->tempBuffer == nullptr, "Memory leak");
				m_mappedData->tempBuffer = new uint8[_size];
				m_mappedData->ptr = m_mappedData->tempBuffer;

				if ((_mode & AM_Read) || _mode == MM_WriteNoOverwrite)
				{
					if (ogl_ext_EXT_direct_state_access)
					{
						glGetNamedBufferSubDataEXT(m_handle, _offset, _size, m_mappedData->ptr);
					}
					else
					{
						_Bind();
						glGetBufferSubData(GL_ARRAY_BUFFER, _offset, _size, m_mappedData->ptr);
					}
				}
			}

			m_mappedData->offset = _offset;
			m_mappedData->size = _size;
			m_mappedData->mode = _mode;
			_mappedData = m_mappedData->ptr;
		}

		m_isMapped = _mappedData != nullptr;
		return (uint8*)_mappedData;
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::Unmap(void)
	{
		if (!m_isMapped)
			return;

		R_DEBUG_NODE();

		if (GLVersion >= 45)
		{
			glUnmapNamedBuffer(m_handle);
		}
		else if (GLVersion >= 33)
		{
			if (ogl_ext_EXT_direct_state_access)
			{
				glUnmapNamedBufferEXT(m_handle);
			}
			else
			{
				_Bind();
				glUnmapBuffer(GL_ARRAY_BUFFER);
			}
		}
		else // 2.1
		{
			// sync of data
			if ((m_mappedData->mode & AM_Write) && m_usage != HBU_Staging)
			{
				if (m_mappedData->offset == 0 && m_mappedData->size == m_size && m_mappedData->mode != MM_WriteDiscard)
				{
					if (ogl_ext_EXT_direct_state_access)
					{
						glUnmapNamedBufferEXT(m_handle);
					}
					else
					{
						_Bind();
						glUnmapBuffer(GL_ARRAY_BUFFER);
					}
				}
				else
				{
					if (ogl_ext_EXT_direct_state_access)
					{
						glNamedBufferSubDataEXT(m_handle, m_mappedData->offset, m_mappedData->size, m_mappedData->ptr);
					}
					else
					{
						_Bind();
						glBufferSubData(GL_ARRAY_BUFFER, m_mappedData->offset, m_mappedData->size, m_mappedData->ptr);
					}
				}
			}

			// cleanup
			if (m_mappedData->tempBuffer)
			{
				delete[] m_mappedData->tempBuffer;
				m_mappedData->tempBuffer = nullptr;
			}
			m_mappedData->ptr = nullptr;
			m_mappedData->mode = MM_None;
		}

		m_isMapped = false;
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::Write(const void* _src, uint _offset, uint _size)
	{
		R_DEBUG_NODE();

		if (!(m_cpuAccess & AM_Write))
		{
			R_DEBUG_ERROR("Couldn't write data to buffer %d: No access", m_handle);
			return;
		}
		if (m_isMapped)
		{
			R_DEBUG_ERROR("Couldn't write data to buffer %d: Buffer is mapped", m_handle);
			return;
		}
		if (!_src)
		{
			R_DEBUG_ERROR("Couldn't write data to buffer %d: No source", m_handle);
			return;
		}
		if (_offset + _size > m_size)
		{
			R_DEBUG_ERROR("Couldn't write data to buffer %d: Range [%u .. %u] is out of buffer with size %d", m_handle, _offset, _offset + _size, m_size);
			return;
		}

		if (GLVersion >= 45)
		{
			glNamedBufferSubData(m_handle, _offset, _size, _src);
		}
		else if (GLVersion >= 33)
		{
			if (ogl_ext_EXT_direct_state_access)
			{
				glNamedBufferSubDataEXT(m_handle, _offset, _size, _src);
			}
			else
			{
				_Bind();
				glBufferSubData(GL_ARRAY_BUFFER, _offset, _size, _src);
			}
		}
		else 
		{
			if (m_mappedData->storage)
				memcpy(m_mappedData->storage + _offset, _src, _size);

			if (m_usage != HBU_Staging)
			{
				if (ogl_ext_EXT_direct_state_access)
				{
					glNamedBufferSubDataEXT(m_handle, _offset, _size, _src);
				}
				else
				{
					_Bind();
					glBufferSubData(GL_ARRAY_BUFFER, _offset, _size, _src);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::Read(void* _dst, uint _offset, uint _size)
	{
		R_DEBUG_NODE();

		if (!(m_cpuAccess & AM_Read))
		{
			R_DEBUG_ERROR("Couldn't read data from buffer %d: No access", m_handle);
			return;
		}
		if (m_isMapped)
		{
			R_DEBUG_ERROR("Couldn't read data from buffer %d: Buffer is mapped", m_handle);
			return;
		}
		if (!_dst)
		{
			R_DEBUG_ERROR("Couldn't read data from buffer %d: No destination", m_handle);
			return;
		}
		if (_offset + _size > m_size)
		{
			R_DEBUG_ERROR("Couldn't read data from buffer %d: Range [%u .. %u] is out of buffer with size %d", m_handle, _offset, _offset + _size, m_size);
			return;
		}

		if (GLVersion >= 45)
		{
			glGetNamedBufferSubData(m_handle, _offset, _size, _dst);
		}
		else if (GLVersion >= 33)
		{
			if (ogl_ext_EXT_direct_state_access)
			{
				glGetNamedBufferSubDataEXT(m_handle, _offset, _size, _dst);
			}
			else
			{
				_Bind();
				glGetBufferSubData(GL_ARRAY_BUFFER, _offset, _size, _dst);
			}
		}
		else // 2.1
		{
			if (m_mappedData->storage)
			{
				memcpy(_dst, m_mappedData->storage + _offset, _size);
			}
			else
			{
				if (ogl_ext_EXT_direct_state_access)
				{
					glGetNamedBufferSubDataEXT(m_handle, _offset, _size, _dst);
				}
				else
				{
					_Bind();
					glGetBufferSubData(GL_ARRAY_BUFFER, _offset, _size, _dst);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size)
	{
		if (!_src || !_size)
			return;

		R_DEBUG_NODE();

		GLBuffer* _srcBuffer = static_cast<GLBuffer*>(_src);

		if (m_isMapped)
		{
			R_DEBUG_ERROR("Couldn't copy data to buffer %d: Buffer is mapped", m_handle);
			return;
		}
		if (_srcBuffer->m_isMapped)
		{
			R_DEBUG_ERROR("Couldn't copy data from buffer %d: Buffer is mapped", _srcBuffer->m_handle);
			return;
		}
		if (_dstOffset + _size > m_size)
		{
			R_DEBUG_ERROR("Couldn't copy data to buffer %d: Range [%u .. %u] is out of buffer with size %d", m_handle, _dstOffset, _dstOffset + _size, m_size);
			return;
		}
		if (_srcOffset + _srcBuffer->m_size > _srcBuffer->m_size)
		{
			R_DEBUG_ERROR("Couldn't copy data from buffer %d: Range [%u .. %u] is out of buffer with size %d", _srcBuffer->m_handle, _srcOffset, _srcOffset + _size, _srcBuffer->m_size);
			return;
		}

		if (GLVersion >= 45)
		{
			glCopyNamedBufferSubData(_srcBuffer->m_handle, m_handle, _srcOffset, _dstOffset, _size);
		}
		else if (GLVersion >= 33)
		{
			if (ogl_ext_EXT_direct_state_access)
			{
				glNamedCopyBufferSubDataEXT(_srcBuffer->m_handle, m_handle, _srcOffset, _dstOffset, _size);
			}
			else
			{
				if (GLState->currentReadBuffer != _srcBuffer)
				{
					GLState->currentReadBuffer = _srcBuffer;
					glBindBuffer(GL_COPY_READ_BUFFER, _srcBuffer->m_handle);
				}

				if (GLState->currentWriteBuffer != this)
				{
					GLState->currentWriteBuffer = this;
					glBindBuffer(GL_COPY_WRITE_BUFFER, m_handle);
				}

				glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, _srcOffset, _dstOffset, _size);
			}
		}
		else // 2.1
		{
			uint8* _tempBuffer = nullptr;
			uint8* _ptr = nullptr;

			// read
			if (_srcBuffer->m_mappedData->storage) // read from local copy
			{
				_ptr = _srcBuffer->m_mappedData->storage + _srcOffset;

				// copy to local copy
				if (m_mappedData->storage)
				{
					memcpy(m_mappedData->storage + _dstOffset, _ptr, _size);
				}
			}
			else // read from source buffer
			{
				if (m_mappedData->storage) // use existent memory
				{
					_ptr = m_mappedData->storage + _dstOffset;
				}
				else // use temp memory
				{
					_tempBuffer = new uint8[_size];
					_ptr = _tempBuffer;

					// read from source buffer
					if (ogl_ext_EXT_direct_state_access)
					{
						glGetNamedBufferSubDataEXT(_srcBuffer->m_handle, _srcOffset, _size, _ptr);
					}
					else
					{
						_srcBuffer->_Bind();
						glGetBufferSubData(GL_ARRAY_BUFFER, _srcOffset, _size, _ptr);
					}
				}
			}

			// write
			if (m_usage != HBU_Staging)
			{
				if (ogl_ext_EXT_direct_state_access)
				{
					glNamedBufferSubDataEXT(m_handle, _dstOffset, _size, _ptr);
				}
				else
				{
					_Bind();
					glBufferSubData(GL_ARRAY_BUFFER, _dstOffset, _size, _ptr);
				}
			}

			// cleanup
			if (_tempBuffer)
				delete[] _tempBuffer;
		}
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::_Bind(void)
	{
		R_DEBUG_NODE();

		if (GLState->currentBuffer != this)
		{
			GLState->currentBuffer = this;
			glBindBuffer(GL_ARRAY_BUFFER, m_handle);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLVertexFormat
	//----------------------------------------------------------------------------//

	const char* GLVertexAttribNames[] =
	{
		"vaPosition", // VA_Position
		"vaNormal", // VA_Normal
		"vaTangent", // VA_Tangent
		"vaBoneIndices", // VA_BoneIndices
		"vaBoneWeights", // VA_BoneWeights
		"vaColor", // VA_Color
		"vaTexCoord0", // VA_TexCoord0
		"vaTexCoord1", // VA_TexCoord1
		"vaTexCoord2", // VA_TexCoord2
		"vaTexCoord3", // VA_TexCoord3
		"vaGeneric0", // VA_Generic0
		"vaGeneric1", // VA_Generic1
		"vaGeneric2", // VA_Generic2
		"vaGeneric3", // VA_Generic3
		"vaGeneric4", // VA_Generic4
		"vaGeneric5", // VA_Generic5
	};

	const GLVertexAttribDesc GLVertexAttribFormats[] =
	{
		{ GL_NONE, 0, 0, false, false, 0, 0, 0 }, // VAF_None
		{ GL_HALF_FLOAT, 2, 4, false, false, 0, 0, 0 }, // VAF_Half2
		{ GL_HALF_FLOAT, 4, 8, false, false, 0, 0, 0 }, // VAF_Half4
		{ GL_FLOAT, 1, 4, false, false, 0, 0, 0 }, // VAF_Float
		{ GL_FLOAT, 2, 8, false, false, 0, 0, 0 }, // VAF_Float2
		{ GL_FLOAT, 3, 12, false, false, 0, 0, 0 }, // VAF_Float3
		{ GL_FLOAT, 4, 16, false, false, 0, 0, 0 }, // VAF_Float4
		{ GL_UNSIGNED_BYTE, 4, 4, false, true, 0, 0, 0 }, // VAF_UByte4
		{ GL_UNSIGNED_BYTE, 4, 4, true, false, 0, 0, 0 }, // VAF_UByte4N
		{ GL_BYTE, 4, 4, true, false, 0, 0, 0 }, // VAF_Byte4N
		{ GL_UNSIGNED_SHORT, 2, 4, false, true, 0, 0, 0 }, // VAF_UShort2
		{ GL_UNSIGNED_SHORT, 2, 4, true, false, 0, 0, 0 }, // VAF_UShort2N
		{ GL_UNSIGNED_SHORT, 4, 8, false, true, 0, 0, 0 }, // VAF_UShort4
		{ GL_UNSIGNED_SHORT, 4, 8, true, false, 0, 0, 0 }, // VAF_UShort4N
	};

	Mutex GLVertexFormat::s_mutex;
	HashMap<uint32, uint> GLVertexFormat::s_formatsTable;
	Array<GLVertexFormat*> GLVertexFormat::s_formats;

	//----------------------------------------------------------------------------//
	GLVertexFormat::GLVertexFormat(const VertexFormatDesc& _desc, uint32 _hash, uint _index) :
		VertexFormat(_desc),
		m_hash(_hash),
		m_index(_index),
		m_numStreams(0)
	{
		memset(m_streams, 0xff, sizeof(m_streams));
		memset(m_streamIndices, 0xff, sizeof(m_streamIndices));

		for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
		{
			const VertexAttribDesc& _desc = m_desc[i];
			if (_desc.format != VAF_None)
			{
				uint8 _streamIndex = m_streamIndices[_desc.stream];
				if (m_streamIndices[_desc.stream] >= MAX_VERTEX_STREAMS)
				{
					_streamIndex = m_numStreams++;
					m_streamIndices[_desc.stream] = _streamIndex;
					m_streams[_streamIndex].index = _desc.stream;
					m_streams[_streamIndex].numAttribs = 0;
					m_streams[_streamIndex].stride = 0;
				}

				GLVertexStreamDesc& _stream = m_streams[_streamIndex];
				GLVertexAttribDesc& _attrib = _stream.attribs[_stream.numAttribs++];
				_attrib = GLVertexAttribFormats[_desc.format];
				_attrib.offset = _desc.offset;
				_attrib.index = i;
				_attrib.streamIndex = _streamIndex;
			}
		}

		// compute stride
		for (uint i = 0; i < m_numStreams; ++i)
		{
			GLVertexStreamDesc& _stream = m_streams[i];
			for (uint j = 0; j < _stream.numAttribs; ++j)
			{
				GLVertexAttribDesc& _attrib = _stream.attribs[j];
				_stream.stride = Max<uint16>(_stream.stride, _attrib.offset + _attrib.size);
			}
		}
	}
	//----------------------------------------------------------------------------//
	GLVertexFormat::~GLVertexFormat(void)
	{
	}
	//----------------------------------------------------------------------------//
	void GLVertexFormat::CreateDefaults(void)
	{
		CHECK_RENDER_THREAD();

		VertexFormatDesc _empty;
		Create(_empty); // with index 0
	}
	//----------------------------------------------------------------------------//
	VertexFormat* GLVertexFormat::Create(const VertexFormatDesc& _desc)
	{
		uint _hash = Crc32(_desc);

		SCOPE_LOCK(s_mutex);

		auto _exists = s_formatsTable.find(_hash);
		if (_exists != s_formatsTable.end())
			return s_formats[_exists->second];

		uint _index = (uint)s_formats.size();
		GLVertexFormat* _newFormat = new GLVertexFormat(_desc, _hash, _index);
		s_formats.push_back(_newFormat);
		s_formatsTable[_hash] = _index;

		return _newFormat;
	}
	//----------------------------------------------------------------------------//
	void GLVertexFormat::ReleaseResources(void)
	{
		CHECK_RENDER_THREAD();

		s_formatsTable.clear();
		while (!s_formats.empty())
		{
			delete s_formats.back();
			s_formats.pop_back();
		}
	}
	//----------------------------------------------------------------------------//
	GLVertexFormat* GLVertexFormat::GetFormat(uint _index)
	{
		SCOPE_READ(s_mutex);
		return s_formats[_index];
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLVertexArray
	//----------------------------------------------------------------------------//
	
	CriticalSection GLVertexArray::s_mutex;
	Array<uint> GLVertexArray::s_toDelete;
	GLVertexArray* GLVertexArray::s_current;
	GLBuffer* GLVertexArray::s_currentIndexBuffer;
	uint GLVertexArray::s_attribSet = 0;

	//----------------------------------------------------------------------------//
	GLVertexArray::GLVertexArray(GLVertexFormat* _format) :
		m_format(_format),
		m_updated(false),
		m_handle(0),
		m_numStreams(0),
		m_baseVertex(0),
		m_attribSet(0)
	{
		R_DEBUG_NODE();
		ASSERT(m_format != nullptr);

		m_numStreams = _format->GetNumStreams();
		memset(m_bufferIds, 0, sizeof(m_bufferIds));
		memset(m_bufferOffsets, 0, sizeof(m_bufferOffsets));
		memset(m_bufferStrides, 0, sizeof(m_bufferStrides));
	}
	//----------------------------------------------------------------------------//
	GLVertexArray::~GLVertexArray(void)
	{
		if (GLVersion >= 33 && m_handle)
		{
			SCOPE_LOCK(s_mutex);
			s_toDelete.push_back(m_handle);
		}
	}
	//----------------------------------------------------------------------------//
	void GLVertexArray::SetBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride)
	{
		R_DEBUG_NODE();

		uint _index = m_format->GetStreamIndex(_slot);
		if (_index < m_numStreams)
		{
			GLVertexStream& _stream = m_streams[_index];
			_stream.buffer = static_cast<GLBuffer*>(_buffer);
			_stream.offset = _offset;
			_stream.stride = _stride ? _stride : m_format->GetStreams()[_index].stride;	// when stride is unspecified, use precomputed stride
			_stream.updated = false;
			m_updated = false;

			if (GLVersion > 43)
			{
				m_bufferIds[_index] = _buffer ? static_cast<GLBuffer*>(_buffer)->Handle() : 0;
				m_bufferOffsets[_index] = _offset;
				m_bufferStrides[_index] = _stream.stride;
			}
			else if (GLVersion == 21)
			{
				const GLVertexStreamDesc& _streamDesc = m_format->GetStreams()[_index];
				if (_buffer)
				{
					for (uint i = 0, numAttribs = _streamDesc.numAttribs; i < numAttribs; ++i)
						m_attribSet |= 1 << _streamDesc.attribs[i].index;
				}
				else
				{
					for (uint i = 0, numAttribs = _streamDesc.numAttribs; i < numAttribs; ++i)
						m_attribSet &= ~(1 << _streamDesc.attribs[i].index);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLVertexArray::Bind(uint _baseVertex)
	{
		CHECK_RENDER_THREAD_D();

 		if (GLVersion >= 33)
		{
			if (!m_updated)
			{
				m_updated = true;
				if (GLVersion >= 43)
				{
					if (m_handle)
					{
						if (s_current != this)
						{
							s_current = this;
							glBindVertexArray(m_handle);
						}
					}
					else
					{
						glGenVertexArrays(1, &m_handle);
						glBindVertexArray(m_handle);
						s_current = this;

						// set vertex format
						for (uint i = 0, numStreams = m_format->GetNumStreams(); i < numStreams; ++i)
						{
							const GLVertexStreamDesc& _streamDesc = m_format->GetStreams()[i];
							for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
							{
								const GLVertexAttribDesc& _attrib = _streamDesc.attribs[j];
								glVertexAttribBinding(_attrib.index, i);
								if (_attrib.integer)
									glVertexAttribIFormat(_attrib.index, _attrib.components, _attrib.type, _attrib.offset);
								else
									glVertexAttribFormat(_attrib.index, _attrib.components, _attrib.type, _attrib.normalized, _attrib.offset);
							}
						}
					}

					// enable/disable attribs
					for (uint i = 0, numStreams = m_format->GetNumStreams(); i < numStreams; ++i)
					{
						const GLVertexStreamDesc& _streamDesc = m_format->GetStreams()[i];
						GLVertexStream& _stream = m_streams[i];

						if (_stream.buffer && !_stream.enabled) // enable
						{
							_stream.enabled = true;
							for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
								glEnableVertexAttribArray(_streamDesc.attribs[j].index);
						}
						else if (!_stream.buffer && _stream.enabled) // disable
						{
							_stream.enabled = false;
							for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
								glDisableVertexAttribArray(_streamDesc.attribs[j].index);
						}
					}

					// bind buffers
					glBindVertexBuffers(0, m_numStreams, m_bufferIds, m_bufferOffsets, m_bufferStrides);
				}
				else // 3.3
				{
					if (!m_handle)
						glGenVertexArrays(1, &m_handle);
					if (s_current != this)
					{
						s_current = this;
						glBindVertexArray(m_handle);
					}

					for (uint i = 0, numStreams = m_format->GetNumStreams(); i < numStreams; ++i)
					{
						const GLVertexStreamDesc& _streamDesc = m_format->GetStreams()[i];
						GLVertexStream& _stream = m_streams[i];

						if (!_stream.updated)
						{
							// bind vertex stream
							if (_stream.buffer)
							{
								_stream.buffer->_Bind();
								for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
								{
									const GLVertexAttribDesc& _attrib = _streamDesc.attribs[j];

									// enable attrib
									if (!_stream.enabled)
										glEnableVertexAttribArray(_attrib.index);

									// set vertex format
									if (_attrib.integer)
										glVertexAttribIPointer(_attrib.index, _attrib.components, _attrib.type, _stream.stride, (const void*)(uintptr_t)(_stream.offset + _attrib.offset));
									else
										glVertexAttribPointer(_attrib.index, _attrib.components, _attrib.type, _attrib.normalized, _stream.stride, (const void*)(uintptr_t)(_stream.offset + _attrib.offset));
								}
								_stream.enabled = true;
							}
							// disable vertex stream
							else if (_stream.enabled)
							{
								_stream.enabled = false;
								for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
									glDisableVertexAttribArray(_streamDesc.attribs[j].index);
							}
						}
					}
				}
			}
			else if (s_current != this)
			{
				s_current = this;
				glBindVertexArray(m_handle);
			}
		}
		else  // 2.1
		{
			m_updated = m_updated && s_current == this;

			// enable/disable attribs
			if (!m_updated)
			{
				for (uint i = 0, bit = 1; i < MAX_VERTEX_ATTRIBS; ++i, bit <<= 1)
				{
					bool _isEnabled = (s_attribSet & bit) != 0;
					bool _needEnable = (m_attribSet & bit) != 0;
					if (_needEnable && !_isEnabled)
						glEnableVertexAttribArray(i);
					else if (!_needEnable && _isEnabled)
						glDisableVertexAttribArray(i);
				}
				s_attribSet = m_attribSet;
			}

			// bind buffers
			if (m_baseVertex != _baseVertex || !m_updated)
			{
				m_baseVertex = _baseVertex;
				for (uint i = 0, numStreams = m_format->GetNumStreams(); i < numStreams; ++i)
				{
					const GLVertexStreamDesc& _streamDesc = m_format->GetStreams()[i];
					GLVertexStream& _stream = m_streams[i];

					if (_stream.buffer)
					{
						_stream.buffer->_Bind();
						uint _offset = _stream.offset + _stream.stride * _baseVertex;
						for (uint j = 0, numAttribs = _streamDesc.numAttribs; j < numAttribs; ++j)
						{
							const GLVertexAttribDesc& _attrib = _streamDesc.attribs[j];
							glVertexAttribPointer(_attrib.index, _attrib.components, _attrib.type, _attrib.normalized, _stream.stride, (const void*)(uintptr_t)(_stream.offset + _attrib.offset));
						}
					}
				}
			}

			m_updated = true;
			s_current = this;
		}
	}
	//----------------------------------------------------------------------------//
	void GLVertexArray::BindIndexBuffer(GLBuffer* _indexBuffer)
	{
		CHECK_RENDER_THREAD_D();

		if (GLVersion >= 33)
		{
			ASSERT(s_current == this);
			if (m_lastIndexBuffer != _indexBuffer)
			{
				m_lastIndexBuffer = _indexBuffer;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ? _indexBuffer->Handle() : 0);
			}
		}
		else if(s_currentIndexBuffer != _indexBuffer) // 2.1
		{
			s_currentIndexBuffer = _indexBuffer;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ? _indexBuffer->Handle() : 0);
		}
	}
	//----------------------------------------------------------------------------//
	void GLVertexArray::ReleaseResources(void)
	{
		CHECK_RENDER_THREAD_D();

		SCOPE_LOCK(s_mutex);
		if (!s_toDelete.empty())
		{
			glDeleteVertexArrays((int)s_toDelete.size(), &s_toDelete[0]);
			s_toDelete.clear();
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLRenderState
	//----------------------------------------------------------------------------//

	Ptr<GLVertexArray> GLRenderState::vertexArray;
	Ptr<GLBuffer> GLRenderState::indexBuffer;
	IndexType GLRenderState::indexType = IF_UShort;
	uint GLRenderState::indexOffset = 0;
	bool GLRenderState::primitveRestartEnabled = false;
	uint GLRenderState::primitveRestartIndex = 0;
	PrimitiveType GLRenderState::primitiveType = PT_Triangles;
	uint GLRenderState::patchSize = 0;

	//----------------------------------------------------------------------------//
	// GLRenderContext
	//----------------------------------------------------------------------------//

	namespace
	{
		THREAD_LOCAL GLRenderContext* tls_glrc = nullptr;
		THREAD_LOCAL bool isRenderThread = false;
	}

	//----------------------------------------------------------------------------//
	GLRenderContext* GLRenderContext::CurrentContext(void)
	{
		return tls_glrc;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderContext::IsRenderThread(void)
	{
		return isRenderThread;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderContext::Create(ShaderModel _shaderModel, uint _flags)
	{
		Destroy();

		coreProfile = (_flags & RSF_GL_CoreProfile) != 0;
		sRGB = (_flags & RSF_sRGB) != 0;
#ifdef DEBUG_RC
		debugContext = (_flags & RSF_DebugOutput) != 0;
#else
		debugContext = false;
#endif

		const uint _numVersions = 4;
		const uint _versions[_numVersions] = { 45, 43, 33, 21 };
		uint _minVersion = 21, _maxVersion = 45;
		switch (_shaderModel)
		{
		case SM3:
			_maxVersion = 21;
			break;

		case SM4:
			_minVersion = (_flags & RSF_AnyShaderModel) ? 21 : 33;
			_maxVersion = 33;
			break;

		case SM5:
			_minVersion = (_flags & RSF_AnyShaderModel) ? 21 : 43;
			_maxVersion = 45;
			break;
		}

		for (uint i = 0; i < _numVersions; ++i)
		{
			uint _version = _versions[i];
			if (_version >= _minVersion && _version <= _maxVersion)
			{
				SetupHints(_version, coreProfile, debugContext, sRGB);

				window = SDL_CreateWindow("GLContext", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
				if (window)
				{
					context = SDL_GL_CreateContext(window);
					if (context)
					{
						version = _version;
						break;
					}

					SDL_DestroyWindow(window);
					window = nullptr;
				}
			}
		}

		if (version == 21)
			coreProfile = false;

		return context != nullptr;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderContext::Create(const GLRenderContext& _sharedContext)
	{
		Destroy();

		_sharedContext.MakeCurrent();

		SetupHints(_sharedContext.version, _sharedContext.coreProfile, _sharedContext.debugContext, _sharedContext.sRGB);
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

		window = SDL_CreateWindow("GLContext", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
		if (window)
		{
			context = SDL_GL_CreateContext(window);
			if (context)
			{
				coreProfile = _sharedContext.coreProfile;
				debugContext = _sharedContext.debugContext;
				sRGB = _sharedContext.sRGB;
				version = _sharedContext.version;
			}
			else
			{
				SDL_DestroyWindow(window);
				window = nullptr;
			}
		}

		return context != nullptr;
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::Destroy(void)
	{
		if (context)
		{
			SDL_GL_DeleteContext(context);
			context = nullptr;
		}
		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}

		coreProfile = false;
		debugContext = false;
		sRGB = false;
		version = 0;
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::MakeCurrent(SDL_Window* _window) const
	{
		SDL_GL_MakeCurrent(_window ? _window : window, context);
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::SetupHints(void)
	{
		SetupHints(version, coreProfile, debugContext, sRGB);
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::SetupHints(uint _version, bool _coreProfile, bool _debugContext, bool _sRGB)
	{
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, _version / 10);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, _version % 10);
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, _sRGB);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, _coreProfile ? SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#ifdef DEBUG_RC
		if (_debugContext)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLWindow
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLWindow::GLWindow(SDL_Window* _handle) :
		Window(_handle)
	{
	}
	//----------------------------------------------------------------------------//
	GLWindow::~GLWindow(void)
	{
		if (!Thread::IsMain())
			LOG_ERROR("Window can be destroyed only in main thread");

		if (SDL_GL_GetCurrentWindow() == m_handle)
			gGLRenderSystem->_RestoreContext();
	}
	//----------------------------------------------------------------------------//
	void GLWindow::SwapBuffers(bool _vsync)
	{
#ifdef DEBUG_RC
		R_DEBUG_NODE();
		LOG_DISABLE(LL_RenderDebugOutput);
		//GLDebugPoint("SwapBuffers");
#endif
		SDL_GL_SetSwapInterval(_vsync ? 1 : 0);
		SDL_GL_SwapWindow(m_handle);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLRenderSystem
	//----------------------------------------------------------------------------//

	uint GLVersion = 0;
	uint GLRealVersion = 0;

	//----------------------------------------------------------------------------//
	GLRenderSystem::GLRenderSystem(void)
	{
		m_type = RST_GL;
	}
	//----------------------------------------------------------------------------//
	GLRenderSystem::~GLRenderSystem(void)
	{
		CHECK_RENDER_THREAD();

		GLVertexArray::ReleaseResources();
		GLVertexFormat::ReleaseResources();

		// todo: destroy background contexts

		m_mainContext.Destroy();
	}
	//----------------------------------------------------------------------------//
	bool GLRenderSystem::Init(ShaderModel _shaderModel, uint _flags)
	{
		// Init context
		if (!m_mainContext.Create(_shaderModel, _flags))
		{
			LOG_ERROR("OpenGL is not supported");
			return false;
		}
		tls_glrc = &m_mainContext;
		isRenderThread = true;

		// Load opengl
		if (!ogl_LoadFunctions())
		{
			LOG_ERROR("Couldn't load OpenGL functions");
			return false;
		}

		// Init version

		m_shaderModel = SM3;
		if (m_mainContext.version >= 33)
			m_shaderModel = SM4;
		if (m_mainContext.version >= 43)
			m_shaderModel = SM5;

		GLVersion = m_mainContext.version;
		GLRealVersion = ogl_GetMajorVersion() * 10 + ogl_GetMinorVersion();

		// Log version
		{
			LOG_INFO("OpenGL %s on %s, %s", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
			LOG_INFO("GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

			// Requested version
			String _msg = "Requested:";
			switch (_shaderModel)
			{
			case SM3: _msg += " OpenGL 2.1"; break;
			case SM4: _msg += " OpenGL 3.3"; break;
			case SM5: _msg += " OpenGL 4.3"; break;
			}

			if (_flags & RSF_GL_CoreProfile)
				_msg += " Core profile";
			else
				_msg += " Compatible profile";

#ifdef DEBUG_RC
			if (_flags & RSF_DebugOutput)
				_msg += " debug";
#endif
			_msg += " context";
			LOG_INFO("%s", _msg.CStr());

			// Created version
			_msg = String::Format("Created: OpenGL %d.%d", GLVersion / 10, GLVersion % 10);
			if (m_mainContext.coreProfile)
				_msg += " Core profile";
			else
				_msg += " Compatible profile";
#ifdef DEBUG_RC
			if (m_mainContext.debugContext)
				_msg += " debug";
#endif
			_msg += " context";
			LOG_INFO("%s", _msg.CStr());
		}

		// Check features
		{
			glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, (int*)&m_maxRenderTargets);
			if (m_maxRenderTargets < 2)
			{
				LOG_ERROR("MRT is not supported");
				return false;
			}
		}

		// Check features for OpenGL 2.1
		if (GLVersion == 21)
		{
			if (!ogl_ext_ARB_half_float_vertex && GLRealVersion < 30)
			{
				LOG_ERROR("Float16 vertex is not supported");
				return false;
			}

			if (!ogl_ext_ARB_framebuffer_object && GLRealVersion < 30)
			{
				LOG_ERROR("Framebuffer is not supported");
				return false;
			}

			if (!ogl_ext_EXT_stencil_two_side && GLRealVersion < 30)
			{
				LOG_ERROR("Two-sided stencil is not supported");
				return false;
			}

			if (!ogl_ext_ARB_texture_non_power_of_two && GLVersion < 30)
			{
				LOG_ERROR("NPOT textures is not supported");
				return false;
			}

			if (!ogl_ext_EXT_packed_depth_stencil && GLRealVersion < 30)
			{
				LOG_ERROR("Float textures is not supported");
				return false;
			}

			if (!ogl_ext_ARB_texture_float && !ogl_ext_ARB_color_buffer_float && !ogl_ext_ARB_half_float_pixel && GLRealVersion < 30)
			{
				LOG_ERROR("Float textures is not supported");
				return false;
			}
		}

		// Init debug
#ifdef DEBUG_RC
		GLInitDebug(m_mainContext.debugContext);
#endif

		// Log features
		{

		}

		GLVertexFormat::CreateDefaults();
		m_emptyVertexArray = new GLVertexArray(GLVertexFormat::GetFormat(0));
		//m_emptyVertexArray->_Update();

		return true;
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::DebugPoint(const char* _title)
	{
#ifdef DEBUG_RC
		GLDebugPoint(_title);
#endif
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::BeginFrame(void)
	{
		
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::EndFrame(void)
	{
#ifdef DEBUG_RC
		GLDebugPoint("Frame");
#endif
	}
	//----------------------------------------------------------------------------//
	WindowPtr GLRenderSystem::CreateWindow(void* _externalWindow)
	{
		LOG_NODE_DEFERRED("GLRenderSystem::CreateWindow");

		TODO("move to GLWindow");

		if (!Thread::IsMain())
		{
			LOG_ERROR("Window can be created only in main thread");
			return nullptr;
		}

		m_mainContext.SetupHints();
		//set hint SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT %#p = m_mainContext.window
		SDL_Window* _handle = nullptr;
		if (_externalWindow)
		{
			_handle = SDL_CreateWindowFrom(_externalWindow);
		}
		else
		{
			SDL_DisplayMode _dm;
			if (!SDL_GetDesktopDisplayMode(0, &_dm))
			{
				_handle = SDL_CreateWindow("GLWindow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (_dm.w >> 2) * 3, (_dm.h >> 2) * 3, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
			}
			else
			{
				LOG_ERROR("Couldn't get desktop display mode");
			}
		}


		if (!_handle)
		{
			LOG_ERROR("Couldn't create window: %s", SDL_GetError());
			return nullptr;
		}

		return new GLWindow(_handle);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::_RestoreContext(void)
	{
		FIXME("this is hack. remove it");
		m_mainContext.MakeCurrent();
	}
	//----------------------------------------------------------------------------//
	VertexFormat* GLRenderSystem::CreateVertexFormat(const VertexFormatDesc& _desc)
	{
		return GLVertexFormat::Create(_desc);
	}
	//----------------------------------------------------------------------------//
	HardwareBufferPtr GLRenderSystem::CreateBuffer(HardwareBufferUsage _usage, AccessMode _cpuAccess, uint _size, const void* _data)
	{
		return new GLBuffer(_usage, _cpuAccess, _size, _data);
	}
	//----------------------------------------------------------------------------//
	VertexArrayPtr GLRenderSystem::CreateVertexArray(VertexFormat* _format)
	{
		GLVertexFormat* _formatImpl = static_cast<GLVertexFormat*>(_format);
		if (!_formatImpl)
			_formatImpl = GLVertexFormat::GetFormat(0);
		return new GLVertexArray(_formatImpl);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::SetGeometry(VertexArray* _vertices, HardwareBuffer* _indices, IndexType _indexType, uint _indexOffset)
	{
		CHECK_RENDER_THREAD();

		GLRenderState::vertexArray = static_cast<GLVertexArray*>(_vertices);
		GLRenderState::indexBuffer = static_cast<GLBuffer*>(_indices);
		GLRenderState::indexOffset = _indexOffset;
		GLRenderState::indexType = _indexType;
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::Draw(PrimitiveType _type, uint _baseVertex, uint _numElements)
	{
		_Draw(_type, _baseVertex, 0, _numElements, 1, false);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::DrawInstanced(PrimitiveType _type, uint _baseVertex, uint _numElements, uint _numInstances)
	{
		_Draw(_type, _baseVertex, 0, _numElements, _numInstances, false);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::DrawIndexed(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements)
	{
		_Draw(_type, _baseVertex, _baseIndex, _numElements, 1, true);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::DrawIndexedInstanced(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements, uint _numInstances)
	{
		_Draw(_type, _baseVertex, _baseIndex, _numElements, _numInstances, true);
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::_Draw(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements, uint _numInstances, bool _indexed)
	{
		if (!_numInstances || !_numElements)
			return;

		CHECK_RENDER_THREAD();

		if (!GLRenderState::vertexArray)
		{
			R_DEBUG_ERROR("Unable to draw: No vertex array");
			return;
		}
		if (_indexed && !GLRenderState::indexBuffer)
		{
			R_DEBUG_ERROR("Unable to draw: No index buffer");
			return;
		}
		if (GLVersion == 21 && _numInstances > 1)
		{
			R_DEBUG_ERROR("Unable to draw: Draw instanced is not supported");
			return;
		}

		// set primitive type
		const GLPrimitiveInfo& _primInfo = GLPrimitiveTypes[_type];
		if (_type != GLRenderState::primitiveType)
		{
			if (_primInfo.minVersion > GLVersion)
			{
				R_DEBUG_ERROR("Unable to draw: Primitive type '%s' is not supported", _primInfo.name);
				return;
			}

			GLRenderState::primitiveType = _type;

			if (GLVersion >= 43)
			{
				if (_primInfo.isStrip)
				{
					// enable primitive restart
					if (_primInfo.usePritiveRestart && !GLRenderState::primitveRestartEnabled)
					{
						GLRenderState::primitveRestartEnabled = true;
						glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
					}
					// disable primitive restart
					else if (!_primInfo.usePritiveRestart && GLRenderState::primitveRestartEnabled)
					{
						GLRenderState::primitveRestartEnabled = false;
						glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
					}
				}
				// set patch size
				else if (_primInfo.isPatch && GLRenderState::patchSize != _primInfo.vertices)
				{
					GLRenderState::patchSize = _primInfo.vertices;
					glPatchParameteri(GL_PATCH_VERTICES, _primInfo.vertices);
				}
			}
			else if(GLVersion >= 33 && _primInfo.isStrip)
			{
				if (_primInfo.usePritiveRestart)
				{
					// set primitive restart index
					uint _restartIndex = GLRenderState::indexType == IF_UShort ? 0xffff : 0xffffffff;
					if (GLRenderState::primitveRestartIndex != _restartIndex)
					{
						GLRenderState::primitveRestartIndex = _restartIndex;
						glPrimitiveRestartIndex(_restartIndex);
					}

					// enable primitive restart
					if (GLRenderState::primitveRestartEnabled)
					{
						GLRenderState::primitveRestartEnabled = true;
						glEnable(GL_PRIMITIVE_RESTART);
					}
				}
				// disable primitive restart
				else if (GLRenderState::primitveRestartEnabled)
				{
					GLRenderState::primitveRestartEnabled = false;
					glDisable(GL_PRIMITIVE_RESTART);
				}
			}
		}

		//_ApplyChanges();

		if (_indexed)
		{
			GLRenderState::vertexArray->Bind(_baseVertex);
			GLRenderState::vertexArray->BindIndexBuffer(GLRenderState::indexBuffer);

			const void* _indexPtr = (const void*)(uintptr_t)(_baseIndex * GLRenderState::indexType + GLRenderState::indexOffset);
			uint _indexType = GLRenderState::indexType == IF_UShort ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			if (GLVersion >= 33)
			{
				glDrawElementsInstancedBaseVertex(_primInfo.type, _numElements, _indexType, _indexPtr, _numInstances, _baseVertex);
			}
			else // 2.1
			{
				glDrawElements(_primInfo.type, _numElements, _indexType, _indexPtr);
			}
		}
		else
		{
			GLRenderState::vertexArray->Bind(0);
				
			if (GLVersion >= 33)
			{
				glDrawArraysInstanced(_primInfo.type, _baseVertex, _numElements, _numInstances);
			}
			else // 2.1
			{
				// crashed in NVIDIA  driver 344 (OpenGL 4.5 core)
				glDrawArrays(_primInfo.type, _baseVertex, _numElements);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void GLRenderSystem::_ApplyChanges(void)
	{

	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Creator
	//----------------------------------------------------------------------------//

	RenderSystemPtr _CreateGLRenderSystem(ShaderModel _shaderModel, uint _flags)
	{
		Ptr<GLRenderSystem> _rs = new GLRenderSystem;
		if (!_rs->Init(_shaderModel, _flags))
			return nullptr;

		return _rs.Get();
	}

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

#endif
