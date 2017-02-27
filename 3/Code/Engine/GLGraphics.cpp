#include "GLGraphics.hpp"
#include "Window.hpp"
#include "Timer.hpp"
#include "GLGraphicsBackend.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Driver thread marker
	//----------------------------------------------------------------------------//

	namespace
	{
		THREAD_LOCAL bool isDriverThread = false;
	}

	//----------------------------------------------------------------------------//
	bool IsDriverThread(void)
	{
		return isDriverThread;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLContext
	//----------------------------------------------------------------------------//

	class GLContext final : public NonCopyable
	{
		friend class GLRenderDevice;

		bool Create(ShaderModel _minShaderModel, RenderContextProfile _profile, bool _debugContext)
		{
			ASSERT(window == nullptr);
			ASSERT(context == nullptr);

			debugContext = _debugContext;
			profile = _profile;
			minShaderModel = _minShaderModel == SM_Unknown ? SM4 : _minShaderModel;
			maxShaderModel = SM_Unknown;

			const uint _numVersions = 2;
			const uint _versions[_numVersions] = { 43, 33 };
			const ShaderModel _shaderModels[_numVersions] = { SM5, SM4 };
			uint _minVersion = 33, _maxVersion = 43;
			switch (minShaderModel)
			{
			case SM4:
				_minVersion = 33;
				if (profile != RCP_Forward)
					_maxVersion = 33;
				break;

			case SM5:
				_minVersion = 43;
				if (profile != RCP_Forward)
					_maxVersion = 43;
				break;
			}

			for (uint i = 0; i < _numVersions; ++i)
			{
				uint _version = _versions[i];
				if (_version >= _minVersion && _version <= _maxVersion)
				{
					SetupHints(_version, profile, debugContext);

					window = SDL_CreateWindow("GLContext", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
					if (window)
					{
						context = SDL_GL_CreateContext(window);
						if (context)
						{
							maxShaderModel = _shaderModels[i];
							version = _version;
							break;
						}

						SDL_DestroyWindow(window);
						window = nullptr;
					}
				}
			}

			if (!context)
			{
				LOG_MSG(LL_Error, "Couldn't create OpenGL context: Minimal version %d.%d is not supported", _minVersion / 10, _minVersion % 10);
				return false;
			}

			return true;
		}
		void Destroy(void)
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
		}
		bool MakeCurrent(SDL_Window* _window = nullptr) const
		{
			return SDL_GL_MakeCurrent(_window ? _window : window, context) == 0;
		}
		static void SetupHints(uint _version, RenderContextProfile _profile, bool _debugContext)
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
			SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, true);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, _profile == RCP_Compatible ? SDL_GL_CONTEXT_PROFILE_COMPATIBILITY : SDL_GL_CONTEXT_PROFILE_CORE);
			if (_debugContext)
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}

		SDL_Window* window = nullptr;
		SDL_GLContext context = nullptr;
		ShaderModel minShaderModel = SM_Unknown;
		ShaderModel maxShaderModel = SM_Unknown;
		uint version = 0; // 3.3, 4.3
		RenderContextProfile profile = RCP_Core;
		bool debugContext = false;
	};


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	const uint GLCompareFuncs[] =
	{
		GL_NEVER, // CF_Never
		GL_ALWAYS, // CF_Always
		GL_LESS, // CF_Less
		GL_LEQUAL, // CF_LessEqual
		GL_EQUAL, // CF_Equal
		GL_NOTEQUAL, // CF_NotEqual
		GL_GREATER, // CF_Greater
		GL_GEQUAL, // CF_GreaterEqual
	};

	const uint GLStencilOps[] =
	{
		GL_KEEP, // SO_Keep
		GL_ZERO, // SO_Zero
		GL_REPLACE, // SO_Replace
		GL_INVERT, // SO_Invert
		GL_INCR_WRAP, // SO_IncrWrap
		GL_DECR_WRAP, // SO_DecrWrap
		GL_INCR, // SO_Incr
		GL_DECR, // SO_Decr
	};


	const uint GLTextureTypes[] =
	{
		0, // TT_Unknown
		GL_TEXTURE_2D, // TT_2D
		GL_TEXTURE_2D_ARRAY, // TT_2DArray
		GL_TEXTURE_2D_MULTISAMPLE, // TT_2DMultisample
		GL_TEXTURE_3D, // TT_3D
		GL_TEXTURE_CUBE_MAP, // TT_Cube
		GL_TEXTURE_BUFFER, // TT_Buffer
	};

	enum PixelFormatFlags
	{
		PFF_Packed = 0x1,
		PFF_Compressed = 0x2,
		PFF_DepthStencil = 0x4,
	};

	struct GLPixelFormat
	{
		const char* name;
		uint bpp;
		uint internalFormat;
		uint format;
		uint type;
		PixelFormat closestFormat;
		uint flags;
		//bool compressed = false;
	}
	const GLPixelFormats[] =
	{
		{ "unknown", 0, 0, 0, 0, PF_Unknown, 0 }, // PF_Unknown

												  // unorm 
		{ "r8", 8, GL_R8, GL_RED, GL_UNSIGNED_BYTE, PF_RGTC1, 0 }, // PF_R8
		{ "rg8", 16, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, PF_RGTC2, 0 }, // PF_RG8
		{ "rgba8", 32, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, PF_DXT5, 0 }, // PF_RGBA8
		{ "rgb5a1", 32, GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE, PF_DXT1A, PFF_Packed }, // PF_RGB5A1 (?)
		{ "rgb10a2", 32, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, PF_DXT1A, PFF_Packed }, // PF_RGB10A2

																									   // float
		{ "r16f", 16, GL_R16F, GL_RED, GL_HALF_FLOAT, PF_RGTC1, 0 }, // PF_R16F
		{ "rg16f", 32, GL_RG16F, GL_RG, GL_HALF_FLOAT, PF_RGTC2, 0 }, // PF_RG16F
		{ "rgba16f", 64, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, PF_DXT5, 0 }, // PF_RGBA16F
		{ "r32f", 32, GL_R32F, GL_RED, GL_FLOAT, PF_RGTC1, 0 }, // PF_R32F
		{ "rg32f", 64, GL_RG32F, GL_RG, GL_FLOAT, PF_RGTC2, 0 }, // PF_RG32F
		{ "rgba32f", 128, GL_RGBA32F, GL_RGBA, GL_FLOAT, PF_DXT5, 0 }, // PF_RGBA32F
		{ "rg11b10f", 32, GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, PF_DXT1, PFF_Packed }, // PF_RG11B10F

																											 // integer
		{ "r16ui", 16, GL_R16UI, GL_RED, GL_UNSIGNED_SHORT, PF_RGTC1, 0 }, // PF_R16UI
		{ "rg16ui", 32, GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT, PF_RGTC2, 0 }, // PF_RG16UI
		{ "r32ui", 32, GL_R32UI, GL_RED, GL_UNSIGNED_INT, PF_RGTC1, 0 }, // PF_R32UI
		{ "rgb10a2ui", 32, GL_RGB10_A2UI, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, PF_DXT1A, PFF_Packed }, // PF_RGB10A2UI

																										   // depth/stencil
		{ "d24s8", 32, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, PF_Unknown, PFF_Packed | PFF_DepthStencil }, // PF_D24S8
		{ "d32f", 32, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, PF_Unknown, PFF_DepthStencil }, // PF_D32F

																										   // compressed
		{ "rgtc1", 4, GL_COMPRESSED_RED_RGTC1, 0, 0, PF_R8, PFF_Compressed }, // PF_RGTC1
		{ "rgtc2", 4, GL_COMPRESSED_RG_RGTC2, 0, 0, PF_RG8, PFF_Compressed }, // PF_RGTC2
		{ "dxt1", 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 0, 0, PF_RGB10A2, PFF_Compressed }, // PF_DXT1
		{ "dxt1a", 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0, PF_RGB10A2, PFF_Compressed }, // PF_DXT1A
		{ "dxt3", 8, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0, PF_RGBA8, PFF_Compressed }, // PF_DXT3
		{ "dxt5", 8, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0, PF_RGBA8, PFF_Compressed }, // PF_DXT5
	};

	const uint GLTextureFilters[] =
	{
		GL_NEAREST, // TF_Nearest
		GL_LINEAR, // TF_Linear
		GL_NEAREST_MIPMAP_NEAREST, // TF_Bilinear
		GL_LINEAR_MIPMAP_LINEAR, // TF_Trilinear
	};

	const uint GLTextureWraps[] =
	{
		GL_REPEAT, // TW_Repeat
		GL_CLAMP_TO_EDGE, // TW_Clamp
	};

	const uint GLShaderTypes[] =
	{
		GL_VERTEX_SHADER, // ST_Vertex
		GL_FRAGMENT_SHADER, // ST_Fragment
		GL_GEOMETRY_SHADER, // ST_Geometry
	};

	const char* GLShaderNames[] =
	{
		"Vertex", // ST_Vertex
		"Fragment", // ST_Fragment
		"Geometry", // ST_Geometry
	};

	//----------------------------------------------------------------------------//
	// GLState
	//----------------------------------------------------------------------------//

#define gGLState GLState::Get()

	struct GLState final : public NonCopyable
	{
		static GLState* Get(void) { return &s_instance; }
		static GLState s_instance;

		GLVertexStream vertexStreams[MAX_VERTEX_STREAMS]; // changed in GLVertexFormat::_BindBuffer()
		uint enabledVertexAttribs = 0; // changed in GLVertexFormat::_BindFormat()
		uint maxVertexAttribIndex = 0; // changed in GLVertexFormat::_BindFormat()
	};

	GLState GLState::s_instance;

	//----------------------------------------------------------------------------//
	// GLBuffer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLBuffer::GLBuffer(HwBufferUsage _usage, uint _size, const void* _data, int _memoryGroup) :
		m_usage(_usage),
		m_size(_size),
		m_memoryGroup(_memoryGroup),
		m_handle(new GLBufferHandle)
	{
		TODO("Update memory statistics");
		m_handle->Create(gGLResourceQueue, _usage, _size, _data);
	}
	//----------------------------------------------------------------------------//
	GLBuffer::~GLBuffer(void)
	{
		TODO("Update memory statistics");
		m_handle->Destroy();
	}
	//----------------------------------------------------------------------------//
	uint8* GLBuffer::Map(MappingMode _mode, uint _offset, uint _size)
	{
		return m_handle->Map(gGLResourceQueue, _mode, _offset, _size);
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::Unmap(void)
	{
		m_handle->Unmap(gGLResourceQueue);
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size)
	{
		ASSERT(_src != nullptr);

		m_handle->Copy(gGLResourceQueue, static_cast<GLBuffer*>(_src)->m_handle, _srcOffset, _dstOffset, _size);
	}
	//----------------------------------------------------------------------------//
	void GLBuffer::_Realloc(uint _newSize)
	{
		VERIFY_GL_SCOPE();
		TODO("Update memory statistics");

		glNamedBufferDataEXT(m_handle->glusage, _newSize, nullptr, m_handle->glusage);
		m_handle->size = _newSize;
		m_size = _newSize;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLVertexAttrib
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLVertexAttrib& GLVertexAttrib::operator = (const VertexAttribDesc& _desc)
	{
		VertexAttribDesc:: operator = (_desc);
		switch (type)
		{
		case VAT_Unknown:
			break;
		case VAT_Half2:
			size = 4, components = 2, normalized = false, integer = false, gltype = GL_HALF_FLOAT;
			break;
		case VAT_Half4:
			size = 8, components = 4, normalized = false, integer = false, gltype = GL_HALF_FLOAT;
			break;
		case VAT_Float:
			size = 4, components = 1, normalized = false, integer = false, gltype = GL_FLOAT;
			break;
		case VAT_Float2:
			size = 8, components = 2, normalized = false, integer = false, gltype = GL_FLOAT;
			break;
		case VAT_Float3:
			size = 12, components = 3, normalized = false, integer = false, gltype = GL_FLOAT;
			break;
		case VAT_Float4:
			size = 16, components = 4, normalized = false, integer = false, gltype = GL_FLOAT;
			break;
		case VAT_UByte4:
			size = 4, components = 4, normalized = false, integer = true, gltype = GL_UNSIGNED_BYTE;
			break;
		case VAT_UByte4N:
			size = 4, components = 4, normalized = true, integer = false, gltype = GL_UNSIGNED_BYTE;
			break;
		case VAT_Byte4N:
			size = 4, components = 4, normalized = true, integer = false, gltype = GL_BYTE;
			break;
		case VAT_UShort2:
			size = 4, components = 2, normalized = false, integer = true, gltype = GL_UNSIGNED_SHORT;
			break;
		case VAT_UShort2N:
			size = 4, components = 2, normalized = true, integer = false, gltype = GL_UNSIGNED_SHORT;
			break;
		case VAT_UShort4:
			size = 8, components = 4, normalized = false, integer = true, gltype = GL_UNSIGNED_SHORT;
			break;
		case VAT_UShort4N:
			size = 8, components = 4, normalized = true, integer = false, gltype = GL_UNSIGNED_SHORT;
			break;
		case VAT_Short4N:
			size = 8, components = 1, normalized = true, integer = false, gltype = GL_SHORT;
			break;
		default:
			ASSERT(false, "unknown enum");
			break;
		}

		return *this;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLVertexFormat
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLVertexFormat::GLVertexFormat(const VertexFormatDesc& _desc) :
		m_desc(_desc)
	{
		memset(m_streamIndices, -1, sizeof(m_streamIndices));

		for (uint8 i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
		{
			const VertexAttribDesc& _vad = m_desc[i];
			if (_vad.type != VAT_Unknown)
			{
				// add attrib
				uint _attribIndex = m_numAttribs++;
				GLVertexAttrib& _va = m_attribs[_attribIndex];
				m_attribIndices[i] = _attribIndex;
				_va = _vad;
				_va.userIndex = i;

				if (m_maxAttribIndex < i)
					m_maxAttribIndex = i;

				// add stream
				uint _streamIndex = m_streamIndices[_va.stream];
				if (_streamIndex >= m_numStreams)
				{
					_streamIndex = m_numStreams++;
					m_streams[_streamIndex].userIndex = _va.stream;
					m_streamIndices[_streamIndex] = _va.stream;
				}

				// update stream info
				GLVertexStreamInfo& _s = m_streams[_streamIndex];
				_s.attribs[_s.numAttribs++] = _attribIndex;
				_s.stride = Max<uint>(_s.stride, _va.offset + _va.size);
			}
		}
	}
	//----------------------------------------------------------------------------//
	GLVertexFormat::~GLVertexFormat(void)
	{
	}
	//----------------------------------------------------------------------------//
	void GLVertexFormat::_BindFormat(void)
	{
		VERIFY_GL_SCOPE();

		// bind format
		for (uint i = 0; i < m_numAttribs; ++i)
		{
			const GLVertexAttrib& _attrib = m_attribs[i];
			glVertexAttribBinding(_attrib.userIndex, _attrib.stream);
			glVertexAttribDivisor(_attrib.userIndex, _attrib.divisor);
			if (_attrib.integer)
				glVertexAttribIFormat(_attrib.userIndex, _attrib.components, _attrib.gltype, _attrib.offset);
			else
				glVertexAttribFormat(_attrib.userIndex, _attrib.components, _attrib.gltype, _attrib.normalized, _attrib.offset);
		}

		// enable/disable attributes
		for (uint i = 0, _mask = 1, _num = Max(m_maxAttribIndex, gGLState->maxVertexAttribIndex); i < _num; ++i, _mask <<= 1)
		{
			if (m_attribIndices[i] < m_numAttribs) // enable
			{
				if ((gGLState->enabledVertexAttribs & _mask) == 0)
				{
					glEnableVertexAttribArray(i);
					gGLState->enabledVertexAttribs |= _mask;
				}
			}
			else // disable
			{
				if ((gGLState->enabledVertexAttribs & _mask) != 0)
				{
					glDisableVertexAttribArray(i);
					gGLState->enabledVertexAttribs ^= _mask;
				}
			}
		}
		gGLState->maxVertexAttribIndex = m_maxAttribIndex;
	}
	//----------------------------------------------------------------------------//
	void GLVertexFormat::_BindBuffer(uint _stream, GLBuffer* _buffer, uint _offset, uint _stride)
	{
		VERIFY_GL_SCOPE();

		uint _streamIndex = GetStreamInternalIndex(_stream);
		if (_stream < m_numStreams)
		{
			GLVertexStream& _s = gGLState->vertexStreams[_streamIndex];
			const GLVertexStreamInfo& _sf = m_streams[_streamIndex];

			if (_stride == 0)
				_stride = _sf.stride;

			glBindVertexBuffer(_stream, _buffer->_Handle()->glhandle, _offset, _stride);
			_s.buffer = _buffer;
			_s.offset = _offset;
			_s.stride = _stride;
		}
#ifdef DEBUG_RC
		else
		{
			gGLDebugHelper->Message(__FUNCTION__, __FILE__, __LINE__, "", "", "high", "Invalid index of vertex stream");
		}
#endif
	}
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// GLTexture
	//----------------------------------------------------------------------------//

	struct GLTextureHandle
	{
		uint glhandle = 0;
		uint gltarget = 0;

		struct Cmd_Create
		{
			GLTextureHandle* handle;
			TextureType type;
			TextureUsage usage;
			PixelFormat format;
			uint maxLods;
		};

		struct Cmd_Destroy
		{
			GLTextureHandle* handle;

			Cmd_Destroy(GLTextureHandle* _handle) : handle(_handle) { }
			void Exec(void) { delete handle; }
		};

		~GLTextureHandle(void)
		{
			// ...
		}
		void Destroy(void)
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

		void Create(void)
		{
			if (ogl_IsVersionGEQ(4, 5))
			{
				glCreateTextures(gltarget, 1, &glhandle);
			}
			else
			{
				glGenTextures(1, &glhandle);
				glBindTexture(gltarget, glhandle);
			}

			/*if (m_type == TT_Buffer)
			{
				m_buffer = new HwBuffer(m_usage == TU_Dynamic ? HBU_Dynamic : HBU_Default, 0);
				m_buffer->AddRef();
				glTexBuffer(GL_TEXTURE_2D, GLPixelFormats[m_format].internalFormat, m_buffer->_Handle()->glhandle);
			}

			if (m_type == TT_2DMultisample || m_type == TT_Buffer)
				m_maxLods = 1;
			*/
		}

	};

	//----------------------------------------------------------------------------//
	GLTexture::GLTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods)
	{

	}
	//----------------------------------------------------------------------------//
	GLTexture::~GLTexture(void)
	{

	}
	//----------------------------------------------------------------------------//
	bool GLTexture::Realloc(uint _width, uint _height, uint _depth)
	{
		return 0;
	}
	//----------------------------------------------------------------------------//
	uint8* GLTexture::Map(MappingMode _mode, uint _lod)
	{
		return 0;
	}
	//----------------------------------------------------------------------------//
	void GLTexture::Unmap(void)
	{
	}
	//----------------------------------------------------------------------------//
	HardwareBuffer* GLTexture::GetBuffer(void)
	{
		return 0;
	}
	//----------------------------------------------------------------------------//
	void GLTexture::SetLodRange(uint _baseLod, uint _numLods)
	{

	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#if 0

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	//TODO: check max texture units
	const uint GLDefaultTextureUnit = GL_TEXTURE16;
	//TODO: best usage hint	(need test)
	const uint GLDefaultTexturePackUnpackBufferUsage = GL_STATIC_COPY;

	//----------------------------------------------------------------------------//
	// HwDepthStencilState
	//----------------------------------------------------------------------------//

	HwDepthStencilState::HwDepthStencilState(const DepthStencilDesc& _desc)	:
		m_desc(_desc)
	{

	}
	//----------------------------------------------------------------------------//
	HwDepthStencilState::~HwDepthStencilState(void)
	{
	}
	//----------------------------------------------------------------------------//
	void HwDepthStencilState::_Bind(HwDepthStencilState* _current)
	{
		ASSERT(_current != nullptr);

		if (m_desc.depthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		glDepthMask(m_desc.depthWrite);
		glDepthFunc(GLCompareFuncs[m_desc.depthFunc]);


		if (m_desc.stencilTest)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(m_desc.stencilMask);
			if (m_desc.stencilFuncSeparate)
			{
				glStencilOpSeparate(GL_FRONT, GLStencilOps[m_desc.stencilFunc.fail], GLStencilOps[m_desc.stencilFunc.zfail], GLStencilOps[m_desc.stencilFunc.zpass]);
				glStencilFuncSeparate(GL_FRONT, GLCompareFuncs[m_desc.stencilFunc.func], m_desc.stencilRef, m_desc.stencilMask);
				glStencilOpSeparate(GL_BACK, GLStencilOps[m_desc.stencilBackFunc.fail], GLStencilOps[m_desc.stencilBackFunc.zfail], GLStencilOps[m_desc.stencilBackFunc.zpass]);
				glStencilFuncSeparate(GL_BACK, GLCompareFuncs[m_desc.stencilBackFunc.func], m_desc.stencilRef, m_desc.stencilMask);
			}
			else
			{
				glStencilOp(GLStencilOps[m_desc.stencilFunc.fail], GLStencilOps[m_desc.stencilFunc.zfail], GLStencilOps[m_desc.stencilFunc.zpass]);
				glStencilFunc(GLCompareFuncs[m_desc.stencilFunc.func], m_desc.stencilRef, m_desc.stencilMask);
			}
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// HwVertexArray 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	HwVertexArray::HwVertexArray(HwVertexFormat* _format) :
		m_format(_format),
		m_handle(0)
	{
		VERIFY_GL_SCOPE();

		int _currentVao = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &_currentVao);
		glGenVertexArrays(1, &m_handle);
		glBindVertexArray(m_handle);

		for (uint i = 0; i < m_format->GetNumElements(); ++i)
		{
			const HwVertexFormat::Element& _e = m_format->GetElement(i);
			glVertexAttribBinding(_e.index, _e.stream);
			glVertexAttribDivisor(_e.index, _e.divisor);
			if (_e.integer)
				glVertexAttribIFormat(_e.index, _e.components, _e.internalType, _e.offset);
			else
				glVertexAttribFormat(_e.index, _e.components, _e.internalType, _e.normalized, _e.offset);
		}

		m_streams.Resize(m_format->GetNumStreams());

		// restore state
		if (_currentVao)
			glBindVertexArray(_currentVao);
	}
	//----------------------------------------------------------------------------//
	HwVertexArray::~HwVertexArray(void)
	{
		VERIFY_GL_SCOPE();

		glDeleteVertexArrays(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	void HwVertexArray::SetBuffer(uint _stream, HwBuffer* _buffer, uint _offset, uint _stride)
	{
		VERIFY_GL_SCOPE();

		uint _streamIndex = m_format->GetStreamIndex(_stream);
		if (_stream < m_format->GetNumStreams())
		{
			Stream& _s = m_streams[_streamIndex];
			const HwVertexFormat::Stream& _sf = m_format->GetStreamByIndex(_streamIndex);

			if (_stride == 0)
				_stride = _sf.stride;

			if (_s.buffer != _buffer || (_buffer && (_s.offset != _offset || _s.stride != _stride)))
			{
				glVertexArrayBindVertexBufferEXT(m_handle, _stream, _buffer->_Handle()->glhandle, _offset, _stride);

				if (!_s.buffer && _buffer) // enable
				{
					for (uint i = 0, n = _sf.elements.Size(); i < n; ++i)
						glEnableVertexArrayAttribEXT(m_handle, _sf.elements[i]);
				}
				else if (_s.buffer && !_buffer)	// disable
				{
					for (uint i = 0, n = _sf.elements.Size(); i < n; ++i)
						glDisableVertexArrayAttribEXT(m_handle, _sf.elements[i]);
				}

				_s.buffer = _buffer;
				_s.offset = _offset;
				_s.stride = _stride;
			}
		}
	}
	//----------------------------------------------------------------------------//
	HwBuffer* HwVertexArray::GetBuffer(uint _stream, uint* _offset, uint* _stride)
	{
		uint _streamIndex = m_format->GetStreamIndex(_stream);
		if (_stream < m_format->GetNumStreams())
		{
			if (_offset)
				*_offset = m_streams[_streamIndex].offset;
			if (_stride)
				*_stride = m_streams[_streamIndex].stride;

			return m_streams[_streamIndex].buffer;
		}
		return nullptr;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HwTexture
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	HwTexture::HwTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods) :
		m_type(_type),
		m_usage(_usage),
		m_format(_format),
		m_size(0),
		m_baseLod(0),
		m_numLods(0),
		m_createdLods(0),
		m_maxLods((_maxLods == 0 || _maxLods > 31) ? 32 : (uint)_maxLods),
		m_mappedLod(0),
		m_mappedData(0),
		m_handle(0),
		m_buffer(nullptr)
	{
		VERIFY_GL_SCOPE();

		assert(_type != TT_Unknown);
		assert(_format != PF_Unknown);

		//glActiveTexture(GLWorkTextureUnit);
		glGenTextures(1, &m_handle);
		glBindTexture(GLTextureTypes[m_type], m_handle);

		if (m_type == TT_Buffer)
		{
			m_buffer = new HwBuffer(m_usage == TU_Dynamic ? HBU_Dynamic : HBU_Default, 0);
			m_buffer->AddRef();
			glTexBuffer(GL_TEXTURE_2D, GLPixelFormats[m_format].internalFormat, m_buffer->_Handle()->glhandle);
		}

		if (m_type == TT_2DMultisample || m_type == TT_Buffer)
			m_maxLods = 1;
	}
	//----------------------------------------------------------------------------//
	HwTexture::~HwTexture(void)
	{
		VERIFY_GL_SCOPE();

		SAFE_RELEASE(m_buffer);
		if (m_mappedData)
			glDeleteBuffers(1, &m_mappedData);
		glDeleteTextures(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	bool HwTexture::Realloc(uint _width, uint _height, uint _depth)
	{
		VERIFY_GL_SCOPE();

		if (m_mappingMode) // unable to reallocate mapped texture
			return false;

		// verify dimensions
		if (!_width || !_height) // empty textures is not supported	(???)
			return false;

		if (!_depth) // use current depth
			_depth = m_size.z;
		switch (m_type)
		{
		case TT_2D:
			_depth = 1;
			break;
		case TT_2DArray:
		case TT_3D:
			_depth = Max<uint>(_depth, 1);
			break;
		case TT_Cube:
			_depth = 6;
			break;
		case TT_2DMultisample:
			_depth = Clamp<uint>((_depth >> 1) << 1, 1, 16);
			break;
		case TT_Buffer:
			_height = 1, _depth = 1;
			break;
		}

		if (_width == m_size.x && _height == m_size.y && _depth == m_size.z) // no changes
			return true;

		/*
		1. Удалить ненужные уровни детализации (при уменьшении текстуры)

		2. Некоторые драйверы не позволяют присоединять к fbo текстуры с одним уровнем детализации.
		AMD Catalyst (13.3) вообще вылетает с ошибкой нарушения доступа при отсутствии уровней дализации у текстуры присоединенной к fbo,
		поэтому неиспользуемые уровни детализации (+1) создаются с размером 1x1.

		3. Создать уровни детализации.
		*/

		const GLPixelFormat& _pf = GLPixelFormats[m_format];
		bool _isCompressed = (_pf.flags & PFF_Compressed) != 0;
		const GLPixelFormat& _upf = GLPixelFormats[_isCompressed ? _pf.closestFormat : m_format]; // uncompressed pixel format
		uint _mw = _isCompressed ? 4 : 1; // minimum width of block 
		uint _maxLodCount = Log2i(Max(Max(_width, _height), 1u)) + 1;
		uint _numLods = Min<uint>(_maxLodCount, m_maxLods);

		// allocate image
		switch (m_type)
		{
		case TT_2D:
			for (uint i = _maxLodCount; i <= m_createdLods; ++i)
				glTextureImage2DEXT(m_handle, GL_TEXTURE_2D, i, _pf.internalFormat, 0, 0, 0, _upf.format, _upf.type, nullptr);
			for (uint i = m_createdLods; i <= _maxLodCount; ++i)
				glTextureImage2DEXT(m_handle, GL_TEXTURE_2D, i, _pf.internalFormat, 1, 1, 0, _upf.format, _upf.type, nullptr);
			for (uint i = 0, _w = _width, _h = _height; i < _numLods; ++i)
				glTextureImage2DEXT(m_handle, GL_TEXTURE_2D, i, _pf.internalFormat, Min(_w >> i, 1u), Min(_h >> i, 1u), 0, _pf.format, _pf.type, nullptr);
			break;

		case TT_2DArray:
			for (uint i = _maxLodCount; i <= m_createdLods; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_2D_ARRAY, i, _pf.internalFormat, 0, 0, 0, 0, _upf.format, _upf.type, nullptr);
			for (uint i = m_createdLods; i <= _maxLodCount; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_2D_ARRAY, i, _pf.internalFormat, 1, 1, _depth, 0, _upf.format, _upf.type, nullptr);
			for (uint i = 0, _w = _width, _h = _height; i < _numLods; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_2D_ARRAY, i, _pf.internalFormat, Min(_w >> i, 1u), Min(_h >> i, 1u), _depth, 0, _upf.format, _upf.type, nullptr);
			break;

		case TT_2DMultisample:
			ASSERT(_isCompressed == false);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_handle);	// no direct state access for multisample textures
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _depth, _pf.internalFormat, _width, _height, true);
			break;

		case TT_3D:
			for (uint i = _maxLodCount; i <= m_createdLods; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_3D, i, _pf.internalFormat, 0, 0, 0, 0, _upf.format, _upf.type, nullptr);
			for (uint i = m_createdLods; i <= _maxLodCount; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_3D, i, _pf.internalFormat, 1, 1, 1, 0, _upf.format, _upf.type, nullptr);
			for (uint i = 0, _w = _width, _h = _height, _d = _height; i < _numLods; ++i)
				glTextureImage3DEXT(m_handle, GL_TEXTURE_3D, i, _pf.internalFormat, Min(_w >> i, 1u), Min(_h >> i, 1u), Min(_d >> i, 1u), 0, _upf.format, _upf.type, nullptr);
			break;

		case TT_Cube:
			for (uint f = 0; f < 6; ++f)
			{
				uint _target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
				for (uint i = _maxLodCount; i <= m_createdLods; ++i)
					glTextureImage2DEXT(m_handle, _target, i, _pf.internalFormat, 0, 0, 0, _upf.format, _upf.type, nullptr);
				for (uint i = m_createdLods; i <= _maxLodCount; ++i)
					glTextureImage2DEXT(m_handle, _target, i, _pf.internalFormat, 1, 1, 0, _upf.format, _upf.type, nullptr);
				for (uint i = 0, _w = _width, _h = _height; i < _numLods; ++i)
					glTextureImage2DEXT(m_handle, _target, i, _pf.internalFormat, Min(_w >> i, 1u), Min(_h >> i, 1u), 0, _upf.format, _upf.type, nullptr);
			}
			break;

		case TT_Buffer:
			ASSERT(_isCompressed == false);
			m_buffer->_Realloc(_width);
			break;
		}

		// create mappable buffer
		if (m_usage == TU_Dynamic && m_type != TT_Buffer && m_type != TT_2DMultisample)
		{
			if (!m_mappedData)
			{
				glGenBuffers(1, &m_mappedData);
				glBindBuffer(GL_ARRAY_BUFFER, m_handle);
			}
			glNamedBufferDataEXT(m_mappedData, Max(_width, _mw) * Max(_height, _mw) * _depth, nullptr, GLDefaultTexturePackUnpackBufferUsage); // alloc data
		}

		m_baseLod = 0;
		m_numLods = (uint8_t)_numLods;
		m_createdLods = (uint8_t)_numLods;
		m_size.Set(_width, _height, _depth);

		glTextureParameteriEXT(m_handle, GLTextureTypes[m_type], GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteriEXT(m_handle, GLTextureTypes[m_type], GL_TEXTURE_MAX_LEVEL, _numLods - 1);

		return true;
	}
	//----------------------------------------------------------------------------//
	uint8_t* HwTexture::Map(MappingMode _mode, uint _lod)
	{
		VERIFY_GL_SCOPE();

		if (!m_size.z) // texture not was initialized
			return nullptr;

		if (_lod > m_createdLods) // non-existent LOD
			return nullptr;

		if (!m_mappingMode) // already mapped
			return nullptr;

		if (m_type == TT_2DMultisample)	// multisample textures cannot be mapped
			return nullptr;

		m_mappedLod = (uint8_t)_lod;
		m_mappingMode = _mode;

		if (m_type == TT_Buffer)
			return m_buffer->Map(_mode, 0, m_buffer->GetSize());

		const GLPixelFormat& _pf = GLPixelFormats[m_format];
		uint _mw = (_pf.flags & PFF_Compressed) ? 4 : 1; // minimum width of block 
		uint _imgSize = ((Max<uint>(m_size.x >> _lod, _mw) * Max<uint>(m_size.y >> _lod, _mw) * _pf.bpp) >> 3) * (m_type == TT_3D ? Max<uint>(m_size.z >> _lod, 1) : m_size.z);

		if (m_usage != TU_Dynamic) // create temporary buffer
		{
			glGenBuffers(1, &m_mappedData);
			if (_mode != MM_Discard) // create buffer now
				glBindBuffer(GL_ARRAY_BUFFER, m_mappedData);
			glNamedBufferDataEXT(m_mappedData, _imgSize, nullptr, GLDefaultTexturePackUnpackBufferUsage); // alloc data
		}

		if (_mode != MM_Discard)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, m_mappedData);
			if (_pf.flags & PFF_Compressed)
				glGetCompressedTextureImageEXT(m_handle, GLTextureTypes[m_type], _lod, nullptr);
			else
				glGetTextureImageEXT(m_handle, GLTextureTypes[m_type], _lod, _pf.format, _pf.type, nullptr);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		}

		return (uint8_t*)glMapNamedBufferRangeEXT(m_mappedData, 0, _imgSize, GLMappingModes[_mode]);
	}
	//----------------------------------------------------------------------------//
	void HwTexture::Unmap(void)
	{
		VERIFY_GL_SCOPE();

		if (!m_mappingMode) // texture not was mapped 
			return;

		if (m_type == TT_Buffer)
		{
			m_buffer->Unmap();
		}
		else
		{
			if (m_mappingMode != MM_Read)
			{
				const GLPixelFormat& _pf = GLPixelFormats[m_format];
				uint _w = Max<uint>(m_size.x >> m_mappedLod, 1);
				uint _h = Max<uint>(m_size.y >> m_mappedLod, 1);
				uint _d = m_type == TT_3D ? Max<uint>(m_size.z >> m_mappedLod, 1) : m_size.z;
				uint _mw = (_pf.flags & PFF_Compressed) ? 4 : 1; // minimum width of block 
				uint _bpl = ((Max(_w, _mw) * Max(_h, _mw) * _pf.bpp) >> 3);
				uint _bpi = _bpl * _d;

				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_mappedData);
				if (_pf.flags & PFF_Compressed)
				{
					switch (m_type)
					{
					case TT_2D:
						glCompressedTextureImage2DEXT(m_handle, GL_TEXTURE_2D, m_mappedLod, _pf.internalFormat, _w, _h, 0, _bpi, nullptr);
						break;
					case TT_2DArray:
					case TT_3D:
						glCompressedTextureImage3DEXT(m_handle, GLTextureTypes[m_type], m_mappedLod, _pf.internalFormat, _w, _h, _d, 0, _bpi, nullptr);
						break;
					case TT_Cube:
						for (uint i = 0; i < 6; ++i)
							glCompressedTextureImage2DEXT(m_handle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_mappedLod, _pf.internalFormat, _w, _h, 0, _bpl, (void*)(_bpl*i));
						break;
					}
				}
				else
				{
					switch (m_type)
					{
					case TT_2D:
						glTextureImage2DEXT(m_handle, GL_TEXTURE_2D, m_mappedLod, _pf.internalFormat, _w, _h, 0, _pf.format, _pf.type, nullptr);
						break;
					case TT_2DArray:
					case TT_3D:
						glTextureImage3DEXT(m_handle, GLTextureTypes[m_type], m_mappedLod, _pf.internalFormat, _w, _h, m_size.z, 0, _pf.format, _pf.type, nullptr);
						break;
					case TT_Cube:
						for (uint i = 0; i < 6; ++i)
							glTextureImage2DEXT(m_handle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_mappedLod, _pf.internalFormat, _w, _h, 0, _pf.format, _pf.type, (void*)(_bpl*i));
						break;
					}
				}
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			}

			if (m_usage != TU_Dynamic)
			{
				glDeleteBuffers(1, &m_mappedData);
				m_mappedData = 0;
			}
		}

		m_mappingMode = 0;
	}
	//----------------------------------------------------------------------------//
	void HwTexture::SetBaseLod(uint _lod)
	{
		VERIFY_GL_SCOPE();

		if (_lod > m_numLods)
			_lod = m_numLods;
		if (_lod != m_baseLod)
		{
			glTextureParameteriEXT(m_handle, GLTextureTypes[m_type], GL_TEXTURE_BASE_LEVEL, _lod);
			m_baseLod = _lod;
		}
	}
	//----------------------------------------------------------------------------//
	void HwTexture::SetNumLods(uint _num)
	{
		VERIFY_GL_SCOPE();

		if (_num > m_createdLods)
			_num = m_createdLods;
		if (_num != m_numLods && _num > 1)
		{
			glTextureParameteriEXT(m_handle, GLTextureTypes[m_type], GL_TEXTURE_MAX_LEVEL, _num - 1);
			m_numLods = _num;
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HwSampler
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	HwSamplerPtr HwSampler::Create(TextureFilter _min, TextureFilter _mag, uint _aniso, TextureWrap _wrapS, TextureWrap _wrapT, TextureWrap _wrapR, CompareFunc _func)
	{
		return nullptr;
	}
	//----------------------------------------------------------------------------//
	HwSampler::HwSampler(TextureFilter _min, TextureFilter _mag, uint _aniso, TextureWrap _wrapS, TextureWrap _wrapT, TextureWrap _wrapR, CompareFunc _func) :
		m_depthFunc(_func),
		m_minFilter(_min),
		m_magFilter(_mag),
		m_wrapS(_wrapS),
		m_wrapT(_wrapT),
		m_wrapR(_wrapR),
		m_aniso(_aniso),
		m_minLod(0),
		m_maxLod(32),
		m_lodBias(0),
		m_handle(0)
	{
		glGenSamplers(1, &m_handle);
		glBindSampler(GLDefaultTextureUnit, m_handle);
		glSamplerParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GLTextureFilters[_min]);
		glSamplerParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GLTextureFilters[_mag]);
		glSamplerParameteri(m_handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, _aniso);
		glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_S, GLTextureWraps[_wrapS]);
		glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_T, GLTextureWraps[_wrapT]);
		glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_R, GLTextureWraps[_wrapR]);
		if (_func != CF_Always)
		{
			glSamplerParameteri(m_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glSamplerParameteri(m_handle, GL_TEXTURE_COMPARE_FUNC, GLCompareFuncs[_func]);
		}
	}
	//----------------------------------------------------------------------------//
	HwSampler::~HwSampler(void)
	{
		glDeleteSamplers(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	void HwSampler::SetMaxAnisotropy(uint _value)
	{
		_value = (_value >> 2) << 2;
		if (m_aniso != _value)
		{
			glSamplerParameteri(m_handle, GL_TEXTURE_MAX_ANISOTROPY_EXT, _value);
			m_aniso = (uint8_t)_value;
		}
	}
	//----------------------------------------------------------------------------//
	void HwSampler::SetMinLod(uint _value)
	{
		if (_value > 32)
			_value = 32;
		if (m_minLod != _value)
		{
			glSamplerParameteri(m_handle, GL_TEXTURE_MIN_LOD, _value);
			m_minLod = (uint8_t)_value;
		}
	}
	//----------------------------------------------------------------------------//
	void HwSampler::SetMaxLod(uint _value)
	{
		if (_value > 32)
			_value = 32;
		if (m_maxLod != _value)
		{
			glSamplerParameteri(m_handle, GL_TEXTURE_MAX_LOD, _value);
			m_maxLod = (uint8_t)_value;
		}
	}
	//----------------------------------------------------------------------------//
	void HwSampler::SetLodBias(float _value)
	{
		_value = Clamp(_value, 0.f, 1.f); // ???
		if (m_lodBias != _value)
		{
			glSamplerParameterf(m_handle, GL_TEXTURE_LOD_BIAS, _value);
			m_maxLod = (uint8_t)_value;
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HwShader
	//----------------------------------------------------------------------------//

	HashMap<uint, uint16> HwShader::s_nameTable;
	Array<String> HwShader::s_names;
	CriticalSection HwShader::s_namesMutex;

	//----------------------------------------------------------------------------//
	HwShader::HwShader(ShaderType _type) :
		m_type(_type),
		m_state(SS_Initial),
		m_version(0),
		m_handle(0)
	{
		m_handle = glCreateShader(GLShaderTypes[m_type]);
	}
	//----------------------------------------------------------------------------//
	HwShader::~HwShader(void)
	{
		glDeleteShader(m_handle);
	}
	//----------------------------------------------------------------------------//
	void HwShader::Compile(const String& _source)
	{
		const char* _srcp = _source;
		glShaderSource(m_handle, 1, &_srcp, nullptr);
		glCompileShader(m_handle);
		m_state = SS_Compiling;
		++m_version;
	}
	//----------------------------------------------------------------------------//
	uint16 HwShader::AddFileName(const String& _name)
	{
		uint _hash = _name.Hashi();

		SCOPE_LOCK(s_namesMutex);
		auto _exists = s_nameTable.find(_hash);
		if (_exists != s_nameTable.end())
			return _exists->second;

		ASSERT(s_names.size() < 0xffff);
		uint16 _id = (uint16)s_names.size();
		s_names.push_back(_name);
		s_nameTable[_hash] = _id;

		return _id;
	}
	//----------------------------------------------------------------------------//
	String HwShader::GetFileName(uint16 _id)
	{
		SCOPE_LOCK(s_namesMutex);
		return (size_t)_id < s_names.size() ? s_names[_id] : String::Empty;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// HwProgram
	//----------------------------------------------------------------------------//

	HashMap<uint, HwProgram*> HwProgram::s_instances;

	//----------------------------------------------------------------------------//
	HwProgramPtr HwProgram::Create(HwVertexFormat* _vfIn, HwVertexFormat* _vfOut, HwShader* _vs, HwShader* _fs, HwShader* _gs)
	{
		if (!_vs && !_fs && !_gs) // no shaders
			return nullptr;
		if (_gs && !_vs) // incomplete
			return nullptr;
		if (_fs && !_vs) // incomplete
			return nullptr;
		if (_vs && _vs->GetType() != ST_Vertex)	// unexpected type
			return nullptr;
		if (_fs && _fs->GetType() != ST_Fragment) // unexpected type
			return nullptr;
		if (_gs && _gs->GetType() != ST_Geometry) // unexpected type
			return nullptr;

		HwShader* _keys[3] = { _vs, _fs, _gs };
		uint _id = Crc32(_keys, sizeof(_keys));
		auto _exists = s_instances.find(_id);
		if (_exists != s_instances.end())
			return _exists->second;

		HwProgramPtr _prog = new HwProgram(_id, _vfIn, _vfOut, _vs, _fs, _gs);
		s_instances[_id] = _prog;

		return _prog;
	}
	//----------------------------------------------------------------------------//
	HwProgram::HwProgram(uint _id, HwVertexFormat* _vfIn, HwVertexFormat* _vfOut, HwShader* _vs, HwShader* _fs, HwShader* _gs) :
		m_vfIn(_vfIn),
		m_vfOut(_vfOut),
		m_linked(false),
		m_valid(false),
		m_id(_id),
		m_handle(0)
	{
		m_handle = glCreateProgram();
		if (_vs)
		{
			m_stages[ST_Vertex].shader = _vs;
			m_stages[ST_Vertex].actualVersion = _vs->m_version;
			glAttachShader(m_handle, _vs->m_handle);
		}
		if (_fs)
		{
			m_stages[ST_Fragment].shader = _fs;
			m_stages[ST_Fragment].actualVersion = _fs->m_version;
			glAttachShader(m_handle, _fs->m_handle);
		}
		if (_gs)
		{
			m_stages[ST_Geometry].shader = _gs;
			m_stages[ST_Geometry].actualVersion = _gs->m_version;
			glAttachShader(m_handle, _gs->m_handle);
		}
	}
	//----------------------------------------------------------------------------//
	HwProgram::~HwProgram(void)
	{
		glDeleteProgram(m_handle);
		s_instances.erase(m_id);
	}
	//----------------------------------------------------------------------------//
	bool HwProgram::Link(void)
	{
		if (m_linked && !IsOutOfDate())
			return m_valid;

		m_linked = true;
		m_log.Clear();

		// compile and validate shaders
		bool _incomplete = false;
		for (uint i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			HwShader* _s = m_stages[i].shader;
			if (_s)
			{
				if (_s->m_state == SS_Compiling)
				{
					int _status;
					glGetShaderiv(_s->m_handle, GL_COMPILE_STATUS, &_status);
					_s->m_state = _status ? SS_Compiled : SS_Invalid;
				}

				if (_s->m_state == SS_Invalid)
				{
					m_log += String(GLShaderNames[_s->m_type]) + " shader was compiled with errors:\n";
					// read shader log here
					_incomplete = true;
					m_valid = false;
				}
				else if (_s->m_state == SS_Initial)
				{
					m_log += String(GLShaderNames[_s->m_type]) + " shader not was compiled\n";
					_incomplete = true;
					m_valid = false;
				}
			}
		}

		if (_incomplete)
			return false;

		// bind vertex input
		if (m_vfIn)
		{
			for (uint i = 0; i < m_vfIn->GetNumElements(); ++i)
			{
				const VertexElement& _e = m_vfIn->GetElement(i);
				glBindAttribLocation(m_handle, _e.index, _e.name);
			}
		}

		// bind vertex output
		if (m_vfOut)
		{
			//TODO: get max transfrom feedback varyings
			static const String _prefix = "R2VB.";
			String _nameStorage[MAX_VERTEX_ELEMENTS];
			const char* _names[MAX_VERTEX_ELEMENTS];
			for (uint i = 0; i < m_vfOut->GetNumElements(); ++i)
			{
				_nameStorage[i] = _prefix + m_vfOut->GetElement(i).name;
				_names[i] = _nameStorage[i];
			}
			glTransformFeedbackVaryings(m_handle, m_vfOut->GetNumElements(), _names, GL_INTERLEAVED_ATTRIBS);
		}

		// compile
		int _status = 0;
		glLinkProgram(m_handle);
		glGetProgramiv(m_handle, GL_LINK_STATUS, &_status);
		m_valid = _status != 0;
		if (!m_valid)
		{
			// read program log here
			return false;
		}

		// reflection
		{
			SArray<Param> _oldParams = m_params;
			m_params.Clear();

			SArray<char> _name;
			int _count = 0, _length = 0, _size;
			uint _type, _numTextures = 0, _numBlocks = 0;

			// uniforms
			glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &_count);
			glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &_length);
			_name.Resize(_length + 1, '\0');

			for (int i = 0; i < _count; ++i)
			{
				glGetActiveUniform(m_handle, i, _name.Size(), &_length, &_size, &_type, &_name[0]);
				_name[_length] = 0;

				Param _p;

				switch (_type)
				{
				case GL_INT:
					_p.type = SPT_Int;
					_p.elementSize = sizeof(int);
					break;
				case GL_INT_VEC2:
					_p.type = SPT_Vec2i;
					_p.elementSize = sizeof(int) * 2;
					break;
				case GL_INT_VEC3:
					_p.type = SPT_Vec3i;
					_p.elementSize = sizeof(int) * 3;
					break;
				case GL_INT_VEC4:
					_p.type = SPT_Vec4i;
					_p.elementSize = sizeof(int) * 4;
					break;
				case GL_FLOAT:
					_p.type = SPT_Float;
					_p.elementSize = sizeof(float);
					break;
				case GL_FLOAT_VEC2:
					_p.type = SPT_Vec2;
					_p.elementSize = sizeof(float) * 2;
					break;
				case GL_FLOAT_VEC3:
					_p.type = SPT_Vec3;
					_p.elementSize = sizeof(float) * 3;
					break;
				case GL_FLOAT_VEC4:
					_p.type = SPT_Vec4;
					_p.elementSize = sizeof(float) * 4;
					break;
				case GL_FLOAT_MAT4:
					_p.type = SPT_Mat44;
					_p.elementSize = sizeof(float) * 16;
					break;
				case GL_FLOAT_MAT3x4:
					_p.type = SPT_Mat34;
					_p.elementSize = sizeof(float) * 12;
					break;

				case GL_SAMPLER_2D:
				case GL_SAMPLER_2D_SHADOW:
				case GL_INT_SAMPLER_2D:
				case GL_UNSIGNED_INT_SAMPLER_2D:
					_p.type = SPT_Tex2D;
					break;

				case GL_SAMPLER_2D_ARRAY:
				case GL_SAMPLER_2D_ARRAY_SHADOW:
				case GL_INT_SAMPLER_2D_ARRAY:
				case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
					_p.type = SPT_Tex2DArray;
					break;

				case GL_SAMPLER_2D_MULTISAMPLE:
				case GL_INT_SAMPLER_2D_MULTISAMPLE:
				case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
					_p.type = SPT_Tex2DMultisample;
					break;

				case GL_SAMPLER_3D:
				case GL_INT_SAMPLER_3D:
				case GL_UNSIGNED_INT_SAMPLER_3D:
					_p.type = SPT_Tex3D;
					break;

				case GL_SAMPLER_CUBE:
				case GL_SAMPLER_CUBE_SHADOW:
				case GL_INT_SAMPLER_CUBE:
				case GL_UNSIGNED_INT_SAMPLER_CUBE:
					_p.type = SPT_TexCube;
					break;

				case GL_SAMPLER_BUFFER:
				case GL_INT_SAMPLER_BUFFER:
				case GL_UNSIGNED_INT_SAMPLER_BUFFER:
					_p.type = SPT_TexBuffer;
					break;
				}

				if (_p.type == SPT_Unknown)
				{
					m_log += String::Format("Unknown parameter \"%s\" (type = 0x%04x)\n", &_name[0], _type);
				}
				else
				{
					if (_p.type >= SPT_Tex2D)
					{
						_p.name = &_name[0];
						_p.id = glGetUniformLocation(m_handle, &_name[0]);
						_p.count = 1;
						_p.elementSize = 0;
						_p.slot = _numTextures++;
						glProgramUniform1iEXT(m_handle, _p.id, _p.slot);
					}
					else
					{
						char* _array = strchr(&_name[0], '['); // exclude index
						if (_array)
							*_array = 0;
						_p.id = glGetUniformLocation(m_handle, &_name[0]);
						_p.name = &_name[0];
						_p.count = _size;
					}

					ASSERT(m_paramNames.find(_p.name) == m_paramNames.end());
					m_paramNames[_p.name] = m_params.Size();
					m_params.PushBack(_p);
				}
			}

			// uniform blocks 
			glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCKS, &_count);
			glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &_length);
			_name.Resize(_length + 1);
			for (int i = 0; i < _count; ++i)
			{
				glGetActiveUniformBlockName(m_handle, i, _name.Size(), &_length, &_name[0]);
				_name[_length] = 0;
				char* _array = strchr(&_name[0], '[');
				if (_array)
					*_array = 0;

				Param _p;
				_p.name = &_name[0];
				_p.type = SPT_Block;
				_p.count = 1; // ?
				_p.slot = _numBlocks++;
				_p.id = glGetUniformBlockIndex(m_handle, &_name[0]);
				glGetActiveUniformBlockiv(m_handle, _p.id, GL_UNIFORM_BLOCK_DATA_SIZE, &_size);
				_p.elementSize = _size;
				//TODO: signature;

				glUniformBlockBinding(m_handle, _p.id, _p.slot);

				ASSERT(m_paramNames.find(_p.name) == m_paramNames.end());
				m_paramNames[_p.name] = m_params.Size();
				m_params.PushBack(_p);
			}

			// bind params
			if (_oldParams.NonEmpty())
			{
				// ...
			}
		}

		m_valid = true;
		return true;
	}
	//----------------------------------------------------------------------------//
	bool HwProgram::IsOutOfDate(void)
	{
		for (uint i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			if (m_stages[i].shader && m_stages[i].actualVersion != m_stages[i].shader->m_version)
				return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	int HwProgram::GetParamIndex(const String& _name)
	{
		auto _it = m_paramNames.find(_name);
		return _it != m_paramNames.end() ? _it->second : -1;
	}
	//----------------------------------------------------------------------------//
	const HwProgram::Param& HwProgram::GetParam(uint _index)
	{
		static const Param _default;
		return _index < m_params.Size() ? m_params[_index] : _default;
	}
	//----------------------------------------------------------------------------//
	void HwProgram::SetParam(uint _index, ShaderParamType _type, const void* _value, uint _count)
	{
		if (_index < m_params.Size())
		{
			Param& _p = m_params[_index];
			if (_p.type < SPT_Block && _p.type == _type)
			{
				switch (_type)
				{
				case SPT_Int:
					glProgramUniform1ivEXT(m_handle, _p.id, _count, (const int*)_value);
					break;
				case SPT_Float:
					glProgramUniform1fvEXT(m_handle, _p.id, _count, (const float*)_value);
					break;
				case SPT_Vec2i:
					glProgramUniform2ivEXT(m_handle, _p.id, _count, (const int*)_value);
					break;
				case SPT_Vec3i:
					glProgramUniform3ivEXT(m_handle, _p.id, _count, (const int*)_value);
					break;
				case SPT_Vec4i:
					glProgramUniform4ivEXT(m_handle, _p.id, _count, (const int*)_value);
					break;
				case SPT_Vec2:
					glProgramUniform2fvEXT(m_handle, _p.id, _count, (const float*)_value);
					break;
				case SPT_Vec3:
					glProgramUniform3fvEXT(m_handle, _p.id, _count, (const float*)_value);
					break;
				case SPT_Vec4:
					glProgramUniform4fvEXT(m_handle, _p.id, _count, (const float*)_value);
					break;
				case SPT_Mat34:
					glProgramUniformMatrix3x4fvEXT(m_handle, _p.id, _count, true, (const float*)_value);
					break;
				case SPT_Mat44:
					glProgramUniformMatrix4fvEXT(m_handle, _p.id, _count, true, (const float*)_value);
					break;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void HwProgram::BindTexture(uint _index, uint _slot)
	{
		if (_index < m_params.Size())
		{
			Param& _p = m_params[_index];
			if (_p.type >= SPT_Tex2D && _p.slot != _slot)
			{
				_p.slot = _slot;
				glProgramUniform1iEXT(m_handle, _p.id, _slot);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void HwProgram::BindBuffer(uint _index, uint _slot)
	{
		if (_index < m_params.Size())
		{
			Param& _p = m_params[_index];
			if (_p.type == SPT_Block && _p.slot != _slot)
			{
				_p.slot = _slot;
				glUniformBlockBinding(m_handle, _p.id, _slot);
			}
		}
	}
	//----------------------------------------------------------------------------//

#endif

	//----------------------------------------------------------------------------//
	// Commands
	//----------------------------------------------------------------------------//

	struct _SetViewportAndClearBuffers // temp
	{
		void Exec(void)
		{
			VERIFY_GL_SCOPE();

			if (gWindow)
			{
				glViewport(0, 0, gWindow->GetWidth(), gWindow->GetHeight());
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	};

	struct Cmd_SwapBuffers
	{
		void Exec(void)
		{
			VERIFY_GL_SCOPE();

			gGLRenderDevice->_SwapBuffers();
			gGLRenderContext->m_endFrame.Signal();
		}
	};

	struct Cmd_BindVertexFormat
	{
		GLVertexFormat* format;

		void Exec(void)
		{
			VERIFY_GL_SCOPE();

			format->_BindFormat();
		}
	};

	struct Cmd_BindVertexBuffer
	{
		GLBuffer* buffer;
		uint stream;
		uint offset;
		uint stride;

		void Exec(void)
		{
			VERIFY_GL_SCOPE();

			glBindVertexBuffer(stream, buffer->_Handle()->glhandle, offset, stride);
			// ...
		}
	};

	//----------------------------------------------------------------------------//
	// GLRenderContext
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	GLRenderContext::GLRenderContext(void)
	{
	}
	//----------------------------------------------------------------------------//
	GLRenderContext::~GLRenderContext(void)
	{
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::BeginFrame(void)
	{
		_SetViewportAndClearBuffers _cmd;
		gGLDrawQueue.Push(&_cmd);
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::EndFrame(void)
	{
		Cmd_SwapBuffers _cmd;
		gGLDrawQueue.Push(&_cmd);
		m_endFrame.Wait();
		m_resources.clear();
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::SetVertexFormat(HardwareVertexFormat* _format)
	{
		TODO("Verify args");

		Cmd_BindVertexFormat _cmd = { static_cast<GLVertexFormat*>(_format) };
		gGLDrawQueue.Push(&_cmd);
	}
	//----------------------------------------------------------------------------//
	void GLRenderContext::SetVertexBuffer(uint _stream, HardwareBuffer* _buffer, uint _offset, uint _stride)
	{
		TODO("Verify args");

		m_resources.push_back(_buffer);

		Cmd_BindVertexBuffer _cmd = { static_cast<GLBuffer*>(_buffer), _stream, _offset, _stride };
		gGLDrawQueue.Push(&_cmd);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// GLRenderDevice
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	GLRenderDevice::GLRenderDevice(void) :
		m_context(new GLContext),
		m_success(false),
		m_runThread(true),
		m_newVsync(false),
		m_vsync(false)
	{
		m_shaderModel = SM_Unknown;
		m_profile = RCP_Core;
		m_debugContext = false;

	}
	//----------------------------------------------------------------------------//
	GLRenderDevice::~GLRenderDevice(void)
	{
		m_runThread = false;
		m_thread.Wait();
		m_context->Destroy();
		delete m_context;

		//TODO: delete RenderContext
	}
	//----------------------------------------------------------------------------//
	bool GLRenderDevice::_Init(ShaderModel _shaderModel, RenderContextProfile _profile, bool _debugContext)
	{
		if (SDL_Init(SDL_INIT_VIDEO))
		{
			LOG_MSG(LL_Error, "Couldn't initialize SDL2 video: %s", SDL_GetError());
			return false;
		}

		if (!m_context->Create(_shaderModel, _profile, _debugContext))
		{
			LOG_MSG(LL_Error, "Couldn't create render context");
			return false;
		}
		SDL_GL_MakeCurrent(nullptr, nullptr);

		m_debugContext = m_context->debugContext;
		m_profile = m_context->profile;
		m_shaderModel = m_context->maxShaderModel;

		m_thread = Thread(this, &GLRenderDevice::_DriverThread);
		m_initialized.Wait();
		if (!m_success)
		{
			LOG_MSG(LL_Error, "Couldn't initialize driver thread");
			return false;
		}

		new GLRenderContext; // create immediate context

		return true;
	}
	//----------------------------------------------------------------------------//
	SDL_Window* GLRenderDevice::_GetSDLWindow(void)
	{
		return m_context->window;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderDevice::_InitDriver(void)
	{
		if (!m_context->MakeCurrent())
		{
			LOG_MSG(LL_Error, "Couldn't capture OpenGL context in render thread");
			return false;
		}

		if (!ogl_LoadFunctions())
		{
			LOG_MSG(LL_Error, "Couldn't load OpenGL functions");
			return false;
		}

		LOG_MSG(LL_Info, "Renderer: OpenGL");
		LOG_MSG(LL_Info, "Backward compatibility context: %s", m_context->profile == RCP_Compatible ? "Yes" : "No");
		LOG_MSG(LL_Info, "GL_VERSION: %s", glGetString(GL_VERSION));
		LOG_MSG(LL_Info, "GL_RENDERER: %s", glGetString(GL_RENDERER));
		LOG_MSG(LL_Info, "GL_VENDOR: %s", glGetString(GL_VENDOR));
		LOG_MSG(LL_Info, "GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		gGLGpuInfo->_Init();

		LOG_MSG(LL_Info, "Requested shader model: %s", m_context->minShaderModel == SM4 ? "SM4" : "SM5");

		const uint _numShaderModels = 2;
		const ShaderModel _shaderModels[_numShaderModels] = { SM5, SM4 };
		m_success = false;
		for (uint i = 0; i < _numShaderModels && !m_success; ++i)
		{
			ShaderModel _sm = _shaderModels[i];
			if (_sm >= m_context->minShaderModel && _sm <= m_context->maxShaderModel)
			{
				switch (_sm)
				{
				case SM4:
					m_success = _Init33();
					break;
				case SM5:
					m_success = _Init43();
					break;
				}
			}
		}

		if (!m_success)
		{
			LOG_MSG(LL_Error, "Minimal shader model %d.%d is not supported", m_context->minShaderModel / 10, m_context->minShaderModel % 10);
			m_initialized.Signal();
			return false;
		}

		LOG_MSG(LL_Info, "Shader model: %s", m_shaderModel == SM4 ? "SM4" : "SM5");
		LOG_MSG(LL_Info, "Debug context: %s", m_debugContext ? "Yes" : "No");

		gGLDebugHelper->_Init(m_debugContext);
		if (LoadGremedyExtensions())
		{
			LOG_MSG(LL_Info, "gDEBugger is presented");
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderDevice::_Init33(void)
	{
		if (!SDL_GL_ExtensionSupported("GL_EXT_direct_state_access"))
		{
			LOG_MSG(LL_Warning, "Shader model 4.1 is not supported: No GL_EXT_direct_state_access extension");
			return false;
		}
		if (!ogl_IsVersionGEQ(4, 3) && !SDL_GL_ExtensionSupported("GL_ARB_vertex_attrib_binding"))
		{
			LOG_MSG(LL_Warning, "Shader model 4.1 is not supported: No GL_ARB_vertex_attrib_binding extension");
			return false;
		}

		m_shaderModel = SM4;
		return true;
	}
	//----------------------------------------------------------------------------//
	bool GLRenderDevice::_Init43(void)
	{
		if (!SDL_GL_ExtensionSupported("GL_EXT_direct_state_access"))
		{
			LOG_MSG(LL_Warning, "Shader model 5.0 is not supported: No GL_EXT_direct_state_access extension");
			return false;
		}

		m_shaderModel = SM5;
		return true;
	}
	//----------------------------------------------------------------------------//
	void GLRenderDevice::_DriverThread(void)
	{
		Thread::SetName(Thread::GetCurrentId(), "Render driver");

		isDriverThread = true;

		m_newVsync = false; // temp

		if (_InitDriver())
		{
			m_vsync = SDL_GL_GetSwapInterval() != 0; // get default vsync

			m_initialized.Signal();

			GLCommandFunc _func;
			uint8_t* _data = new uint8[GLCommandQueue::SIZE]; // ...
			while (m_runThread || gGLResourceQueue.GetSize() || gGLDrawQueue.GetSize())
			{
				while (gGLResourceQueue.Pop(_func, _data, false))
					_func(_data);

				if (gGLDrawQueue.Pop(_func, _data, false))
					_func(_data);
				else
					GLCommandQueue::Wait(20);
			}

			/*
			m_newVsync = true; // temp

			int _mfreq = 60; //TODO: read display mode
			double _st, _et, _ft = 0;
			while (m_runThread)
			{
				if (m_vsync)
				{
					double _t = 1000.0 / (_mfreq + 1);
					double _pause = _t - _ft;
					if (_pause > 1)
					{
						Thread::Pause((uint)_pause); // emulate VSync
						//printf("sleep %.1f ms\n", _pause);
					}
				}

				_st = Timer::Ms();

				_SwapBuffers();

				_et = Timer::Ms();
				_ft = _et - _st;

				//TODO: fps counter
			}
			*/
		}
		else
		{
			LOG_MSG(LL_Error, "Couldn't initalize OpenGL driver");
		}

		SDL_GL_MakeCurrent(nullptr, nullptr);
		m_initialized.Signal();
	}
	//----------------------------------------------------------------------------//
	void GLRenderDevice::_SwapBuffers(void)
	{
		VERIFY_GL_SCOPE();

		if (m_newVsync != m_vsync)
		{
			m_vsync = m_newVsync;
			SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);
			LOG_MSG(LL_Event, "Set VSync: %d", m_vsync);
		}

		DebugOutputLevel _dl = gGLDebugHelper->SetLevel(DOL_Disabled); // disable debug output
		SDL_GL_SwapWindow(m_context->window);
		if (gGLDebugHelper->GetType() == GLDebugHelper::OT_CheckErrors) // reset errors
			while (glGetError());
		gGLDebugHelper->SetLevel(_dl);

		GLDebugBreak();	// frame terminator
	}
	//----------------------------------------------------------------------------//
	/*HwBufferPtr RenderDevice::CreateBuffer(HwBufferUsage _usage, uint _size, const void* _data)
	{
		return new HwBuffer(_usage, _size, _data);
	}
	//----------------------------------------------------------------------------//
	HwVertexFormat* RenderDevice::CreateVertexFormat(VertexElement* _elements, uint _numElements)
	{
		//TODO: verify format

		uint _hash = Crc32(_elements, sizeof(VertexElement) * _numElements);
		auto _exists = m_vertexFormats.find(_hash);
		if (_exists != m_vertexFormats.end())
			return _exists->second;
		HwVertexFormat* _fmt = new HwVertexFormat(_elements, _numElements);
		m_vertexFormats[_hash] = _fmt;
		return _fmt;
	}
	//----------------------------------------------------------------------------//
	HwVertexArrayPtr RenderDevice::CreateVertexArray(HwVertexFormat* _format)
	{
		return new HwVertexArray(_format);
	}
	//----------------------------------------------------------------------------//
	HwTexturePtr RenderDevice::CreateTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods)
	{
		VERIFY_GL_SCOPE();

		if (_type == TT_Unknown) // invalid texture type
			return nullptr;
		if (_format == PF_Unknown) // invalid pixel format
			return nullptr;
		if (_type == TT_Buffer && (GLPixelFormats[_format].flags & (PFF_Compressed | PFF_DepthStencil))) // invalid pixel format for texture buffer
			return nullptr;
		if (_type == TT_2DMultisample && (GLPixelFormats[_format].flags & PFF_Compressed)) // invalid pixel format for multisample texture
			return nullptr;

		return new HwTexture(_type, _usage, _format, _maxLods);
	}
	//----------------------------------------------------------------------------//
	*/

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
