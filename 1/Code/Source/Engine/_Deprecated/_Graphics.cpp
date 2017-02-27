#include "Graphics.hpp"
#include "Device.hpp"
#include <GL/glCore33.h>
COMPILER_MESSAGE("linker", "opengl32.lib");
#pragma comment(lib, "opengl32.lib")
COMPILER_MESSAGE("linker", "glLoad.lib");
#pragma comment(lib, "glLoad.lib")

namespace ge
{
	//----------------------------------------------------------------------------//
	// Internal
	//----------------------------------------------------------------------------//

	const uint GLIndexType = GL_UNSIGNED_SHORT;

	//----------------------------------------------------------------------------//
	// Vertex
	//----------------------------------------------------------------------------//

	struct GLVertexAttrib
	{
		const char* name;
		uint type;
		uint size;
		bool normalized;
		bool integer;
		void* offset;
	}
	const GLVertexAttribs[] =
	{
		{ "vaPos", GL_FLOAT, 3, false, false, (void*)offsetof(Vertex, pos) }, // float[3] pos

		{ "vaTexCoord", GL_FLOAT, 2, false, false, (void*)offsetof(Vertex, mesh.texCoord) }, // float[2] mesh.texcoord, particle.texcoord
		{ "vaTexCoord2", GL_FLOAT, 2, false, false, (void*)offsetof(Vertex, mesh.texCoord2) }, // float[2] mesh.texcoord2, particle.texcoord2
		{ "vaColor", GL_UNSIGNED_BYTE, 4, true, false, (void*)offsetof(Vertex, mesh.color) }, // uint8[4] mesh.color, uint8[4] particle.color, uint8[4] sprite.color
		{ "vaNormal", GL_UNSIGNED_BYTE, 4, true, false, (void*)offsetof(Vertex, mesh.normal) }, // uint8[4] mesh.normal
		{ "vaTangent", GL_UNSIGNED_BYTE, 4, true, false, (void*)offsetof(Vertex, mesh.tangent) }, // uint8[4] mesh.tangent
		{ "vaIndices", GL_UNSIGNED_BYTE, 4, false, true, (void*)offsetof(Vertex, mesh.indices) }, // uint8[4] mesh.indices
		{ "vaWeights", GL_UNSIGNED_BYTE, 4, true, false, (void*)offsetof(Vertex, mesh.weights) }, // uint8[4] mesh.weights

		{ "vaVelocity", GL_FLOAT, 3, false, false, (void*)offsetof(Vertex, particle.velocity) }, // float[3] particle.velocity
		{ "vaAge", GL_FLOAT, 1, false, false, (void*)offsetof(Vertex, particle.age) }, // float particle.age
		{ "vaParticleSize", GL_FLOAT, 1, false, false, (void*)offsetof(Vertex, particle.size) }, // float particle.size
		{ "vaRotation", GL_FLOAT, 1, false, false, (void*)offsetof(Vertex, particle.rotation) }, // float[2] particle.rotation, sprite.rotation
		//{ "vaPrevPos", GL_FLOAT, 3, false, false, (void*)offsetof(Vertex, pos) }, // float[3] particle.prevPos

		{ "vaSpriteSize", GL_FLOAT, 2, false, false, (void*)offsetof(Vertex, sprite.size) }, // float[2] sprite.size
		{ "vaAxis", GL_INT, 1, false, true, (void*)offsetof(Vertex, sprite.axis) }, // float[2] sprite.axis
	};
	enum : uint { MAX_VERTEX_ATTRIBS = 14 };

	//----------------------------------------------------------------------------//
	// HardwareBuffer
	//----------------------------------------------------------------------------//

	const uint GLBufferType[] =
	{
		GL_ARRAY_BUFFER, // HBT_VERTEX
		GL_ELEMENT_ARRAY_BUFFER, // HBT_INDEX
		GL_UNIFORM_BUFFER, // HBT_UNIFORM
	};

	const uint GLBufferUsage[] =
	{
		GL_STATIC_DRAW, // HBU_STATIC
		GL_DYNAMIC_DRAW, // HBU_DYNAMIC
	};

	//----------------------------------------------------------------------------//
	HardwareBuffer::HardwareBuffer(HardwareBufferType _type, HardwareBufferUsage _usage) :
		m_type(_type),
		m_usage(_usage),
		m_size(0),
		m_handle(0),
		m_vertexArray(0)
	{
		glGenBuffers(1, &m_handle);
		if (m_type == HBT_VERTEX)
		{
			glGenVertexArrays(1, &m_vertexArray);
			glBindVertexArray(m_vertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, m_handle);
			for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
			{
				const GLVertexAttrib& _attrib = GLVertexAttribs[i];
				if (_attrib.integer) glVertexAttribIPointer(i, _attrib.size, _attrib.type, sizeof(Vertex), _attrib.offset);
				else glVertexAttribPointer(i, _attrib.size, _attrib.type, _attrib.normalized, sizeof(Vertex), _attrib.offset);
				glEnableVertexAttribArray(i);
			}
		}
		else glBindBuffer(GLBufferType[m_type], m_handle);
	}
	//----------------------------------------------------------------------------//
	HardwareBuffer::~HardwareBuffer(void)
	{
		glDeleteBuffers(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	void HardwareBuffer::Realloc(uint _size)
	{
		if (m_size != _size)
		{
			glBindBuffer(GLBufferType[m_type], m_handle);
			glBufferData(GLBufferType[m_type], _size, 0, GLBufferUsage[m_usage]);
			m_size = _size;
		}
	}
	//----------------------------------------------------------------------------//
	void HardwareBuffer::Write(uint _offset, uint _size, const void* _data)
	{
		glBindBuffer(GLBufferType[m_type], m_handle);
		glBufferSubData(GLBufferType[m_type], _offset, _size, _data);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// PixelFormat
	//----------------------------------------------------------------------------//

	typedef PixelFormatInfo GLPixelFormat;

	const PixelFormatInfo GLPixelFormats[MAX_PIXEL_FORMATS + 1] =
	{
		{ PF_UNKNOWN, "unknown", 0, 0, 0, GL_NONE, GL_NONE, GL_NONE, },

		{ PF_R8_UNORM, "r8", 8, 1, PFF_COLOR | PFF_UNORM, GL_R8, GL_RED, GL_UNSIGNED_BYTE, },
		{ PF_RG8_UNORM, "rg8", 16, 2, PFF_COLOR | PFF_UNORM, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, },
		{ PF_RGB8_UNORM, "rgb8", 24, 3, PFF_COLOR | PFF_UNORM, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, },
		{ PF_RGBA8_UNORM, "rgba8", 32, 4, PFF_COLOR | PFF_UNORM | PFF_WITH_ALPHA, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, },

		{ PF_R8_UINT, "r8ui", 8, 1, PFF_COLOR | PFF_UINT | PFF_UNSIGNED_INTEGER, GL_R8UI, GL_RED, GL_UNSIGNED_BYTE, },
		{ PF_RG8_UINT, "rg8ui", 16, 2, PFF_COLOR | PFF_UINT | PFF_UNSIGNED_INTEGER, GL_RG8UI, GL_RG, GL_UNSIGNED_BYTE, },
		{ PF_RGB8_UINT, "rgb8ui", 24, 3, PFF_COLOR | PFF_UINT | PFF_UNSIGNED_INTEGER, GL_RGB8UI, GL_RGB, GL_UNSIGNED_BYTE, },
		{ PF_RGBA8_UINT, "rgba8ui", 32, 4, PFF_COLOR | PFF_UINT | PFF_UNSIGNED_INTEGER | PFF_WITH_ALPHA, GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_BYTE, },

		{ PF_R16_FLOAT, "r16f", 16, 1, PFF_COLOR | PFF_HALF, GL_R16F, GL_RED, GL_HALF_FLOAT, },
		{ PF_RG16_FLOAT, "rg16f", 32, 2, PFF_COLOR | PFF_HALF, GL_RG16F, GL_RG, GL_HALF_FLOAT, },
		{ PF_RGB16_FLOAT, "rgb16f", 48, 3, PFF_COLOR | PFF_HALF, GL_RGB16F, GL_RGB, GL_HALF_FLOAT, },
		{ PF_RGBA16_FLOAT, "rgba16f", 64, 4, PFF_COLOR | PFF_HALF | PFF_WITH_ALPHA, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, },

		{ PF_R32_FLOAT, "r32f", 32, 1, PFF_COLOR | PFF_FLOAT, GL_R32F, GL_RED, GL_FLOAT, },
		{ PF_RG32_FLOAT, "rg32f", 64, 2, PFF_COLOR | PFF_FLOAT, GL_RG32F, GL_RG, GL_FLOAT, },
		{ PF_RGB32_FLOAT, "rgb32f", 96, 3, PFF_COLOR | PFF_FLOAT, GL_RGB32F, GL_RGB, GL_FLOAT, },
		{ PF_RGBA32_FLOAT, "rgba32f", 128, 4, PFF_COLOR | PFF_FLOAT | PFF_WITH_ALPHA, GL_RGBA32F, GL_RGBA, GL_FLOAT, },

		{ PF_RG11B10_FLOAT, "rg10b11f", 32, 3, PFF_COLOR | PFF_FLOAT | PFF_PACKED, GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, },

		{ PF_D24S8, "d24s8", 32, 2, PFF_DEPTH_STENCIL | PFF_PACKED, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, },

		{ PF_DXT1, "dxt1", 4, 3, PFF_COLOR | PFF_COMPRESSED, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_NONE, GL_NONE, },
		{ PF_DXT1A, "dxt1a", 4, 4, PFF_COLOR | PFF_COMPRESSED | PFF_WITH_ALPHA, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_NONE, GL_NONE, },
		{ PF_DXT3, "dxt3", 8, 4, PFF_COLOR | PFF_COMPRESSED | PFF_WITH_ALPHA, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_NONE, GL_NONE, },
		{ PF_DXT5, "dxt5", 8, 4, PFF_COLOR | PFF_COMPRESSED | PFF_WITH_ALPHA, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_NONE, GL_NONE, },
		{ PF_RGTC1, "rgtc1", 4, 1, PFF_COLOR | PFF_COMPRESSED, GL_COMPRESSED_RED_RGTC1, GL_NONE, GL_NONE, },
		{ PF_RGTC2, "rgtc2", 8, 2, PFF_COLOR | PFF_COMPRESSED, GL_COMPRESSED_RG_RGTC2, GL_NONE, GL_NONE, },

		{ PF_UNKNOWN, "unknown", 0, 0, 0, GL_NONE, GL_NONE, GL_NONE, },
	};

	//----------------------------------------------------------------------------//
	const PixelFormatInfo& GetPixelFormatInfo(PixelFormat _format)
	{
		return GLPixelFormats[_format];
	}
	//----------------------------------------------------------------------------//
	PixelFormat GetCompressedPixelFormat(PixelFormat _format)
	{
		GLPixelFormat _info = GLPixelFormats[_format];
		if (_info.flags & (PFF_COMPRESSED | PFF_INTEGER | PFF_DEPTH_STENCIL)) return _format;
		switch (_info.channels)
		{
		case 1:	return PF_RGTC1;
		case 2:	return PF_RGTC2;
		case 3:	return PF_DXT1;
		case 4:	return PF_DXT5;
		}
		return _format;
	}
	//----------------------------------------------------------------------------//
	PixelFormat GetUncompressedPixelFormat(PixelFormat _format)
	{
		switch (_format)
		{
		case PF_DXT1: return PF_RGB8_UNORM;
		case PF_DXT1A:
		case PF_DXT3:
		case PF_DXT5: return PF_RGBA8_UNORM;
		case PF_RGTC1: return PF_R8_UNORM;
		case PF_RGTC2: return PF_RG8_UNORM;
		}
		return _format;
	}
	//----------------------------------------------------------------------------//
	/*PixelFormat GetClosestPixelFormatForTextureBuffer(PixelFormat _format)
	{
		switch (_format)
		{
		case PF_RGB8_UNORM: return PF_RGBA8_UNORM;
		case PF_RGB8_UINT: return PF_RGBA8_UINT;
		case PF_RGB32_FLOAT: return PF_RGBA32_FLOAT;
		case PF_RGB16_FLOAT:
		case PF_RG11B10_FLOAT: return PF_RGBA16_FLOAT;
		case PF_D24S8: return PF_UNKNOWN;
		case PF_DXT1:
		case PF_DXT1A:
		case PF_DXT3:
		case PF_DXT5: return PF_RGBA8_UNORM;
		case PF_RGTC1: return PF_R8_UNORM;
		case PF_RGTC2: return PF_RG8_UNORM;
		}
		return _format;
	}*/
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Texture
	//----------------------------------------------------------------------------//

	const uint GLTextureType[] =
	{
		GL_TEXTURE_2D, // TT_2D
		GL_TEXTURE_2D_ARRAY, // TT_2D_ARRAY
		GL_TEXTURE_2D_MULTISAMPLE, // TT_2D_MULTISAMPLE
		GL_TEXTURE_3D, // TT_3D
		GL_TEXTURE_CUBE_MAP, // TT_CUBE
	};

	const uint GLTextureFilter[] =
	{
		GL_NEAREST, // TF_NEAREST
		GL_LINEAR, // TF_LINEAR
		GL_NEAREST_MIPMAP_NEAREST, // TF_BILINEAR
		GL_LINEAR_MIPMAP_LINEAR, // TF_TRILINEAR
	};

	const uint GLTextureWrap[] =
	{
		GL_REPEAT, // TW_REPEAT
		GL_CLAMP_TO_EDGE, // TW_CLAMP
	};

	uint Texture::s_memorySize = 0;

	//----------------------------------------------------------------------------//
	Texture::Texture(const String& _name, TextureType _type, uint _flags) :
		Resource(_name),
		m_type(_type),
		m_flags(_flags),
		m_format(PF_UNKNOWN),
		m_size(Vec3ui::ZERO),
		m_baseLod(0),
		m_lodCount(1),
		m_availableLodCount(1),
		m_memorySize(0),
		m_handle(0)
	{
		glGenTextures(1, &m_handle);
		glBindTexture(GLTextureType[m_type], m_handle);
		if (m_type == TT_2D_MULTISAMPLE) m_flags &= ~(TF_COMPRESSION | TF_MIPMAPS);
	}
	//----------------------------------------------------------------------------//
	Texture::~Texture(void)
	{
		glDeleteTextures(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	void Texture::Realloc(PixelFormat _format, uint _width, uint _height, uint _depth)
	{
		if (m_format == _format && m_size.x == _width && m_size.y == _height && m_size.z == _depth) return; // no changes
		if (_format == PF_UNKNOWN) return; // invalid param
		//if ((m_type == TT_2D_MULTISAMPLE || m_usage != TU_IMAGE) && (GLPixelFormats[_format].flags & PFF_COMPRESSED)) return; // invalid param
		//if (m_usage == TU_BUFFER)
		//{
		//	const GLPixelFormat& _info = GLPixelFormats[_format];
		//	if (_info.channels == 3 || (_info.flags & (PFF_COMPRESSED | PFF_DEPTH_STENCIL))) return; // invalid param
		//}
		//if ((m_type == TT_2D_MULTISAMPLE || m_type == TT_3D) && (GLPixelFormats[_format].flags & PFF_DEPTH_STENCIL)) return; // invalid param

		TODO("Запоминать желаемое количество уровней детализации для целей рендеринга");
		TODO("Корректная обработка текстур с нулевым размером");

		//_maxLods = GRenderSystem->GetMaxTextureLods(m_type);
		bool _canBeCompressed = (_width % 4) == 0 && (_height % 4) == 0 && m_type != TT_2D_MULTISAMPLE;// && m_usage == TU_IMAGE;
		if (!_canBeCompressed) _format = GetUncompressedPixelFormat(_format);
		const GLPixelFormat& _srcFormat = GLPixelFormats[GetUncompressedPixelFormat(_format)];
		if ((m_flags & TF_COMPRESSION) && _canBeCompressed) _format = GetCompressedPixelFormat(_format);
		const GLPixelFormat& _dstFormat = GLPixelFormats[_format];


		uint _availableLodCount = Log2i(Max(_width, _height, 1u)) + 1;
		if (m_type == TT_2D_MULTISAMPLE) _availableLodCount = 1;
		uint _lodCount = (m_flags & TF_MIPMAPS) ? _availableLodCount : 1;

		// memory statistics
		uint _minBlockWidth = (_dstFormat.flags & PFF_COMPRESSED) ? 4 : 1;
		//uint _minBlockSize = _minBlockWidth * _minBlockWidth;
		uint _memorySize = ((_availableLodCount - _lodCount) * _minBlockWidth) >> 3; // empty lods


		glBindTexture(GLTextureType[m_type], m_handle);

		if (m_type == TT_2D)
		{
			_depth = 1;
			for (uint i = _availableLodCount; i < m_availableLodCount; ++i)
			{
				// удалить ненужные уровни детализации
				glTexImage2D(GL_TEXTURE_2D, i, _dstFormat.internalFormat, 0, 0, 0, _srcFormat.format, _srcFormat.type, nullptr);
			}
			for (uint i = _lodCount; i < _availableLodCount; ++i)
			{
				// некоторые драйверы не позволяют присоединять к fbo текстуры с одним уровнем детализации
				// AMD Catalyst (13.3) вообще вылетает с ошибкой нарушения доступа при отсутствии уровней дализации у текстуры присоединенной к fbo
				// поэтому неиспользуемые уровни детализации создаются с размером 1x1
				glTexImage2D(GL_TEXTURE_2D, i, _dstFormat.internalFormat, 1, 1, 0, _srcFormat.format, _srcFormat.type, nullptr);
			}
			for (uint i = 0, _w = _width, _h = _height; i < _lodCount; ++i)
			{
				glTexImage2D(GL_TEXTURE_2D, i, _dstFormat.internalFormat, _w, _h, 0, _srcFormat.format, _srcFormat.type, nullptr);
				_memorySize += (_dstFormat.bits * Max(_minBlockWidth, _w) * Max(_minBlockWidth, _h)) >> 3;
				if (_w > 1) _w >>= 1;
				if (_h > 1) _h >>= 1;
			}
		}
		else if (m_type == TT_2D_ARRAY)
		{
			for (uint i = _availableLodCount; i < m_availableLodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_2D_ARRAY, i, _dstFormat.internalFormat, 0, 0, 0, 0, _srcFormat.format, _srcFormat.type, nullptr);
			}
			for (uint i = _lodCount; i < _availableLodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_2D_ARRAY, i, _dstFormat.internalFormat, 1, 1, 1, 0, _srcFormat.format, _srcFormat.type, nullptr); // z ??? filtering
			}
			for (uint i = 0, _w = _width, _h = _height; i < _lodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_2D_ARRAY, i, _dstFormat.internalFormat, _w, _h, _depth, 0, _srcFormat.format, _srcFormat.type, nullptr);
				_memorySize += (_dstFormat.bits * Max(_minBlockWidth, _w) * Max(_minBlockWidth, _h) * _depth) >> 3;
				if (_w > 1) _w >>= 1;
				if (_h > 1) _h >>= 1;
			}
		}
		else if (m_type == TT_2D_MULTISAMPLE)
		{
			int _query = 0, _max = 0;
			if (_dstFormat.flags & PFF_INTEGER) _query = GL_MAX_INTEGER_SAMPLES;
			else if (_dstFormat.flags & PFF_COLOR) _query = GL_MAX_COLOR_TEXTURE_SAMPLES;
			else if (_dstFormat.flags & PFF_DEPTH_STENCIL) _query = GL_MAX_DEPTH_TEXTURE_SAMPLES;
			else return; // internal error;
			glGetIntegerv(_query, &_max);

			_depth = Clamp<uint>(_depth, 1, _max);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _depth, _dstFormat.internalFormat, _width, _height, false);
			_memorySize *= _depth;
		}
		else if (m_type == TT_3D)
		{
			for (uint i = _availableLodCount; i < m_availableLodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_3D, i, _dstFormat.internalFormat, 0, 0, 0, 0, _srcFormat.format, _srcFormat.type, nullptr);
			}
			for (uint i = _lodCount; i < _availableLodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_3D, i, _dstFormat.internalFormat, 1, 1, 1, 0, _srcFormat.format, _srcFormat.type, nullptr);	// z ??? really needed?
			}
			for (uint i = 0, _w = _width, _h = _height, _d = _depth; i < _lodCount; ++i)
			{
				glTexImage3D(GL_TEXTURE_3D, i, _dstFormat.internalFormat, _w, _h, _d, 0, _srcFormat.format, _srcFormat.type, nullptr);
				_memorySize += (_dstFormat.bits * Max(_minBlockWidth, _w) * Max(_minBlockWidth, _h) * _d) >> 3;
				if (_w > 1) _w >>= 1;
				if (_h > 1) _h >>= 1;
				if (_d > 1) _d >>= 1;
			}
		}
		else if (m_type == TT_CUBE)
		{
			_depth = 6;
			for (uint f = 0; f < 6; ++f)
			{
				uint _target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X + f;
				for (uint i = _availableLodCount; i < m_availableLodCount; ++i)
				{
					glTexImage2D(_target, i, _dstFormat.internalFormat, 0, 0, 0, _srcFormat.format, _srcFormat.type, nullptr);
				}
				for (uint i = _lodCount; i < _availableLodCount; ++i)
				{
					glTexImage2D(_target, i, _dstFormat.internalFormat, 1, 1, 0, _srcFormat.format, _srcFormat.type, nullptr);
				}
				for (uint i = 0, _w = _width, _h = _height; i < _lodCount; ++i)
				{
					glTexImage2D(_target, i, _dstFormat.internalFormat, _w, _h, 0, _srcFormat.format, _srcFormat.type, nullptr);
					_memorySize += (_dstFormat.bits * Max(_minBlockWidth, _w) * Max(_minBlockWidth, _h)) >> 3;
					if (_w > 1) _w >>= 1;
					if (_h > 1) _h >>= 1;
				}
			}
		}

		m_format = _format;
		m_baseLod = 0;
		m_lodCount = _lodCount;
		m_availableLodCount = _availableLodCount;
		m_size.Set(_width, _height, _depth);
		glTexParameteri(GLTextureType[m_type], GL_TEXTURE_BASE_LEVEL, m_baseLod);
		glTexParameteri(GLTextureType[m_type], GL_TEXTURE_MAX_LEVEL, m_lodCount-1);

		// memory statistics
		s_memorySize -= m_memorySize;
		m_memorySize = _memorySize;
		s_memorySize += m_memorySize;

		LOG_DEBUG("Realloc texture \"%s\" : %dx%dx%d %s, %d bpp, %d lods (%d unused), memory %.2f kb (%.2f mb total)", m_name.c_str(), _width, _height, _depth, _dstFormat.name, _dstFormat.bits, m_lodCount, m_availableLodCount - m_lodCount, m_memorySize / 1024.f, s_memorySize / (1024.f * 1024.f));
	}
	//----------------------------------------------------------------------------//
	void Texture::Write(uint _lod, uint _zOffset, uint _width, uint _height, PixelFormat _format, const void* _data)
	{
		const GLPixelFormat& _dstFormat = GLPixelFormats[m_format];
		const GLPixelFormat& _srcFormat = GLPixelFormats[_format];

		if (m_format == PF_UNKNOWN || (_dstFormat.flags & PFF_DEPTH_STENCIL) || m_type == TT_2D_MULTISAMPLE) return; // invalid op
		if ((_srcFormat.flags & PFF_INTEGER) != (_dstFormat.flags & PFF_INTEGER)) return; // invalid op
		if (_format == PF_UNKNOWN || _data == nullptr || _lod >= m_lodCount || _zOffset >= m_size.z) return; // invalid param

		glBindTexture(GLTextureType[m_type], m_handle);

		///\todo set alignment & slicing
		uint _target = m_type == TT_CUBE ? GL_TEXTURE_CUBE_MAP_NEGATIVE_X + _zOffset : GLTextureType[m_type];
		if (_srcFormat.flags & PFF_COMPRESSED)
		{
			uint _imageSize = (_width * _height * _srcFormat.bits) >> 3;
			if (m_type == TT_2D || m_type == TT_CUBE)
			{
				glCompressedTexSubImage2D(_target, _lod, 0, 0, _width, _height, _srcFormat.internalFormat, _imageSize, _data);
			}
			else if (m_type == TT_2D_ARRAY || m_type == TT_3D)
			{
				glCompressedTexSubImage3D(_target, _lod, 0, 0, _zOffset, _width, _height, 1, _srcFormat.internalFormat, _imageSize, _data);
			}
		}
		else
		{
			if (m_type == TT_2D || m_type == TT_CUBE)
			{
				glTexSubImage2D(_target, _lod, 0, 0, _width, _height, _srcFormat.format, _srcFormat.type, _data);
			}
			else if (m_type == TT_2D_ARRAY || m_type == TT_3D)
			{
				glTexSubImage3D(_target, _lod, 0, 0, _zOffset, _width, _height, 1, _srcFormat.format, _srcFormat.type, _data);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void Texture::GenerateLods(void)
	{
		if (m_lodCount > 1)
		{
			glBindTexture(GLTextureType[m_type], m_handle);
			glGenerateMipmap(GLTextureType[m_type]);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// TextureManager
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	TextureManager::TextureManager(void)
	{
	}
	//----------------------------------------------------------------------------//
	TextureManager::~TextureManager(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool TextureManager::_Init(void)
	{
		LOG_NODE("Initializing TextureManager");

		// default 2d
		{
			uint _cellSize = 16;
			uint _cellCount = 8;
			uint _size = _cellSize * _cellCount;
			Vec4ub* _imgData = new Vec4ub[_size*_size];
			Vec4ub* _img = _imgData;
			const Vec4ub _colors[2][2] =
			{
				{ { 0xff, 0xff, 0x00, 0xff }, { 0x00, 0x00, 0x00, 0x7f } },
				{ { 0x00, 0x00, 0x00, 0x7f }, { 0xff, 0xff, 0x00, 0xff } },
			};
			uint _vColor, _hColor;
			for (uint y = 0; y < _size; ++y)
			{
				_vColor = (y / _cellSize) % 2;
				for (uint x = 0; x < _size; ++x)
				{
					_hColor = (x / _cellSize) % 2;
					*_img++ = _colors[_vColor][_hColor];
				}
			}

			m_defaultTexture2D = new Texture("$BuiltIn/DefaultTexture2D", TT_2D, TF_MIPMAPS);
			m_defaultTexture2D->Realloc(PF_RGBA8_UNORM, _size, _size, 1);
			m_defaultTexture2D->Write(0, 0, _size, _size, PF_RGBA8_UNORM, _imgData);
			m_defaultTexture2D->GenerateLods();

			delete[] _imgData;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void TextureManager::_Destroy(void)
	{
		LOG_NODE("Destroying TextureManager");

		m_defaultTexture2D = nullptr;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// FrameBuffer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	FrameBuffer::FrameBuffer(void) : 
		m_size(Vec2ui::ZERO),
		m_handle(0)
	{
		glGenFramebuffers(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	FrameBuffer::~FrameBuffer(void)
	{
		glDeleteFramebuffers(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	void FrameBuffer::SetSize(const Vec2ui& _size)
	{
		for (uint i = 0; i < MAX_RENDER_TARGETS; ++i)
		{
			if (m_color[i].target) m_color[i].target->Realloc(_size.x, _size.y);
		}
		if (m_depthStencil.target) m_depthStencil.target->Realloc(_size.x, _size.y);
		m_size = _size;
	}
	//----------------------------------------------------------------------------//
	void FrameBuffer::SetColorTarget(uint _slot, Texture* _target, uint _zOffset)
	{
		if (_slot < MAX_RENDER_TARGETS)
		{
			Slot& _rtt = m_color[_slot];
			
			if (_rtt.target != _target || _rtt.zOffset != _zOffset)
			{
				int _currentFbo = 0;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_currentFbo);
				if (_currentFbo != m_handle) glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
				if (_target)
				{
					if (_zOffset)
					{
						uint _texTarget = _target->GetType() == TT_CUBE ? GL_TEXTURE_CUBE_MAP_NEGATIVE_X + _zOffset : GLTextureType[_target->GetType()];
						switch (_target->GetType())
						{
						case TT_2D:
						case TT_2D_MULTISAMPLE:
						case TT_CUBE:
							glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _slot, _texTarget, _target->_Handle(), 0);
							break;
						case TT_2D_ARRAY:
						case TT_3D:
							glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _slot, _texTarget, _target->_Handle(), 0, _zOffset);
							break;
						default: ASSERT(false, "Unknown enum"); break;
						}
					}
					else
					{
						glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _slot, _target->_Handle(), 0);
					}
				}
				else
				{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _slot, GL_TEXTURE_2D, 0, 0);
				}
				if (_currentFbo != m_handle) glBindFramebuffer(GL_FRAMEBUFFER, _currentFbo);

				_rtt.target = _target;
				_rtt.zOffset = _zOffset;
			}
		}
	}
	//----------------------------------------------------------------------------//
	void FrameBuffer::SetDepthStencilTarget(Texture* _target, uint _zOffset)
	{
		if (m_depthStencil.target != _target || m_depthStencil.zOffset != _zOffset)
		{
			int _currentFbo = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_currentFbo);
			if (_currentFbo != m_handle) glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
			if (_target)
			{
				if (_zOffset)
				{
					uint _texTarget = _target->GetType() == TT_CUBE ? GL_TEXTURE_CUBE_MAP_NEGATIVE_X + _zOffset : GLTextureType[_target->GetType()];
					switch (_target->GetType())
					{
					case TT_2D:
					case TT_2D_MULTISAMPLE:
					case TT_CUBE:
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _texTarget, _target->_Handle(), 0);
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, _texTarget, _target->_Handle(), 0);
						break;
					case TT_2D_ARRAY:
					case TT_3D:
						glFramebufferTexture3D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _texTarget, _target->_Handle(), 0, _zOffset);
						glFramebufferTexture3D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, _texTarget, _target->_Handle(), 0, _zOffset);
						break;
					default: ASSERT(false, "Unknown enum"); break;
					}
				}
				else
				{
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _target->_Handle(), 0);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, _target->_Handle(), 0);
				}
			}
			else
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
			}
			if (_currentFbo != m_handle) glBindFramebuffer(GL_FRAMEBUFFER, _currentFbo);

			m_depthStencil.target = _target;
			m_depthStencil.zOffset = _zOffset;
		}

	}
	//----------------------------------------------------------------------------//
	void FrameBuffer::GenerateLods(void)
	{
		for (uint i = 0; i < MAX_RENDER_TARGETS; ++i)
		{
			if (m_color[i].target) m_color[i].target->GenerateLods();
		}
		if (m_depthStencil.target) m_depthStencil.target->GenerateLods();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Shader utils
	//----------------------------------------------------------------------------//

	namespace
	{
		const uint GLShaderType[] =
		{
			GL_VERTEX_SHADER, // SST_VERTEX
			GL_FRAGMENT_SHADER, // SST_FRAGMENT
			GL_GEOMETRY_SHADER, // SST_GEOMETRY
		};

		String ShaderStageNames[] = { "vertex", "fragment", "geometry" };
		const String ShaderStageShortNames[] = { "vs", "fs", "gs" };

		String _ParseGLSLLog(const char* _log)
		{
			String _r, _name;
			while (*_log)
			{
				const char* _start = _log;
				while (*_log && (*_log < '0' || *_log > '9')) ++_log;
				_r.append(_start, _log);
				if (*_log >= '0' && *_log <= '9')
				{
					_start = _log;
					while (*_log && *_log >= '0' && *_log <= '9') ++_log;
					if (*_log == ':')
					{
						uint _id = 0;
						sscanf(_start, "%u", &_id);
						++_log;
						if (Shader::GetSourceName(_id, _name))
						{
							const char* _start2 = _log;
							while (*_log && *_log >= '0' && *_log <= '9') ++_log;
							if (_start2 != _log)
							{
								_r.append(_name);
								_r.append("(");
								_r.append(_start2, _log);
								_r.append(")");
							}
							else _r.append(_start, _log);
						}
						else _r.append(_start, _log);
					}
					else _r.append(_start, _log);
				}
			}
			if (_log[-1] == '\n' && _log[-2] == '\n' && _r.size() > 1) _r.resize(_r.size() - 1);
			return _r;
		}
	}

	//----------------------------------------------------------------------------//
	// ShaderParam
	//----------------------------------------------------------------------------//

	const ShaderParam ShaderParam::Null;

	//----------------------------------------------------------------------------//
	// ShaderStage
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ShaderStage::ShaderStage(Shader* _creator, const String& _name, ShaderStageType _type, const String& _entry, uint32 _id, uint32 _defsId) :
		m_shader(_creator),
		m_name(_name),
		m_type(_type),
		m_entry(_entry),
		m_id(_id),
		m_defsId(_defsId),
		m_handle(0),
		m_compiled(false),
		m_valid(false)
	{
		m_handle = glCreateShader(GLShaderType[_type]);
	}
	//----------------------------------------------------------------------------//
	ShaderStage::~ShaderStage(void)
	{
		//m_shader->m_stageInstances.erase(m_id);
		glDeleteShader(m_handle);
	}
	//----------------------------------------------------------------------------//
	bool ShaderStage::_Compile(void)
	{
		if (!m_compiled)
		{
			m_compiled = true;
			m_valid = true;
			m_log.clear();

			if (m_shader->_Load())
			{
				String _header;
				{
					// version
					_header += "#version 330 core\n";
					// shader type
					_header += "#define COMPILE_" + StrUpper(ShaderStageShortNames[m_type]) + "\n";
					// entry point
					if (m_entry != "main") _header += "#define main " + m_entry + "\n";
					// user definitions
					const StringArray& _defs = m_shader->_GetDefs(m_defsId);
					for (uint i = 0; i < _defs.size(); ++i)
					{
						_header += "#define ";
						_header += _defs[i];
						_header += "\n";
					}
				}

				int _status = 0, _length = 0;
				const char* _srcp[3] = { _header.c_str(), m_shader->GetData(MAX_SHADER_STAGES).c_str(), m_shader->GetData(m_type).c_str() };
				glShaderSource(m_handle, 3, _srcp, 0);
				glCompileShader(m_handle);
				glGetShaderiv(m_handle, GL_COMPILE_STATUS, &_status);
				glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &_length);

				if (!_status)
				{
					m_valid = false;
					m_log = m_name + " : Couldn't compile " + ShaderStageNames[m_type] + " shader stage :\n";
				}
				else if (_length > 1)
				{
					m_log = m_name + " : Successful compiled" + ShaderStageNames[m_type] + "shader stage :\n";
				}

				if (_length > 1)
				{
					Array<char> _log(_length + 1);
					glGetShaderInfoLog(m_handle, _length, &_length, &_log[0]);
					_log[_length] = 0;
					m_log += _ParseGLSLLog(&_log[0]);
				}
			}
			else
			{
				m_log += m_name + " : Invalid source code \"" + m_shader->GetName() + "\" :\n";
				m_log += m_shader->GetLog();
				m_valid = false;
			}

			if (!m_log.empty())
			{
				if (m_valid)
				{
					LOG_WARNING("%s : Warnings :\n%s", m_name.c_str(), m_log.c_str());
				}
				else
				{
					LOG_ERROR("%s : Errors :\n%s", m_name.c_str(), m_log.c_str());
				}
			}
		}
		return m_valid;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ShaderInstance
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ShaderInstance::ShaderInstance(Shader* _creator, const String& _name, const String& _vsEntry, const String& _fsEntry, const String& _gsEntry, uint32 _id, uint32 _defsId) :
		m_shader(_creator),
		m_name(_name),
		m_id(_id),
		m_defsId(_defsId),
		m_linked(false),
		m_valid(false)
	{
		m_handle = glCreateProgram();

		const String _entries[3] = { _vsEntry, _fsEntry, _gsEntry };
		for (uint i = 0; i < 3; ++i)
		{
			if (!_entries[i].empty())
			{
				m_stages[i] = m_shader->_CreateStage(static_cast<ShaderStageType>(i), _entries[i], _defsId);
				ASSERT(m_stages[i] != nullptr);
				glAttachShader(m_handle, m_stages[i]->m_handle);
			}
		}

		for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
		{
			glBindAttribLocation(m_handle, i, GLVertexAttribs[i].name);
		}

		_Link();
	}
	//----------------------------------------------------------------------------//
	ShaderInstance::~ShaderInstance(void)
	{
		for (uint i = 0; i < MAX_SHADER_STAGES; ++i)
		{
			if (m_stages[i])
			{
				glDetachShader(m_handle, m_stages[i]->m_handle);
				m_stages[i] = nullptr;
			}
		}
		m_shader->m_shaderInstances.erase(m_id);

		glDeleteProgram(m_handle);
	}
	//----------------------------------------------------------------------------//
	bool ShaderInstance::_Link(void)
	{
		if (!m_linked)
		{
			m_linked = true;
			m_valid = true;
			m_log.clear();

			if (m_shader->_Load())
			{
				for (uint i = 0; i < MAX_SHADER_STAGES; ++i)
				{
					if (m_stages[i] && !m_stages[i]->_Compile())
					{
						m_valid = false;
						m_log += m_name + " : Invalid " + ShaderStageNames[i] + " shader stage :\n";
						m_log += m_stages[i]->GetLog();
					}
				}

				//if (m_valid)
				{
					int _status = 0, _length = 0;
					glLinkProgram(m_handle);
					glGetProgramiv(m_handle, GL_LINK_STATUS, &_status);
					glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &_length);

					if (!_status)
					{
						m_valid = false;
						m_log += m_name + " : Couldn't link shader :\n";
					}
					else if (_length > 1)
					{
						m_log += m_name + " : Successful linked shader :\n";
					}

					if (_length > 1)
					{
						Array<char> _log(_length + 1);
						glGetProgramInfoLog(m_handle, _length, &_length, &_log[0]);
						_log[_length] = 0;
						m_log += _ParseGLSLLog(&_log[0]);
					}
				}

				// params
				if (m_valid)
				{
					Array<char> _name;
					int _count = 0, _length = 0, _currentProg = 0, _size;
					uint _type;
					
					// uniforms & samplers
					glGetIntegerv(GL_CURRENT_PROGRAM, &_currentProg);
					glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &_count);
					glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &_length);
					_name.resize(_length + 1);
					for (int i = 0; i < _count; ++i)
					{
						glGetActiveUniform(m_handle, i, _name.size(), &_length, &_size, &_type, &_name[0]);
						_name[_length] = 0;

						ShaderParam _param;
						_param.name = &_name[0];
						_param.count = 1;

						switch (_type)
						{
						case GL_INT: _param.type = SPT_UNIFORM_INT; break;
						case GL_INT_VEC2: _param.type = SPT_UNIFORM_VEC2I; break;
						case GL_INT_VEC3: _param.type = SPT_UNIFORM_VEC3I; break;
						case GL_INT_VEC4: _param.type = SPT_UNIFORM_VEC4I; break;
						case GL_FLOAT: _param.type = SPT_UNIFORM_FLOAT; break;
						case GL_FLOAT_VEC2: _param.type = SPT_UNIFORM_VEC2; break;
						case GL_FLOAT_VEC3: _param.type = SPT_UNIFORM_VEC3; break;
						case GL_FLOAT_VEC4: _param.type = SPT_UNIFORM_VEC4; break;
						case GL_FLOAT_MAT4: _param.type = SPT_UNIFORM_MAT44; break;
						case GL_FLOAT_MAT3x4: _param.type = SPT_UNIFORM_MAT34; break;
						case GL_SAMPLER_2D:
						case GL_SAMPLER_2D_SHADOW:
						case GL_INT_SAMPLER_2D: 
						case GL_UNSIGNED_INT_SAMPLER_2D: _param.type = SPT_SAMPLER_2D; break;
						case GL_SAMPLER_2D_ARRAY:
						case GL_SAMPLER_2D_ARRAY_SHADOW:
						case GL_INT_SAMPLER_2D_ARRAY:
						case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: _param.type = SPT_SAMPLER_2D_ARRAY; break;
						case GL_SAMPLER_2D_MULTISAMPLE: 
						case GL_INT_SAMPLER_2D_MULTISAMPLE: 
						case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: _param.type = SPT_SAMPLER_2D_MULTISAMPLE; break;
						case GL_SAMPLER_3D:
						case GL_INT_SAMPLER_3D:
						case GL_UNSIGNED_INT_SAMPLER_3D: _param.type = SPT_SAMPLER_3D; break;
						case GL_SAMPLER_CUBE:
						case GL_SAMPLER_CUBE_SHADOW:
						case GL_INT_SAMPLER_CUBE: 
						case GL_UNSIGNED_INT_SAMPLER_CUBE: _param.type = SPT_SAMPLER_CUBE; break;
						case GL_SAMPLER_BUFFER:
						case GL_INT_SAMPLER_BUFFER:
						case GL_UNSIGNED_INT_SAMPLER_BUFFER: _param.type = SPT_SAMPLER_BUFFER; break;
						}

						if (_param.type != SPT_UNKNOWN)
						{
							if (_param.IsTexture())
							{
								_param.internalId = glGetUniformLocation(m_handle, &_name[0]);
								_param.count = 1;
								_param.slot = m_textures.size();
								m_textures.push_back(m_params.size());
								if (m_textures.size() == 1) glUseProgram(m_handle);
								glUniform1i(_param.internalId, _param.slot);
							}
							else
							{
								char* _array = strchr(&_name[0], '[');
								if (_array) *_array = 0;
								_param.internalId = glGetUniformLocation(m_handle, &_name[0]);
								_param.name = &_name[0];
								_param.count = _size;
								_param.size = 0; TODO("init _param.size");
								m_uniforms.push_back(m_params.size());
							}
							m_paramNames[_param.name.c_str()] = m_params.size();
							m_params.push_back(_param);
						}
						else
						{
							m_log += StrFormat("%s : Unknown parameter \"%s\" (type = 0x%04x)\n", m_name.c_str(), &_name[0], _type);
						}
					}

					// uniform blocks
					glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCKS, &_count);
					glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &_length);
					_name.resize(_length + 1);
					for (int i = 0; i < _count; ++i)
					{
						glGetActiveUniformBlockName(m_handle, i, _name.size(), &_length, &_name[0]);
						_name[_length] = 0;
						char* _array = strchr(&_name[0], '[');
						if (_array) *_array = 0;

						ShaderParam _param;
						_param.name = &_name[0];
						_param.type = SPT_UNIFORM_BLOCK;
						_param.count = 1;
						//_param.bufferSignature;
						_param.size = 0; TODO("init _param.size");
						_param.slot = 0; TODO("init _param.slot");
						_param.internalId = glGetUniformBlockIndex(m_handle, &_name[0]);
						//_param.slot = GRenderSystem->GetOrAddUniformBlockSlot(_param.name);
						m_buffers.push_back(m_params.size());
						m_paramNames[_param.name.c_str()] = m_params.size();
						m_params.push_back(_param);

						TODO("Uniform groups");
						if (_param.slot)
						{
							glUniformBlockBinding(m_handle, _param.internalId, _param.slot);
						}
					}

					if (!m_textures.empty()) glUseProgram(_currentProg);

				} // params
			}
			else
			{
				m_log += m_name + " : Invalid source code \"" + m_shader->GetName() + "\" :\n";
				m_log += m_shader->GetLog();
				m_valid = false;
			}

			if (!m_log.empty())
			{
				if (m_valid)
				{
					LOG_WARNING("%s : Warnings :\n%s", m_name.c_str(), m_log.c_str());
				}
				else
				{
					LOG_ERROR("%s : Errors :\n%s", m_name.c_str(), m_log.c_str());
				}
			}
		}
		return m_valid;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Shader
	//----------------------------------------------------------------------------//

	HashMap<uint32, String> Shader::s_names;
	uint32 Shader::s_nextId;

	//----------------------------------------------------------------------------//
	Shader::Shader(const String& _name)	:
		Resource(_name),
		m_uid(++s_nextId)
	{
		s_names[m_uid] = m_name;
	}
	//----------------------------------------------------------------------------//
	Shader::~Shader(void)
	{
		for (auto i = m_includes.begin(), e = m_includes.end(); i != e; ++i)
		{
			i->second->m_dependents.erase(m_uid);
		}
		m_includes.clear();
		m_shaderInstances.clear();
		s_names.erase(m_uid);
	}
	//----------------------------------------------------------------------------//
	ShaderInstancePtr Shader::CreateInstance(const String& _vsEntry, const String& _fsEntry, const String& _gsEntry, const String& _defs)
	{
		uint32 _defsId = _AddDefs(_defs);
		String _name = m_name + "("; // e.g. "Shaders/Default.shader(vs@vs_main, fs@fs_main, gs@gs_main, defs@0x1234abcd)"
		{
			if (!_vsEntry.empty()) _name += "vs@" + _vsEntry;
			if (!_fsEntry.empty())
			{
				if (!_name.empty()) _name += ", ";
				_name += "fs@" + _fsEntry;
			}
			if (!_gsEntry.empty())
			{
				if (!_name.empty()) _name += ", ";
				_name += "gs@" + _gsEntry;
			}
			if (_defsId)
			{
				if (!_name.empty()) _name += ", ";
				_name += StrFormat("defs@0x%08x", _defsId);
			}
		}
		if (_name.back() == '(') _name += "null";
		_name += ")";
		uint32 _id = CRC32(_name);

		{
			auto _exists = m_shaderInstances.find(_id);
			if (_exists != m_shaderInstances.end()) return _exists->second;
		}

		ShaderInstancePtr _shader = new ShaderInstance(this, _name, _vsEntry, _fsEntry, _gsEntry, _id, _defsId);
		m_shaderInstances[_id] = _shader;
		return _shader;
	}
	//----------------------------------------------------------------------------//
	bool Shader::GetSourceName(uint32 _uid, String& _name)
	{
		auto _exists = s_names.find(_uid);
		if (_exists != s_names.end())
		{
			_name = _exists->second;
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool Shader::_Load(void)
	{
		if (m_state != RS_LOADING && m_state != RS_LOADED && m_state != RS_INVALID)
		{
			// if(m_state == RS_QUEUED && m_manager) m_manager->_Unqueue(this);
			m_state = RS_LOADING;

			String _rawData;
			{
				DataStream _ds = GFileSystem->Open(m_name);
				if (!_ds)
				{
					m_log += m_name + " : File not found\n";
					goto $_fail;
				}
				_rawData = _ds.ReadString();
			}

			uint _section = MAX_SHADER_STAGES, _prevLine = ~0;
			CStream s(_rawData.c_str());

			while (*s)
			{
				// skip comments & empty lines
				const char* s2 = s;
				while (*s2 && strchr(" \t", *s2)) ++s2;
				if (*s2 == '/')
				{
					if (s2[1] == '/')
					{
						s += (s2 - s) + 2;
						while (*s && !strchr("\n\r", *s)) ++s;
						continue;
					}
					else if (s2[1] == '*')
					{
						s += (s2 - s) + 2;
						while (*s && !(*s == '*' && s[1] == '/')) ++s;
						s += 2;
						continue;
					}
				}
				else if (strchr("\n\r", *s2))
				{
					s += (s2 - s) + 1;
					continue;
				}

				if (*s == '#')
				{
					s2 = s + 1;
					while (*s2 && strchr(" \t", *s2)) ++s2;
					if (strncmp(s2, "include", 7) == 0)
					{
						s2 += 7;
						while (*s2 && strchr(" \t", *s2)) ++s2;
						// "
						if (*s2 != '"')
						{
							m_log += StrFormat("%s(%u:%u) : Expected '\"'\n", m_name.c_str(), s.l, (s2 - s) + 1);
							goto $_fail;
						}
						++s2;

						// name
						uint _start = s2 - s;
						while (*s2 && strchr(" \t", *s2)) ++s2;
						String _name;
						while (*s2 && !strchr("\"\n\r", *s2)) _name += *s2++;

						if (_name.empty())
						{
							m_log += StrFormat("%s(%u:%u) : Expected filename\n", m_name.c_str(), s.l, (s2 - s) + 1);
							goto $_fail;
						}

						// "
						if (*s2 != '"')
						{
							m_log += StrFormat("%s(%u:%u) : Expected '\"'\n", m_name.c_str(), s.l, (s2 - s) + 1);
							goto $_fail;
						}
						++s2;

						// include
						_name = MakeNormPath(_name, false);
						ShaderPtr _inc = GShaderManager->LoadShader(_name);
						if (!_inc || _inc == this)
						{
							m_log += StrFormat("%s(%u) : Internal error\n", m_name.c_str(), s.l);
							goto $_fail;
						}
						if (!_inc->_Load())
						{
							m_log += StrFormat("%s(%u) : Invalid include file \"%s\" :\n", m_name.c_str(), s.l, _inc->m_name.c_str());
							m_log += _inc->m_log;
							goto $_fail;
						}
						m_data[_section].append(_inc->m_data[MAX_SHADER_STAGES]);
						m_includes[_inc->m_uid] = _inc;
						_inc->m_dependents[m_uid] = this;

						s += (s2 - s);
						continue;
					}
					else if (strncmp(s2, "pragma", 6) == 0)
					{
						s2 += 6;
						while (*s2 && strchr(" \t", *s2)) ++s2;
						if (strncmp(s2, "section", 7) == 0)
						{
							s2 += 7;
							// (
							while (*s2 && strchr(" \t", *s2)) ++s2;
							if (*s2 != '(')
							{
								m_log += StrFormat("%s(%u:%u) : Expected '('\n", m_name.c_str(), s.l, (s2 - s) + 1);
								goto $_fail;
							}
							++s2;

							// id
							while (*s2 && strchr(" \t", *s2)) ++s2;
							uint _start = s2 - s;
							String _name;
							while ((*s2 >= 'a' && *s2 <= 'z') || *s2 == '_') _name += *s2++;
							if (_name == "vertex_shader") _section = SST_VERTEX;
							else if (_name == "fragment_shader") _section = SST_FRAGMENT;
							else if (_name == "geometry_shader") _section = SST_GEOMETRY;
							else if (_name.empty())
							{
								m_log += StrFormat("%s(%u:%u) : Expected identifier\n", m_name.c_str(), s.l, _start + 1);
								goto $_fail;
							}
							else
							{
								m_log += StrFormat("%s(%u:%u) : Unknown section '%s'\n", m_name.c_str(), s.l, _start + 1, _name.c_str());
								goto $_fail;
							}

							// )
							while (*s2 && strchr(" \t", *s2)) ++s2;
							if (*s2 != ')')
							{
								m_log += StrFormat("%s(%u:%u) : Expected ')'\n", m_name.c_str(), s.l, (s2 - s) + 1);
								goto $_fail;
							}
							s += (s2 - s) + 1;
							continue;
						}
					} // pragma
				} // #

				while (*s2 && !strchr("\n\r", *s2) && !(*s2 == '/' && strchr("*/", s2[1]))) ++s2;
				if (_prevLine + 1 != s.l) m_data[_section].append(StrFormat("#line %u %u // %s(%u)\n", s.l, m_uid, m_name.c_str(), s.l));
				if (s.c > 1) m_data[_section].resize(m_data[_section].size() + (s.c - 1), ' ');
				m_data[_section].append(s, s2 - s);
				m_data[_section].append("\n");
				s += (s2 - s);
				_prevLine = s.l;
			}

			m_state = RS_LOADED;
		}

		//_WaitLoading();
		return m_state == RS_LOADED;

	$_fail:
		m_state = RS_INVALID;
		return false;
	}
	//----------------------------------------------------------------------------//
	ShaderStage* Shader::_CreateStage(ShaderStageType _type, const String& _entry, uint32 _defsId)
	{
		String _name = m_name + "(" + ShaderStageShortNames[_type] + "@" + _entry + (_defsId ? StrFormat(", defs@0x%08x)", _defsId) : ")");	// e.g. "Shaders/Default.shader(vs@vs_main, defs@0x1234abcd)"
		uint32 _id = CRC32(_name);

		{
			auto _exists = m_stageInstances.find(_id);
			if (_exists != m_stageInstances.end()) return _exists->second;
		}

		ShaderStagePtr _stage = new ShaderStage(this, _name, _type, _entry, _id, _defsId);
		m_stageInstances[_id] = _stage;
		return _stage;
	}
	//----------------------------------------------------------------------------//
	uint32 Shader::_AddDefs(const String& _defs)
	{
		StringArray _defsArray;
		SplitString(_defs, " ,;", _defsArray);
		std::sort(_defsArray.begin(), _defsArray.end());
		String _ndefs;
		for (uint i = 0, s = _defsArray.size(); i < s; ++i)
		{
			_ndefs += _defsArray[i];
			if (i + 1 < s) _ndefs += ", ";
		}
		uint32 _id = CRC32(_ndefs);
		if (m_defs.find(_id) == m_defs.end())
		{
			m_defs[_id] = _defsArray;
		}
		return _id;
	}
	//----------------------------------------------------------------------------//
	const StringArray& Shader::_GetDefs(uint32 _defsId)
	{
		ASSERT(m_defs.find(_defsId) != m_defs.end());
		return m_defs[_defsId];
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ShaderManager
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ShaderPtr ShaderManager::LoadShader(const String& _name)
	{
		uint32 _id = MakeResourceId(_name);
		if (!_id) return nullptr;
		// exists
		{
			auto _exists = m_shaders.find(_id);
			if (_exists != m_shaders.end()) return _exists->second;
		}
		// load
		ShaderPtr _shader = new Shader(_name);
		m_shaders[_id] = _shader;
		ASSERT(_id == _shader->GetId());
		return _shader;
	}
	//----------------------------------------------------------------------------//
	ShaderManager::ShaderManager(void)
	{
		TODO("добавить возможность создания шейдера из текста вручную");
	}
	//----------------------------------------------------------------------------//
	ShaderManager::~ShaderManager(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool ShaderManager::_Init(void)
	{
		LOG_NODE("Initializing ShaderManager");

		return true;
	}
	//----------------------------------------------------------------------------//
	void ShaderManager::_Destroy(void)
	{
		LOG_NODE("Destroying ShaderManager");
		m_shaders.clear();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ------
	//----------------------------------------------------------------------------//

	/*const uint GLPrimitiveType[] =
	{
		GL_POINTS, // PT_POINTS
		GL_LINES, // PT_LINES
		GL_LINES_ADJACENCY, // PT_LINES_ADJACENCY
		GL_LINE_STRIP, // PT_LINE_STRIP
		GL_LINE_STRIP_ADJACENCY, // PT_LINE_STRIP_ADJACENCY
		GL_LINE_LOOP, // PT_LINE_LOOP
		GL_TRIANGLES, // PT_TRIANGLES
		GL_TRIANGLES_ADJACENCY, // PT_TRIANGLES_ADJACENCY
		GL_TRIANGLE_STRIP, // PT_TRIANGLE_STRIP
		GL_TRIANGLE_STRIP_ADJACENCY, // PT_TRIANGLE_STRIP_ADJACENCY
	};


	const uint GLStencilOp[][2] =
	{
		{ GL_KEEP, GL_KEEP }, // SO_KEEP
		{ GL_ZERO, GL_ZERO }, // SO_ZERO
		{ GL_REPLACE, GL_REPLACE }, // SO_REPLACE
		{ GL_INCR, GL_DECR }, // SO_INC
		{ GL_INCR_WRAP, GL_DECR_WRAP }, // SO_INC_WRAP
		{ GL_DECR, GL_INCR }, // SO_DEC
		{ GL_DECR_WRAP, GL_INCR_WRAP }, // SO_DEC_WRAP
		{ GL_INVERT, GL_INVERT }, // SO_INVERT
	};

	const uint GLCompareFunc[] =
	{
		GL_NEVER, // CF_NEVER
		GL_ALWAYS, // CF_ALWAYS
		GL_LESS, // CF_LESS
		GL_LEQUAL, // CF_LEQUAL
		GL_EQUAL, // CF_EQUAL
		GL_NOTEQUAL, // CF_NEQUAL
		GL_GREATER, // CF_GREATER
		GL_GEQUAL, // CF_GEQUAL
	};

	const uint GLPolygonFace[] =
	{
		GL_FRONT, // PF_FRONT
		GL_BACK, // PF_BACK
		GL_FRONT_AND_BACK, // PF_BOTH
	};

	const uint GLPolygonMode[] =
	{
		GL_FILL, // PM_SOLID
		GL_LINE, // PM_WIREFRAME
		GL_POINT, // PM_POINTS
	};

	const uint GLFrontFace[] =
	{
		GL_CCW, // FF_COUNTERCLOCKWISE
		GL_CW, // FF_CLOCKWISE
	};*/

	//----------------------------------------------------------------------------//
	// BlendState
	//----------------------------------------------------------------------------//

	const uint16 GLBlendFactor[] =
	{
		GL_ZERO, // BF_ZERO
		GL_ONE, // BF_ONE
		GL_SRC_COLOR, // BF_SRC_COLOR
		GL_ONE_MINUS_SRC_COLOR, // BF_INV_SRC_COLOR
		GL_DST_COLOR, // BF_DST_COLOR
		GL_ONE_MINUS_DST_COLOR, // BF_INV_DST_COLOR
		GL_SRC_ALPHA, // BF_SRC_ALPHA
		GL_ONE_MINUS_SRC_ALPHA, // BF_INV_SRC_ALPHA
		GL_DST_ALPHA, // BF_DST_ALPHA
		GL_ONE_MINUS_DST_ALPHA, // BF_INV_DST_ALPHA
		GL_NONE, // BF_CONSTANT
		GL_NONE, // BF_INV_CONSTANT
	};

	const uint16 GLBlendFunc[] =
	{
		GL_FUNC_ADD, // BF_ADD
		GL_FUNC_SUBTRACT, // BF_SUB
		GL_FUNC_REVERSE_SUBTRACT, // BF_REVERSE_SUBTRACT
		GL_MIN, // BF_MIN
		GL_MAX, // BF_MAX
	};


	BlendState::BlendState(const BlendStateDesc& _desc) : m_desc(_desc)
	{
	}
	BlendState::~BlendState(void)
	{
	}
	void BlendState::_Bind(void)
	{
		if (m_desc.enabled)
		{
			glEnable(GL_BLEND);

		}
		else
		{
			glDisable(GL_BLEND);
		}
	}


	//----------------------------------------------------------------------------//
	// DebugOutputARB
	//----------------------------------------------------------------------------//

	bool s_enableDebugOutputARB = true;
	// arb
	void APIENTRY _DebugOutputARB(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const GLchar* _message, const GLvoid* _ud)
	{
		if (!s_enableDebugOutputARB) return;

		const char* _dsource;
		const char* _dtype;
		const char* _dseverity;
		switch (_source)
		{
		case GL_DEBUG_SOURCE_API_ARB:  _dsource = "OpenGL"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: _dsource = "Windows"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: _dsource = "Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: _dsource = "Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION_ARB: _dsource = "Application"; break;
		case GL_DEBUG_SOURCE_OTHER_ARB: _dsource = "Other"; break;
		default: _dsource = "Unknown";
		}
		switch (_type)
		{
		case GL_DEBUG_TYPE_ERROR_ARB: _dtype = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: _dtype = "Deprecated behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: _dtype = "Undefined behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY_ARB: _dtype = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE_ARB: _dtype = "Performance"; break;
		case GL_DEBUG_TYPE_OTHER_ARB: _dtype = "Message"; break;
			//case GL_DEBUG_TYPE_MARKER: _dtype = "Marker"; break;
			//case GL_DEBUG_TYPE_PUSH_GROUP: _dtype = "Push group"; break;
			//case GL_DEBUG_TYPE_POP_GROUP: _dtype = "Pop group"; break;
		default: _dtype = "unknown"; break;
		}
		switch (_severity)
		{
		case GL_DEBUG_SEVERITY_HIGH_ARB: _dseverity = "high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM_ARB: _dseverity = "medium"; break;
		case GL_DEBUG_SEVERITY_LOW_ARB: _dseverity = "low"; break;
			//case GL_DEBUG_SEVERITY_NOTIFICATION: _dseverity = "notification"; break;
		default: _dseverity = "?"; break;
		}

		LOG_DEBUG("%s : %s(%s) %d : %s", _dsource, _dtype, _dseverity, _id, _message);

		/*const LogNode* _top = LogNode::GetTop();
		if (_top)
		{
		LogNode::Message(_top->Func(), _top->File(), _top->Line(), LL_DEBUG, Format("%s : %s(%s) %d : %s", _dsource, _dtype, _dseverity, _id, _message));
		}
		else
		{
		LogNode::Message(__FUNCTION__, __FILE__, __LINE__, LL_DEBUG, Format("%s(External) : %s(%s) %d : %s", _dsource, _dtype, _dseverity, _id, _message));
		} */
	}

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void RenderSystem::SetGeometry(HardwareBuffer* _vertices, HardwareBuffer* _indices)
	{
		glBindVertexArray(_vertices ? _vertices->_VertexArray() : 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices ? _indices->_Handle() : 0);
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::SetShader(ShaderInstance* _shader)
	{
		if (m_currentShader != _shader)
		{
			m_currentShader = _shader;
			glUseProgram(_shader ? _shader->_Handle() : 0);
		}
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::SetUniformRaw(int _index, uint _count, const void* _value)
	{
		ASSERT(m_currentShader != nullptr);
		const ShaderParam& _param = m_currentShader->GetParamInfo(_index);
		if (_param.internalId >= 0)
		{
			switch (_param.type)
			{
			case SPT_UNIFORM_INT: glUniform1iv(_param.internalId, _count, (const int*)_value); break;
			case SPT_UNIFORM_VEC2I: glUniform2iv(_param.internalId, _count, (const int*)_value); break;
			case SPT_UNIFORM_VEC3I: glUniform3iv(_param.internalId, _count, (const int*)_value); break;
			case SPT_UNIFORM_VEC4I: glUniform4iv(_param.internalId, _count, (const int*)_value); break;
			case SPT_UNIFORM_FLOAT: glUniform1fv(_param.internalId, _count, (const float*)_value); break;
			case SPT_UNIFORM_VEC2: glUniform2fv(_param.internalId, _count, (const float*)_value); break;
			case SPT_UNIFORM_VEC3: glUniform3fv(_param.internalId, _count, (const float*)_value); break;
			case SPT_UNIFORM_VEC4: glUniform4fv(_param.internalId, _count, (const float*)_value); break;
			case SPT_UNIFORM_MAT34: glUniformMatrix3x4fv(_param.internalId, _count, true, (const float*)_value); break;
			case SPT_UNIFORM_MAT44: glUniformMatrix4fv(_param.internalId, _count, true, (const float*)_value); break;
			}
		}
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::SetFrameBuffer(FrameBuffer* _fb)
	{
		if (_fb != m_currentFrameBuffer)
		{
			TODO("glDrawBuffers");
			glBindFramebuffer(GL_FRAMEBUFFER, _fb ? _fb->_Handle() : 0);
			//if (_fb) glDrawBuffer(GL_COLOR_ATTACHMENT0);
			//else glDrawBuffer(GL_BACK);
			m_currentFrameBuffer = _fb;
		}
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::BeginFrame(void)
	{

	}
	//----------------------------------------------------------------------------//
	void RenderSystem::EndFrame(void)
	{
		bool _debugOutput = s_enableDebugOutputARB;
		s_enableDebugOutputARB = false;

		GDevice->_SwapBuffers();
		glGetError();

		s_enableDebugOutputARB = _debugOutput;
	}
	//----------------------------------------------------------------------------//
	RenderSystem::RenderSystem(void)
	{
		new TextureManager;
		new ShaderManager;
	}
	//----------------------------------------------------------------------------//
	RenderSystem::~RenderSystem(void)
	{
		delete GShaderManager;
		delete GTextureManager;
	}
	//----------------------------------------------------------------------------//
	bool RenderSystem::_Init(void)
	{
		LOG_NODE("Initializing RenderSystem");

		if(!ogl_LoadFunctions())
		{
			LOG_ERROR("Counld't load OpenGL functions");
			return false;
		}

		LOG_INFO("OpenGL %s on %s, %s", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));
		LOG_INFO("GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		LOG_INFO("GL_ARB_debug_output : %s", (ogl_ext_ARB_debug_output ? ("enabled") : ("disabled")));
		if (ogl_ext_ARB_debug_output)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
			glDebugMessageCallbackARB(&_DebugOutputARB, 0);
		}

		if (!GTextureManager->_Init())
		{
			LOG_ERROR("Counld't initialize TextureManager");
			return false;
		}

		if (!GShaderManager->_Init())
		{
			LOG_ERROR("Counld't initialize ShaderManager");
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void RenderSystem::_Destroy(void)
	{
		LOG_NODE("Destroying RenderSystem");

		GShaderManager->_Destroy();
		GTextureManager->_Destroy();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
