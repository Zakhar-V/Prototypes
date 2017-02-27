#include "GraphicsCore.hpp"
#include "GL/glLoad.h"
#include <SDL.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	const uint PrimitiveType2GL[] =
	{
		GL_POINTS,
		// todo
	};

	//----------------------------------------------------------------------------//
	// Buffer
	//----------------------------------------------------------------------------//

	const uint MappingMode2GL[] =
	{
		0, // MM_None
		GL_MAP_READ_BIT, // MM_Read
		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT, // MM_ReadWrite
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT, // MM_Discard
	};

	//----------------------------------------------------------------------------//
	BufferObject::BufferObject(bool _dynamic, uint _size, uint _esize, const void* _data) :
		m_size(_size),
		m_elementSize(_esize),
		m_handle(0)
	{
		glGenBuffers(1, &m_handle);
		glBindBuffer(GL_ARRAY_BUFFER, m_handle);
		glBufferData(GL_ARRAY_BUFFER, _size, _data, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	}
	//----------------------------------------------------------------------------//
	BufferObject::~BufferObject(void)
	{
		glDeleteBuffers(1, &m_handle);
	}
	//----------------------------------------------------------------------------//
	uint8* BufferObject::Map(MappingMode _mode, uint _offset, uint _size)
	{
		if (_mode == MM_None)
			return nullptr;
		return (uint8*)glMapNamedBufferRangeEXT(m_handle, _offset, _size, MappingMode2GL[_mode]);
	}
	//----------------------------------------------------------------------------//
	void BufferObject::Unmap(void)
	{
		glUnmapNamedBufferEXT(m_handle);
	}
	//----------------------------------------------------------------------------//
	void BufferObject::Copy(BufferObject* _src, uint _srcOffset, uint _dstOffset, uint _size)
	{
		if (_src)
			glNamedCopyBufferSubDataEXT(_src->m_handle, m_handle, _srcOffset, _dstOffset, _size);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// VertexFormat
	//----------------------------------------------------------------------------//

	struct GLVertexAttrib
	{
		uint type;
		uint components;
		bool integer;
		bool normalized;
	}
	const VertexAttrib2GL[] =
	{
		{ 0, 0, false, false }, // VT_Unknown
		{ GL_HALF_FLOAT, 2, false, false }, // VT_Half2
		{ GL_HALF_FLOAT, 4, false, false }, // VT_Half4
		{ GL_FLOAT, 1, false, false }, // VT_Float
		{ GL_FLOAT, 2, false, false }, // VT_Float2
		{ GL_FLOAT, 3, false, false }, // VT_Float3
		{ GL_FLOAT, 4, false, false }, // VT_Float4
		{ GL_UNSIGNED_BYTE, 4, true, false }, // VT_UByte4
		{ GL_UNSIGNED_BYTE, 4, false, true }, // VT_UByte4N
		{ GL_BYTE, 4, true, false }, // VT_Byte4
		{ GL_BYTE, 4, false, true }, // VT_Byte4N
	};

	const char* VertexAttribNames[] =
	{
		"vaPosition", // VS_Position
		"vaNormal", // VS_Normal
		"vaTangent", // VS_Tangent
		"vaColor", // VS_Color
		"vaColor2", // VS_Color2
		"vaWeights", // VS_Weights
		"vaIndices", // VS_Indices
		"vaTexCoord", // VS_TexCoord
		"vaTexCoord2", // VS_TexCoord2
		"vaTexCoord3", // VS_TexCoord3
		"vaTexCoord4", // VS_TexCoord4
		"vaLightMap", // VS_LightMap
		"vaAux0", // VS_Aux0
		"vaAux1", // VS_Aux1
		"vaAux2", // VS_Aux2
		"vaAux3", // VS_Aux3
	};

	HashMap<uint, uint> VertexFormat::s_indices;
	Array<VertexFormat*> VertexFormat::s_instances;

	//----------------------------------------------------------------------------//
	VertexFormat::VertexFormat(Array<VertexAttrib>&& _attribs, uint _streams, uint _mask) :
		m_streams(_streams),
		m_attribMask(_mask),
		m_attribs(_attribs)
	{
	}
	//----------------------------------------------------------------------------//
	VertexFormat::~VertexFormat(void)
	{
	}
	//----------------------------------------------------------------------------//
	uint VertexFormat::_Bind(uint _activeAttribs)
	{
		for (size_t i = 0, s = m_attribs.size(); i < s; ++i)
		{
			const VertexAttrib& _attrib = m_attribs[i];
			const GLVertexAttrib& _glattrib = VertexAttrib2GL[_attrib.type];
			glVertexAttribBinding(_attrib.semantic, _attrib.stream);
			glVertexAttribDivisor(_attrib.semantic, _attrib.divisor);
			if (_glattrib.integer)
				glVertexAttribIFormat(_attrib.semantic, _glattrib.components, _glattrib.type, _attrib.offset);
			else
				glVertexAttribFormat(_attrib.semantic, _glattrib.components, _glattrib.type, _glattrib.normalized, _attrib.offset);
		}

		for (uint i = 0, bit = 1; ((m_attribMask ^ _activeAttribs) >> i) && i < MAX_VERTEX_ATTRIBS; ++i, bit <<= 1)
		{
			if ((m_attribMask ^ _activeAttribs) & bit)
			{
				if (m_attribMask & bit)
					glDisableVertexAttribArray(i);
				else
					glEnableVertexAttribArray(i);
			}
		}

		return m_attribMask;
	}
	//----------------------------------------------------------------------------//
	bool VertexFormat::_InitCache(void)
	{
		_AddInstance(nullptr);
		return false;
	}
	//----------------------------------------------------------------------------//
	void VertexFormat::_DestroyCache(void)
	{
		for (auto _vf : s_instances)
			delete _vf;
	}
	//----------------------------------------------------------------------------//
	VertexFormat* VertexFormat::_AddInstance(const VertexAttrib* _attribs)
	{
		uint _numAttribs = 0;
		while (_attribs && _attribs[_numAttribs].type != VT_Unknown)
			++_numAttribs;

		uint _hash = Hash(_attribs, sizeof(VertexAttrib) * _numAttribs);

		auto _exists = s_indices.find(_hash);
		if (_exists != s_indices.end())
			return s_instances[_exists->second];

		uint _attribMask = 0, _streamMask = 0;
		Array<VertexAttrib> _attribArray;
		_attribArray.reserve(_numAttribs);
		for (uint i = 0; i < _numAttribs; ++i)
		{
			const VertexAttrib& _attrib = _attribs[i];

			assert((_attribMask & (1 << _attrib.semantic)) == 0); // already exists
			assert(_attrib.semantic < VS_MaxAttribs); // invalid semantic
			assert(_attrib.stream < MAX_VERTEX_STREAMS); // invalid stream

			_attribMask |= (1 << _attrib.semantic);
			_streamMask |= (1 << _attrib.stream);
		}

		VertexFormat* _newInstance = new VertexFormat(std::move(_attribArray), _streamMask, _attribMask);
		s_indices[_hash] = (uint)s_instances.size();
		s_instances.push_back(_newInstance);

		return _newInstance;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ShaderDefines
	//----------------------------------------------------------------------------//

	const ShaderDefines ShaderDefines::Empty;
	HashMap<uint, ShaderDefines> ShaderDefines::s_instances;

	//----------------------------------------------------------------------------//
	ShaderDefines::ShaderDefines(void) :
		m_hash(0)
	{
	}
	//----------------------------------------------------------------------------//
	ShaderDefines::ShaderDefines(const String& _str) :
		m_hash(0)
	{
		AddString(_str);
	}
	//----------------------------------------------------------------------------//
	ShaderDefines::ShaderDefines(ShaderDefines&& _temp) :
		m_defs(std::move(_temp.m_defs)),
		m_text(std::move(_temp.m_text)),
		m_hash(_temp.m_hash)
	{
	}
	//----------------------------------------------------------------------------//
	ShaderDefines::ShaderDefines(const ShaderDefines& _other) :
		m_defs(_other.m_defs),
		m_text(_other.m_text),
		m_hash(_other.m_hash)
	{
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::operator = (ShaderDefines&& _temp)
	{
		m_defs = std::move(_temp.m_defs);
		m_text = std::move(_temp.m_text);
		m_hash = _temp.m_hash;
		return *this;
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::operator = (const ShaderDefines& _rhs)
	{
		m_defs = _rhs.m_defs;
		m_text = _rhs.m_text;
		m_hash = _rhs.m_hash;
		return *this;
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::Clear(void)
	{
		m_hash = 0;
		m_defs.clear();
		return *this;
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::AddString(const String& _str)
	{
		m_hash = 0;
		Array<String> _splitted;
		StrSplit(_str, ";|", _splitted);
		for (String& _def : _splitted)
		{
			_def = StrTrim(_def, " \t\n\r");
			if (!_def.empty())
			{
				const char* _e = strchr(_def.c_str(), '=');
				if (_e)
				{
					String _name = StrTrim(String(_def.c_str(), _e - 1), " \t\n\r");
					String _val = StrTrim(String(_def.c_str(), _e + 1), " \t\n\r");
					m_defs[_name] = _val;
				}
				else
				{
					m_defs[_def] = "";
				}
			}
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::AddDef(const String& _def, const String& _val)
	{
		m_hash = 0;
		m_defs[_def] = _val;
		return *this;
	}
	//----------------------------------------------------------------------------//
	ShaderDefines& ShaderDefines::AddDefs(const ShaderDefines& _defs)
	{
		if (&m_hash != &_defs.m_hash)
		{
			m_hash = 0;
			m_defs.insert(_defs.m_defs.begin(), _defs.m_defs.end());
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	void ShaderDefines::_Update(void) const
	{
		if (!m_hash && !m_defs.empty())
		{
			m_text.clear();
			for (auto i = m_defs.begin(); i != m_defs.end();)
			{
				if (i->first.empty())
				{
					i = m_defs.erase(i);
				}
				else
				{
					m_text += "#define ";
					m_text += i->first;
					m_text += " ";
					m_text += i->second;
					m_text += "\n";
					m_hash = StrHash(i->first, m_hash);
					m_hash = StrHash(i->second, m_hash);
					++i;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	uint ShaderDefines::AddUnique(const ShaderDefines& _defs)
	{
		uint _hash = _defs.GetHash();
		if (s_instances.find(_hash) == s_instances.end())
			s_instances[_hash] = _defs;
		return _hash;
	}
	//----------------------------------------------------------------------------//
	const ShaderDefines& ShaderDefines::GetUnique(uint _id)
	{
		auto _exists = s_instances.find(_id);
		if (_exists == s_instances.end())
			return _exists->second;
		return Empty;
	}
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	// ShaderResource
	//----------------------------------------------------------------------------//
	
	const char* ShaderTypePrefix[] =
	{
		"VS", // ST_Vertex
		"FS", // ST_Fragment
		"GS", // ST_Geometry
	};

	Array<String> ShaderSource::s_names;
	HashMap<uint, uint16> ShaderSource::s_nameIndices;

	//----------------------------------------------------------------------------//
	ShaderSource::ShaderSource(void) :
		m_nameId(0),
		m_checksum(0),
		m_processed(false),
		m_loaded(false)
	{

	}
	//----------------------------------------------------------------------------//
	ShaderSource::~ShaderSource(void)
	{
		m_includes.clear();
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::BeginReload(void)
	{
		if (Resource::BeginReload())
		{
			m_loaded = false;
			_Invalidate();
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	ShaderPtr ShaderSource::CreateInstance(ShaderType _type, const ShaderDefines& _defs)
	{
		uint _defsId = ShaderDefines::AddUnique(_defs);

		auto _exists = m_instances.find(_defsId);
		if (_exists != m_instances.end())
			return _exists->second;

		String _name = m_name + StrFormat("@%s@%08x", ShaderTypePrefix[_type], _defsId);
		uint _uid = NameHash(_name);

		Touch();

		ShaderPtr _instance = new Shader(_type, this, _defsId, _name, _uid);
		m_instances[_uid] = _instance;

		return _instance;
	}
	//----------------------------------------------------------------------------//
	void ShaderSource::_InitResource(ResourceManager* _mgr, const String& _name, uint _uid, uint _flags)
	{
		Resource::_InitResource(_mgr, _name, _uid, _flags);
		m_nameId = _AddShaderName(m_name);
	}
	//----------------------------------------------------------------------------//
	void ShaderSource::_GetFileName(void)
	{
		m_fileName = m_name;
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::_Load(void)
	{
		if (!m_loaded)
			return Resource::_Load();

		double _st = TimeMs();
		bool _r = _ProcessSource();
		if (m_trackingEnabled)
		{
			double _t = TimeMs() - _st;
			LOG_DEBUG("Load %s '%s', %.2f ms", ClassName().c_str(), m_name.c_str(), _t);
		}
		return _r;
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::_Load(File& _f)
	{
		if (_f)
		{
			_SetSource(_f.AsString());
			return _ProcessSource();
		}

		_SetSource("");
		_ProcessSource();
		m_errors = "> " + m_name + ": Couldn't open file\n";
		m_loaded = true;
		m_valid = false;
		LOG_ERROR("Couldn't load %s '%s'", ClassName().c_str(), m_name.c_str());

		return false;
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::_IsIncluded(ShaderSource* _include)
	{
		assert(_include != nullptr);
		for (ShaderSource* i : m_includes)
		{
			if (i == _include || i->_IsIncluded(_include))
				return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void ShaderSource::_Invalidate(void)
	{
		if (m_processed)
		{
			m_processed = false;
			gResourceCache->GetManager<ShaderSource>()->AddResourceForReload(this);
			for (ShaderSource* i : m_dependents)
				i->_Invalidate();

			for (auto& i : m_instances)
				i.second->_Invalidate();
		}
	}
	//----------------------------------------------------------------------------//
	void ShaderSource::_SetSource(const String& _src)
	{
		for (ShaderSource* i : m_includes)
			i->m_dependents.erase(this);
		m_includes.clear();

		m_rawSource = _src;
		m_source.clear();
		m_loaded = true;
		m_processed = false;
		m_checksum = 0;
		m_errors.clear();
		m_valid = true;	// reset invalid flag

		for (ShaderSource* i : m_dependents)
			i->_Invalidate();

		for (auto& i : m_instances)
			i.second->_Invalidate();
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::_ProcessSource(void)
	{
		if (m_processed)
			return m_valid;
		m_processed = true;

		m_errors = _Parse(m_rawSource.c_str());
		m_checksum = StrHash(m_source);

#if defined(_DEBUG) && 1
		String _name = "Data/Shaders/_Processed/" + StrReplace(m_name, ":\\/", '_');
		File _f = gFileSystem->WriteFile(_name);
		if (_f)
		{
			_f.Write(m_source.c_str(), (uint)m_source.length());
			//LOG_DEBUG("Processed ShaderSource '%s' saved to '%s'", m_name.c_str(), _name.c_str());
		}
#endif

		if (!m_errors.empty())
		{
			LOG_ERROR("ShaderSource '%s' was processed with errors:\n%s", m_name.c_str(), m_errors.c_str());
			m_valid = false; // set invalid flag
			return false;
		}

		m_valid = true;	// reset invalid flag
		return true;
	}
	//----------------------------------------------------------------------------//
	String ShaderSource::_Parse(const char* s)
	{
		if (!s || !s[0])
			return ""; // empty source

		uint l = 1;
		m_source = StrFormat("#line %d %d // %s\n", l, m_nameId, m_name.c_str());

		while (*s)
		{
			if (strchr("\n\r", *s)) // new line
			{
				if (s[0] == '\r' && s[1] == '\n')
					++s;
				++s;
				++l;
				m_source += "\n";
			}
			else if (*s == '/' && strchr("/*", s[1])) // comment
			{
				const char* _start = s;

				if (s[1] == '/') // line
				{
					s += 2;
					while (*s && !strchr("\n\r", *s))
						++s;
				}
				else // multiline
				{
					s += 2;
					while (*s && !(s[0] == '*' && s[1] == '/'))
					{
						if (strchr("\n\r", *s)) // new line
						{
							if (s[0] == '\r' && s[1] == '\n')
								++s;
							++l;
						}
						++s;
					}
					if (!*s)
						return StrFormat("> %s(%d): Unexpected End of file in multiline comment\n", m_name.c_str(), l);
					s += 2;
				}

				m_source.append(_start, s);
			}
			else if (*s == '#')	// preprocessor
			{
				const char* _start = s;
				++s;
				while (*s && strchr(" \t", *s))
					++s;

				if (strncmp(s, "include", 7) == 0)
				{
					s += 7;
					while (*s && strchr(" \t", *s))
						++s;

					String _name;
					while (*s && !strchr("\n\r", *s) && !(*s == '/' && strchr("/*", s[1])))
						_name += *s++;
					_name = StrTrim(_name, " \t\"");

					ShaderSourcePtr _inc = gResourceCache->LoadResource<ShaderSource>(_name);
					if (!_inc)
						return StrFormat("> %s(%d): File '%s' not was found\n", m_name.c_str(), l, _name.c_str());

					if (!_inc || _inc == this || _inc->_IsIncluded(this))
						return StrFormat("> %s(%d): Unable to include file '%s'\n", m_name.c_str(), l, _name.c_str());
					
					m_includes.insert(_inc);
					_inc->m_dependents.insert(this);

					_inc->Touch(); // load now
					if (!_inc->_ProcessSource())
					{
						return StrFormat("> %s(%d): Invalid included file '%s'\n", m_name.c_str(), l, _name.c_str()) + _inc->m_errors;
					}

					m_source += _inc->m_source;
					m_source += StrFormat("\n#line %d %d // %s\n", l, m_nameId, m_name.c_str());
				}
				else
				{
					m_source.append(_start, s);
				}
			}
			else
				m_source += *s++;
		}
		return ""; // no error
	}
	//----------------------------------------------------------------------------//
	uint16 ShaderSource::_AddShaderName(const String& _name)
	{
		uint _hash = NameHash(_name);

		auto _exists = s_nameIndices.find(_hash);
		if (_exists != s_nameIndices.end())
			return _exists->second;

		if (s_names.size() >= 0xffff)
		{
			LOG_ERROR("Too many shader names");
			assert(s_names.size() < 0xffff);
		}

		uint16 _id = (uint16)s_names.size();
		s_nameIndices[_hash] = _id;
		s_names.push_back(_name);

		return _id;
	}
	//----------------------------------------------------------------------------//
	bool ShaderSource::_GetShaderName(uint16 _id, String& _name)
	{
		if (_id < s_names.size())
		{
			_name = s_names[_id];
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	String ShaderSource::_ParseGLSLLog(const char* _log)
	{
		String _r, _name;
		if (*_log && !strchr("\n\r", *_log))
			_r += "> ";
		while (*_log)
		{
			const char* _start = _log;
			while (*_log && !strchr("0123456789\n\r", *_log))
				++_log;
			_r.append(_start, _log);

			if (*_log >= '0' && *_log <= '9') // amd = file:line, nvidia = file(line)
			{
				_start = _log;
				while (*_log && *_log >= '0' && *_log <= '9')
					++_log;
				if (*_log == ':' || *_log == '(')
				{
					bool _isNVidiaLog = *_log == '(';
					uint _id = 0;
					sscanf(_start, "%u", &_id);
					++_log;
					if (_GetShaderName(_id, _name))
					{
						const char* _start2 = _log;
						while (*_log && *_log >= '0' && *_log <= '9')
							++_log;
						if (_start2 != _log)
						{
							_r.append(_name);
							_r.append("(");
							_r.append(_start2, _log);
							_r.append(")");
						}
						else
							_r.append(_start, _log);
					}
					else
						_r.append(_start, _log);
					if (_isNVidiaLog && *_log == ')')
						++_log;
				}
				else
					_r.append(_start, _log);
			}
			else if (*_log == '\n' || *_log == '\r')
			{
				if (_log[0] == '\r' && _log[1] == '\n')
					++_log;
				_r.push_back(*_log++);
				if(*_log)
					_r.append("> ");
			}
		}
		if (_log[-1] == '\n' && _log[-2] == '\n' && _r.size() > 1)
			_r.resize(_r.size() - 1);
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Shader
	//----------------------------------------------------------------------------//
	
	const uint ShaderType2GL[] =
	{
		GL_VERTEX_SHADER, // ST_Vertex
		GL_FRAGMENT_SHADER, // ST_Fragment
		GL_GEOMETRY_SHADER, // ST_Geometry
	};

	const char* ShaderCacheTags[]=
	{
		"VSHC", // ST_Vertex
		"FSHC", // ST_Fragment
		"GSHC", // ST_Geometry
	};

	bool Shader::s_cacheLoaded = false;
	bool Shader::s_cacheChanged = false;
	HashMap<uint, Shader::CacheItem> Shader::s_cache;
	HashSet<Shader*> Shader::s_uncompiledShaders;

	//----------------------------------------------------------------------------//
	Shader::Shader(ShaderType _type, ShaderSource* _src, uint _defs, const String& _name, uint _uid) :
		m_name(_name),
		m_type(_type),
		m_source(_src),
		m_uid(_uid),
		m_defs(_defs),
		m_handle(0),
		m_compiled(false),
		m_valid(false)
	{
		m_handle = glCreateProgram();
		glProgramParameteri(m_handle, GL_PROGRAM_SEPARABLE, 1);
		glProgramParameteri(m_handle, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, 1);
		_Compile();
	}
	//----------------------------------------------------------------------------//
	Shader::~Shader(void)
	{
		// ...
		m_source->m_instances.erase(m_uid);
		s_uncompiledShaders.erase(this);
	}
	//----------------------------------------------------------------------------//
	void Shader::_Invalidate(void)
	{
		if (m_compiled)
		{
			m_compiled = false;
			s_uncompiledShaders.insert(this);
		}
	}
	//----------------------------------------------------------------------------//
	bool Shader::_Compile(void)
	{
		if (m_compiled)
			return m_valid;
		m_compiled = true;
		s_uncompiledShaders.erase(this);

		double _st = TimeMs();

		m_valid = true;

		// bind vertex input
		for (uint i = 0; i < VS_MaxAttribs; ++i)
			glBindAttribLocation(m_handle, i, VertexAttribNames[i]);

		// compile
		int _status, _length;
		bool _createdFromCache = false;
		CacheItem _bin;

		if (_GetCacheItem(m_uid, m_source->m_checksum, _bin))
		{
			_createdFromCache = true;
			glProgramBinary(m_handle, _bin.format, _bin.data, _bin.size);
		}
		else
		{
			String _src = "#version 330\n#define COMPILE_" + String(ShaderTypePrefix[m_type]) + "\n";

			/*if (!ogl_IsVersionGEQ(4, 1))
			{
				_src += "#extension GL_ARB_separate_shader_objects : enable\n";
			}

			if (!ogl_IsVersionGEQ(4, 4))
			{
				_src += "#extension GL_ARB_enhanced_layouts : enable\n";
			}*/

			const ShaderDefines& _defs = ShaderDefines::GetUnique(m_defs);
			_src += _defs.BuildText();
			_src += m_source->m_source;

			const char* _srcv = _src.c_str();
			uint _shader = glCreateShader(ShaderType2GL[m_type]);
			glShaderSource(_shader, 1, &_srcv, nullptr);
			glCompileShader(_shader);
			glGetShaderiv(_shader, GL_COMPILE_STATUS, &_status);
			glAttachShader(m_handle, _shader);
			glLinkProgram(m_handle);
			glDetachShader(m_handle, _shader);

			if (!_status)
			{
				m_valid = false;

				glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &_length);
				if (_length > 0)
				{
					Array<char> _buff(_length + 1);
					glGetShaderInfoLog(_shader, _length, &_length, &_buff[0]);
					_buff[_length] = 0;
					String _log = ShaderSource::_ParseGLSLLog(&_buff[0]);

					LOG_ERROR("Shader '%s' (%s@%08x) was compiled with errors:\n%s", m_source->m_name.c_str(), ShaderTypePrefix[m_type], m_defs, _log.c_str());
				}
			}
			glDeleteShader(_shader);
		}

		glGetProgramiv(m_handle, GL_COMPILE_STATUS, &_status);
		if (!_status)
		{
			if (m_valid && _createdFromCache) // log not yet was written
			{
				glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &_length);
				if (_length > 0)
				{
					Array<char> _buff(_length + 1);
					glGetProgramInfoLog(m_handle, _length, &_length, &_buff[0]);
					_buff[_length] = 0;
					String _log = ShaderSource::_ParseGLSLLog(&_buff[0]);

					LOG_ERROR("Shader '%s' (%s@%08x) was compiled with errors:\n%s", m_source->m_name.c_str(), ShaderTypePrefix[m_type], m_defs, _log.c_str());
				}
			}

			m_valid = false;
		}


		if(!_createdFromCache)
		{
			int _size = 0;
			uint _format = (uint)-1;
			glGetProgramiv(m_handle, GL_PROGRAM_BINARY_LENGTH, &_size);
			uint8* _data = new uint8[_size];
			glGetProgramBinary(m_handle, _size, &_size, &_format, &_data[0]);
			_SetCacheItem(m_uid, m_source->m_checksum, _format, _size, &_data[0]);
		}

		LOG_DEBUG("Compile Shader '%s', from %s, %.2f ms", m_name.c_str(), _createdFromCache ? "cache" : "source", TimeMs() - _st);

		return m_valid;
	}
	//----------------------------------------------------------------------------//
	void Shader::_Reflect(void)
	{
		// get uniforms

		Array<char> _name;
		int _count = 0, _length = 0, _currentProg = 0, _size;
		uint _type;

		glGetIntegerv(GL_CURRENT_PROGRAM, &_currentProg);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &_count);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &_length);
		_name.resize(_length + 1);
		for (int i = 0; i < _count; ++i)
		{
			glGetActiveUniform(m_handle, i, _name.size(), &_length, &_size, &_type, &_name[0]);
			_name[_length] = 0;

			ShaderParam _param;

			switch (_type)
			{
			case GL_INT:
				_param.type = SPT_Int;
				_param.size = sizeof(int);
				break;
			case GL_INT_VEC2:
				_param.type = SPT_Vec2i;
				_param.size = sizeof(int) * 2;
				break;
			case GL_INT_VEC3:
				_param.type = SPT_Vec3i;
				_param.size = sizeof(int) * 3;
				break;
			case GL_INT_VEC4:
				_param.type = SPT_Vec4i;
				_param.size = sizeof(int) * 4;
				break;
			case GL_FLOAT:
				_param.type = SPT_Float;
				_param.size = sizeof(float);
				break;
			case GL_FLOAT_VEC2:
				_param.type = SPT_Vec2;
				_param.size = sizeof(float) * 2;
				break;
			case GL_FLOAT_VEC3:
				_param.type = SPT_Vec3;
				_param.size = sizeof(float) * 3;
				break;
			case GL_FLOAT_VEC4:
				_param.type = SPT_Vec4;
				_param.size = sizeof(float) * 4;
				break;
			case GL_FLOAT_MAT4:
				_param.type = SPT_Mat44;
				_param.size = sizeof(float) * 16;
				break;
			case GL_FLOAT_MAT3x4:
				_param.type = SPT_Mat34;
				_param.size = sizeof(float) * 12;
				break;

			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_SHADOW:
			case GL_INT_SAMPLER_2D:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_INT_SAMPLER_2D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_SAMPLER_3D:
			case GL_INT_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_INT_SAMPLER_CUBE:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_SAMPLER_BUFFER:
			case GL_INT_SAMPLER_BUFFER:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				_param.type = SPT_Texture;
				break;

			} // switch (_type)

			if (_param.type == SPT_Texture)	// sampler
			{
				_param.name = &_name[0];
				_param.location = glGetUniformLocation(m_handle, &_name[0]);
				_param.count = 1;
				_param.size = 0;
				_param.slot = (uint)m_textures.size();
				m_textures[_param.name] = (uint)m_params.size();
				m_names[_param.name] = (uint)m_params.size();
				m_params.push_back(_param);
				glProgramUniform1i(m_handle, _param.location, _param.slot);
			}
			else if (_param.type != SPT_Unknown) // uniform
			{
				char* _array = strchr(&_name[0], '[');
				if (_array)
					*_array = 0;
				_param.location = glGetUniformLocation(m_handle, &_name[0]);
				_param.name = &_name[0];
				_param.count = _size;
				m_names[_param.name] = (uint)m_params.size();
				m_params.push_back(_param);
			}
			else // unknown
			{
				// m_valid = false;
				//m_log += StrFormat("Unknown parameter \"%s\" (type = 0x%04x)\n", &_name[0], _type);
			}
		}

		// get uniform blocks

		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCKS, &_count);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &_length);
		_name.resize(_length + 1);
		for (int i = 0; i < _count; ++i)
		{
			glGetActiveUniformBlockName(m_handle, i, _name.size(), &_length, &_name[0]);
			_name[_length] = 0;
			char* _array = strchr(&_name[0], '[');
			if (_array)
				*_array = 0;

			ShaderParam _param;
			_param.name = &_name[0];
			_param.type = SPT_Buffer;
			_param.count = 1;
			_param.size = 0; //TODO: init _param.size
			_param.slot = 0; //TODO: init _param.slot
			_param.location = glGetUniformBlockIndex(m_handle, &_name[0]);
			m_buffers[_param.name] = m_buffers.size();
			m_params.push_back(_param);
		}

		/*
		uCameraSlot = shader->GetSlot("uCamera");
		uMatrixBufferSlot = shader->GetSlot("uWorldMatrices");
		uDiffuseMapSlot = shader->GetSlot("uDiffuseMap");
		gRenderContext->SetUniformBuffer(ST_Vertex, uCameraSlot, m_cameraParams);

		gRenderContext->SetTexture(ST_Fragment, _fs->GetTextureSlot("uDiffuseMap"), _material->GetDiffuseTexture()); 
		*/

	}
	//----------------------------------------------------------------------------//
	bool Shader::_InitCache(void)
	{
		s_cacheLoaded = false;
		s_cacheChanged = false;
		return true;
	}
	//----------------------------------------------------------------------------//
	void Shader::_DestroyCache(void)
	{
		_SaveCache();

		for (const auto& i : s_cache)
		{
			delete[] i.second.data;
		}
		s_cache.clear();
	}
	//----------------------------------------------------------------------------//
	void Shader::_LoadCache(void)
	{
		if (s_cacheLoaded)
			return;
		assert(s_cache.empty());
		s_cacheLoaded = true;

		double _st = TimeMs();

		String _cacheName = gFileSystem->FindFile("Shaders/ShaderCache.bin"); // todo:
		if (_cacheName.empty())
			return; 

		File _f = gFileSystem->ReadFile(_cacheName);
		if (!_f)
		{
			LOG_ERROR("Couldn't load ShaderCache : IO error");
			return;
		}

		char _tag[4];
		uint _driverId, _numItems, _maxSize, _id, _totalSize = 0;
		_f.Read(_tag, 4);
		_f.Read(&_driverId, 4);
		_f.Read(&_maxSize, 4);
		_f.Read(&_numItems, 4);
		if (_f.Tell() != 16 || strncmp(_tag, "SHCH", 4) || _maxSize > (_f.Size() - 16)) // empty or corrupted file
		{
			LOG_ERROR("Couldn't load ShaderCache : Invalid file");
			return;
		}

		if (_driverId != 0) // todo:
		{
			LOG_EVENT("Load ShaderCache: out of date");
			return;
		}

		uint8* _tempBuffer = new uint8[_maxSize];
		HashMap<uint, CacheItem> _cache;
		for (uint i = 0; i < _numItems; ++i)
		{
			CacheItem _item;
			_id = 0;
			_f.Read(&_id, 4);
			_f.Read(&_item.checksum, 4);
			_f.Read(&_item.format, 4);
			_f.Read(&_item.size, 4);
			uint _readed = _f.Read(_tempBuffer, _item.size);
			if (_item.size > _maxSize || _cache.find(_id) != _cache.end() || _readed != _item.size)
			{
				LOG_ERROR("Couldn't load ShaderCache : Invalid file");
				delete[]_tempBuffer;
				return;
			}

			_item.data = new uint8[_item.size];
			memcpy(_item.data, _tempBuffer, _item.size);
			_cache[_id] = _item;
			_totalSize += _item.size;
		}
		delete[]_tempBuffer;

		s_cache = std::move(_cache);
		LOG_EVENT("Load ShaderCache: %u shaders, %u bytes, %.2f ms", (uint)s_cache.size(), _totalSize, TimeMs() - _st);
	}
	//----------------------------------------------------------------------------//
	void Shader::_SaveCache(void)
	{
		if (!s_cacheLoaded || !s_cacheChanged)
			return;

		double _st = TimeMs();

		File _f = gFileSystem->WriteFile("Data/Shaders/ShaderCache.bin"); // todo:
		if (!_f)
		{
			LOG_ERROR("Couldn't ShaderCache");
			return;
		}

		s_cacheChanged = false;

		uint _totalSize = 0, _maxSize = 0, _numItems = (uint)s_cache.size(), _driverId = 0;
		for (const auto& i : s_cache)
		{
			if (i.second.size > _maxSize)
				_maxSize = i.second.size;
			_totalSize += i.second.size;
		}

		_f.Write("SHCH", 4);
		_f.Write(&_driverId, 4); // todo:
		_f.Write(&_maxSize, 4);
		_f.Write(&_numItems, 4);
		
		for (const auto& i : s_cache)
		{
			_f.Write(&i.first, 4);
			_f.Write(&i.second.checksum, 4);
			_f.Write(&i.second.format, 4);
			_f.Write(&i.second.size, 4);
			_f.Write(i.second.data, i.second.size);
		}

		_f.Flush();

		LOG_EVENT("Save ShaderCache: %u shaders, %u bytes, %.2f ms", (uint)s_cache.size(), _totalSize, TimeMs() - _st);
	}
	//----------------------------------------------------------------------------//
	bool Shader::_GetCacheItem(uint _id, uint _checksum, CacheItem& _item)
	{
		_LoadCache();

		auto _exists = s_cache.find(_id);
		if (_exists != s_cache.end() && _exists->second.checksum == _checksum && _exists->second.size > 0)
		{
			_item = _exists->second;
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void Shader::_SetCacheItem(uint _id, uint _checksum, uint _format, uint _size, uint8* _data)
	{
		s_cacheChanged = true;

		auto _exists = s_cache.find(_id);
		if (_exists != s_cache.end())
		{
			delete[] _exists->second.data;
			s_cache.erase(_id);
		}
		
		CacheItem _item;
		_item.checksum = _checksum;
		_item.format = _format;
		_item.size = _size;
		_item.data = _data;
		s_cache[_id] = _item;
	}
	//----------------------------------------------------------------------------//

#if 0
	Array<String> Shader::s_names;
	HashMap<uint, uint16> Shader::s_nameIndices;
	HashMap<uint, ShaderPtr> Shader::s_cache;

	//----------------------------------------------------------------------------//
	Shader::Shader(const String& _name, uint _uid) :
		m_name(_name),
		m_nameId(0),
		m_uid(_uid),
		m_valid(true),
		m_processed(true),
		m_loaded(false),
		m_fileTime(-1),
		m_lastTime(-1),
		m_log("Source code is empty\n")
	{
		m_nameId = _AddName(m_name);
		_Load();
	}
	//----------------------------------------------------------------------------//
	Shader::~Shader(void)
	{
		m_includes.clear();
	}
	//----------------------------------------------------------------------------//
	void Shader::_Invalidate(void)
	{
		if (m_processed)
		{
			m_processed = false;
			for (Shader* i : m_dependents)
				i->_Invalidate();
		}
	}
	//----------------------------------------------------------------------------//
	bool Shader::_Load(bool _checkFileTime)
	{
		if (m_loaded && !_checkFileTime)
			return false; // already loaded

		m_loaded = true;

		String _fileName = "Shaders/" + m_name;
		time_t _ft = gFileSystem->FileTime(_fileName);
		if (_ft == m_fileTime)
			return true; // already loaded

		m_fileTime = _ft;
		m_lastTime = _ft;
		File _f = gFileSystem->ReadFile(_fileName);

		for (Shader* i : m_includes)
			i->m_dependents.erase(this);
		m_includes.clear();

		m_rawSource = _f.AsString();
		m_source.clear();
		m_processed = false;
		m_log = "Source code not was processed\n";
		m_valid = true;

		for (Shader* i : m_dependents)
			i->_Invalidate();

		if (!_f)
		{
			m_log = "File '" + m_name + "' not was found\n";
			m_processed = true;
			m_valid = false;
		}

		return true; // loaded now
	}
	//----------------------------------------------------------------------------//
	bool Shader::_IsIncluded(Shader* _include)
	{
		assert(_include != nullptr);
		for (Shader* i : m_includes)
		{
			if (i == _include || i->_IsIncluded(_include))
				return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool Shader::_LogError(const char* _err, ...)
	{
		va_list _args;
		va_start(_args, _err);
		m_log += StrFormatV(_err, _args);
		va_end(_args);
		return false;
	}
	//----------------------------------------------------------------------------//
	bool Shader::_Parse(void)
	{
		if (m_processed)
			return m_valid;
		m_processed = true;

		m_log.clear();
		m_source.clear();
		m_valid = false;

		const char* s = m_rawSource.c_str();
		uint l = 1;

		if (!s || !s[0])
		{
			m_valid = true;
			m_log = "Source code is empty\n";
			return true;
		}

		m_source += StrFormat("#line %d %d // %s\n", l, m_nameId, m_name.c_str());

		while (*s)
		{
			if (strchr("\n\r", *s)) // new line
			{
				if (s[0] == '\r' && s[1] == '\n')
					++s;
				++s;
				++l;
				m_source += "\n";
			}
			else if (*s == '/' && strchr("/*", s[1])) // comment
			{
				const char* _start = s;

				if (s[0] == '/' && s[1] == '/')	// line
				{
					s += 2;
					while (*s && !strchr("\n\r", *s))
						++s;
				}
				else if (s[0] == '*' && s[1] == '/') // multiline
				{
					s += 2;
					while (*s && !(s[0] == '*' && s[1] == '/'))
					{
						if (strchr("\n\r", *s)) // new line
						{
							if (s[0] == '\r' && s[1] == '\n')
								++s;
							++l;
						}
						++s;
					}
					if (!*s)
						return _LogError("%s(%d): Error: Unexpected End of file in multiline comment\n", m_name.c_str(), l);
					s += 2;
				}

				m_source.append(_start, s);
			}
			else if (*s == '#')	// preprocessor
			{
				const char* _start = s;
				++s;
				while (*s && strchr(" \t", *s))
					++s;

				if (strncmp(s, "include", 7) == 0)
				{
					s += 7;
					while (*s && strchr(" \t", *s))
						++s;

					String _name;
					while (*s && !strchr("\n\r", *s) && !(*s == '/' && strchr("/*", *s)))
						_name += *s++;
					_name = StrTrim(_name, " \t\"");

					ShaderPtr _inc = _Create(_name);
					if (!_inc)
						return _LogError("%s(%d): Error: File '%s' not was found\n", m_name.c_str(), l, _name.c_str());

					if (!_inc || _inc == this || _inc->_IsIncluded(this))
						return _LogError("%s(%d): Error: Unable to include file '%s'\n", m_name.c_str(), l, _name.c_str());

					m_includes.insert(_inc);
					_inc->m_dependents.insert(this);

					if (m_lastTime < _inc->m_fileTime)
						m_lastTime = _inc->m_fileTime;

					if (!_inc->_Parse())
					{
						m_log += StrFormat("%s(%d): Error: Invalid included file '%s'\n", m_name.c_str(), l, _name.c_str());
						m_log += _inc->m_log;
						return false;
					}

					if (m_lastTime < _inc->m_lastTime)
						m_lastTime = _inc->m_lastTime;

					m_source += _inc->m_source; // include common section
					m_source += StrFormat("\n#line %d %d // %s\n", l, m_nameId, m_name.c_str());
				}
				else
				{
					m_source.append(_start, s);
				}
			}
			else
				m_source += *s++;
		}

#if 1 // dump 
		File _f = gFileSystem->WriteFile(StrFormat("Cache/Shaders/%08x.txt", m_uid));
		_f.Write(m_source.c_str(), m_source.length());
		_f.Flush();
#endif

		m_valid = true;
		return true;
	}
	//----------------------------------------------------------------------------//
	ShaderInstancePtr Shader::_AddInstance(ShaderType _type, uint _defs)
	{
		auto _exists = m_instances.find(_defs);
		if (_exists != m_instances.end())
			return _exists->second;

		String _name = m_name + StrFormat("@%s@08x", ShaderTypePrefix[_type], _defs);
		uint _uid = NameHash(_name);

		ShaderInstancePtr _instance = new ShaderInstance(this, _type, _defs, _name, _uid);
		m_instances[_uid] = _instance;

		return _instance;
	}
	//----------------------------------------------------------------------------//
	void Shader::_RemoveInstance(uint _defs)
	{
		m_instances.erase(_defs);
	}
	//----------------------------------------------------------------------------//
	bool Shader::_InitMgr(void)
	{
		return true;
	}
	//----------------------------------------------------------------------------//
	void Shader::_DestroyMgr(void)
	{

		// ...

	}
	//----------------------------------------------------------------------------//
	Shader* Shader::_Create(const String& _name)
	{
		if (_name.empty())
			return nullptr;

		uint _hash = NameHash(_name);

		auto _exists = s_cache.find(_hash);
		if (_exists != s_cache.end())
			return _exists->second;

		ShaderPtr _shader = new Shader(_name, _hash);
		s_cache[_hash] = _shader;

		_shader->_Load(false);

		return _shader;
	}
	//----------------------------------------------------------------------------//
	uint16 Shader::_AddName(const String& _name)
	{
		uint _hash = NameHash(_name);

		auto _exists = s_nameIndices.find(_hash);
		if (_exists != s_nameIndices.end())
			return _exists->second;

		if (s_names.size() >= 0xffff)
		{
			LOG_ERROR("Too many shader names");
			assert(s_names.size() < 0xffff);
		}

		uint16 _id = (uint16)s_names.size();
		s_nameIndices[_hash] = _id;
		s_names.push_back(_name);

		return _id;
	}
	//----------------------------------------------------------------------------//
	bool Shader::_GetName(String& _name, uint16 _id)
	{
		auto _exists = s_nameIndices.find(_id);
		if (_exists != s_nameIndices.end())
		{
			_name = s_names[_exists->second];
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	String Shader::_ParseGLSLLog(const char* _log)
	{
		String _r, _name;
		while (*_log)
		{
			const char* _start = _log;
			while (*_log && (*_log < '0' || *_log > '9'))
				++_log;
			_r.append(_start, _log);
			if (*_log >= '0' && *_log <= '9') // amd = file:line, nvidia = file(line)
			{
				_start = _log;
				while (*_log && *_log >= '0' && *_log <= '9')
					++_log;
				if (*_log == ':' || *_log == '(')
				{
					bool _isNVidiaLog = *_log == '(';
					uint _id = 0;
					sscanf(_start, "%u", &_id);
					++_log;
					if (_GetName(_name, _id))
					{
						const char* _start2 = _log;
						while (*_log && *_log >= '0' && *_log <= '9')
							++_log;
						if (_start2 != _log)
						{
							_r.append(_name);
							_r.append("(");
							_r.append(_start2, _log);
							_r.append(")");
						}
						else
							_r.append(_start, _log);
					}
					else
						_r.append(_start, _log);
					if (_isNVidiaLog && *_log == ')')
						++_log;
				}
				else
					_r.append(_start, _log);
			}
		}
		if (_log[-1] == '\n' && _log[-2] == '\n' && _r.size() > 1)
			_r.resize(_r.size() - 1);
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ShaderInstance
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ShaderInstance::ShaderInstance(Shader* _shader, ShaderType _type, uint _defs, const String& _name, uint _uid) :
		m_shader(_shader),
		m_type(_type),
		m_valid(false),
		m_compiled(false),
		m_inCompile(false),
		m_uid(_uid),
		m_defs(_defs),
		m_handle(0)
	{
		m_handle = glCreateShader(ShaderType2GL[_type]);
		_Compile();
	}
	//----------------------------------------------------------------------------//
	ShaderInstance::~ShaderInstance(void)
	{
		m_shader->_RemoveInstance(m_defs);
		glDeleteShader(m_handle);
	}
	//----------------------------------------------------------------------------//
	void ShaderInstance::_Invalidate(void)
	{
		if (m_compiled)
		{
			m_compiled = false;
			m_inCompile = false;
			// ...
		}
	}
	//----------------------------------------------------------------------------//
	void ShaderInstance::_Compile(void)
	{
		if (m_compiled)
			return;
		
		//todo: invalidate programs

		m_compiled = true;
		m_inCompile = true;
		
		String _src = "version 330\n#define COMPILE_" + String(ShaderTypePrefix[m_type]) + "\n";
		const ShaderDefines& _defs = ShaderDefines::GetUnique(m_defs);
		_src += _defs.BuildText();
		_src += m_shader->GetSource();

		const char* _srcv = _src.c_str();
		glShaderSource(m_handle, 1, &_srcv, nullptr);
	}
	//----------------------------------------------------------------------------//
	bool ShaderInstance::_Wait(void)
	{
		_Compile();
		if (m_compiled && m_inCompile)
		{
			m_inCompile = false;
			int _status = 0;
			glGetShaderiv(m_handle, GL_COMPILE_STATUS, &_status);
			m_valid = _status != 0;
			if (!m_valid)
			{
				// todo: log errors
			}
		}
		return m_valid;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ShaderProgram
	//----------------------------------------------------------------------------//
	
	HashMap<uint, ShaderProgram*> ShaderProgram::s_cache;

	//----------------------------------------------------------------------------//
	ShaderProgram::ShaderProgram(uint _uid, ShaderInstance* _vs, ShaderInstance* _fs, ShaderInstance* _gs, uint _outputAttribs) :
		m_uid(_uid),
		m_outputAttribs(_outputAttribs),
		m_handle(0),
		m_linked(false),
		m_valid(false)
	{
		m_handle = glCreateProgram();
		if (_vs)
		{
			m_stages[ST_Vertex] = _vs;
			glAttachShader(m_handle, _vs->m_handle);
		}
		if (_fs)
		{
			m_stages[ST_Fragment] = _fs;
			glAttachShader(m_handle, _fs->m_handle);
		}
		if (_gs)
		{
			m_stages[ST_Geometry] = _gs;
			glAttachShader(m_handle, _gs->m_handle);
		}
		_Link();
	}
	//----------------------------------------------------------------------------//
	ShaderProgram::~ShaderProgram(void)
	{
		glDeleteProgram(m_handle);
	}
	//----------------------------------------------------------------------------//
	void ShaderProgram::_Link(void)
	{
		if (m_linked)
			return;
		m_linked = true;
		m_valid = false;
		
		//m_params.clear();
		m_log.clear();

		// compile shaders
		for (uint i = 0; i < 3; ++i)
		{
			ShaderInstance* _st = m_stages[i];
			if (_st)
			{
				_st->_Wait();
				if (!_st->m_valid)
				{
					switch (i)
					{
					case ST_Vertex:
						m_log += "Vertex shader was compiled with errors\n";
						break;
					case ST_Fragment:
						m_log += "Fragment shader was compiled with errors\n";
						break;
					case ST_Geometry:
						m_log += "Geometry shader was compiled with errors\n";
						break;
					}
					return;
				}
			}
		}

		// bind vertex input
		for (uint i = 0; i < VS_MaxAttribs; ++i)
			glBindAttribLocation(m_handle, i, VertexAttribNames[i]);


		// link program
		int _status = 0;
		glLinkProgram(m_handle);
		glGetProgramiv(m_handle, GL_LINK_STATUS, &_status);
		if(!_status)
		{

			// handle errors
			return;
		}


		// bind vertex output
		/*if (m_output != VT_Empty)
		{
			const GLVertexFormat& _format = GLVertexFormats[(int)m_output];
			vector<const char*> _names;
			for (size_t i = 0; i < _format.numAttribs; ++i)
				_names.push_back(GLTransformFeedbackNames[_format.attribs[i].index]);
			glTransformFeedbackVaryings(m_handle, _names.size(), ArrayPtr(_names), GL_INTERLEAVED_ATTRIBS);
		}*/

		// get uniforms

		Array<char> _name;
		int _count = 0, _length = 0, _currentProg = 0, _size;
		uint _type;

		glGetIntegerv(GL_CURRENT_PROGRAM, &_currentProg);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &_count);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &_length);
		_name.resize(_length + 1);
		for (int i = 0; i < _count; ++i)
		{
			glGetActiveUniform(m_handle, i, _name.size(), &_length, &_size, &_type, &_name[0]);
			_name[_length] = 0;

			ShaderParam _param;

			switch (_type)
			{
			case GL_INT:
				_param.type = SPT_Int;
				_param.size = sizeof(int);
				break;
			case GL_INT_VEC2:
				_param.type = SPT_Vec2i;
				_param.size = sizeof(int) * 2;
				break;
			case GL_INT_VEC3:
				_param.type = SPT_Vec3i;
				_param.size = sizeof(int) * 3;
				break;
			case GL_INT_VEC4:
				_param.type = SPT_Vec4i;
				_param.size = sizeof(int) * 4;
				break;
			case GL_FLOAT:
				_param.type = SPT_Float;
				_param.size = sizeof(float);
				break;
			case GL_FLOAT_VEC2:
				_param.type = SPT_Vec2;
				_param.size = sizeof(float) * 2;
				break;
			case GL_FLOAT_VEC3:
				_param.type = SPT_Vec3;
				_param.size = sizeof(float) * 3;
				break;
			case GL_FLOAT_VEC4:
				_param.type = SPT_Vec4;
				_param.size = sizeof(float) * 4;
				break;
			case GL_FLOAT_MAT4:
				_param.type = SPT_Mat44;
				_param.size = sizeof(float) * 16;
				break;
			case GL_FLOAT_MAT3x4:
				_param.type = SPT_Mat34;
				_param.size = sizeof(float) * 12;
				break;

			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_SHADOW:
			case GL_INT_SAMPLER_2D:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_INT_SAMPLER_2D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_SAMPLER_3D:
			case GL_INT_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_INT_SAMPLER_CUBE:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_SAMPLER_BUFFER:
			case GL_INT_SAMPLER_BUFFER:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				_param.type = SPT_Texture;
				break;

			} // switch (_type)

			if (_param.type == SPT_Texture)	// sampler
			{
				_param.name = &_name[0];
				_param.location = glGetUniformLocation(m_handle, &_name[0]);
				_param.count = 1;
				_param.size = 0;
				_param.slot = (uint)m_textures.size();
				m_textures[_param.name] = (uint)m_params.size();
				m_names[_param.name] = (uint)m_params.size();
				m_params.push_back(_param);
				glProgramUniform1iEXT(m_handle, _param.location, _param.slot);
			}
			else if (_param.type != SPT_Unknown) // uniform
			{
				char* _array = strchr(&_name[0], '[');
				if (_array)
					*_array = 0;
				_param.location = glGetUniformLocation(m_handle, &_name[0]);
				_param.name = &_name[0];
				_param.count = _size;
				m_names[_param.name] = (uint)m_params.size();
				m_params.push_back(_param);
			}
			else // unknown
			{
				// m_valid = false;
				m_log += StrFormat("Unknown parameter \"%s\" (type = 0x%04x)\n", &_name[0], _type);
			}
		}

		// get uniform blocks

		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCKS, &_count);
		glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &_length);
		_name.resize(_length + 1);
		for (int i = 0; i < _count; ++i)
		{
			glGetActiveUniformBlockName(m_handle, i, _name.size(), &_length, &_name[0]);
			_name[_length] = 0;
			char* _array = strchr(&_name[0], '[');
			if (_array)
				*_array = 0;

			ShaderParam _param;
			_param.name = &_name[0];
			_param.type = SPT_Buffer;
			_param.count = 1;
			_param.size = 0; //TODO: init _param.size
			_param.slot = 0; //TODO: init _param.slot
			_param.location = glGetUniformBlockIndex(m_handle, &_name[0]);
			m_buffers[_param.name] = m_buffers.size();
			m_params.push_back(_param);
		}

		m_valid = true;
	}
	//----------------------------------------------------------------------------//
	int ShaderProgram::GetParam(const String& _name)
	{
		auto _param = m_names.find(_name);
		return _param != m_names.end() ? _param->second : -1;
	}
	//----------------------------------------------------------------------------//
	void ShaderProgram::BindBuffer(uint _id, uint _slot)
	{
		if (_id < (uint)m_params.size())
		{
			ShaderParam& _param = m_params[_id];
			if (_param.type == SPT_Buffer)
			{
				glUniformBlockBinding(m_handle, _param.location, _slot);
				_param.slot = _slot;
			}
		}
	}
	//----------------------------------------------------------------------------//
	void ShaderProgram::BindTexture(uint _id, uint _slot)
	{
		if (_id < (uint)m_params.size())
		{
			ShaderParam& _param = m_params[_id];
			if (_param.type == SPT_Texture)
			{
				glProgramUniform1iEXT(m_handle, _param.location, _slot);
				_param.slot = _slot;
			}
		}
	}
	//----------------------------------------------------------------------------//
	bool ShaderProgram::_InitMgr(void)
	{
		return true;
	}
	//----------------------------------------------------------------------------//
	void ShaderProgram::_DestroyMgr(void)
	{
		// ...
	}
	//----------------------------------------------------------------------------//
	ShaderProgramPtr ShaderProgram::_Create(ShaderInstance* _vs, ShaderInstance* _fs, ShaderInstance* _gs, uint _outputAttribs)
	{
		String _name;  // example: "VS:Model.glsl@VS@00000140; FS:R2ShadowMap.glsl@FS:00000000; GS:R2CubeMap@GS@00000001; VF:0@0;"
		_name += "VS:";
		if (_vs)
			_name += _vs->m_name;
		_name += "; ";
		_name += "FS:";
		if (_fs)
			_name += _fs->m_name;
		_name += "; ";
		_name += "GS:";
		if (_gs)
			_name += _gs->m_name;
		_name += "; ";

		_outputAttribs &= 0xf;
		_name += StrFormat("VF:0@%1x;", _outputAttribs);

		uint _hash = NameHash(_name);

		auto _exists = s_cache.find(_hash);
		if (_exists != s_cache.end())
			return _exists->second;

		ShaderProgramPtr _prog = new ShaderProgram(_hash, _vs, _fs, _gs, _outputAttribs);
		
		return nullptr;
	}
	//----------------------------------------------------------------------------//
#endif

	//----------------------------------------------------------------------------//
	// ShaderManager
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// RenderContext
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool RenderContext::_Create(void)
	{
		LOG_EVENT("Create RenderContext");

		new RenderContext;
		if (!s_instance->_Init())
		{
			LOG_ERROR("Couldn't initialize RenderContext");
			delete s_instance;
			return false;
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	void RenderContext::_Destroy(void)
	{
		if (s_instance)
		{
			LOG_EVENT("Destroy RenderContext");
			delete s_instance;
		}
	}
	//----------------------------------------------------------------------------//
	RenderContext::RenderContext(void)
	{
	}
	//----------------------------------------------------------------------------//
	RenderContext::~RenderContext(void)
	{
		m_indexBuffer = nullptr;

		for (BufferObjectPtr& _vb : m_vertexBuffers)
			_vb = nullptr;

		Shader::_DestroyCache();

		gResourceCache->UnregisterManager(ShaderSource::StaticClassID());

		VertexFormat::_DestroyCache();

		if (m_context)
			SDL_GL_DeleteContext(m_context);

		if (m_window)
			SDL_DestroyWindow(m_window);
	}
	//----------------------------------------------------------------------------//
	bool RenderContext::_Init(void)
	{
		// init context

		if (SDL_Init(SDL_INIT_VIDEO))
		{
			LOG_ERROR("Couldn't initialize SDL Video : %s", SDL_GetError());
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, true);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

		// create window

		m_window = SDL_CreateWindow(0, 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
		if (!m_window)
		{
			LOG_ERROR("Couldn't create SDL window : %s", SDL_GetError());
			return false;
		}

		// create context

		m_context = SDL_GL_CreateContext(m_window);
		if (!m_context)
		{
			const char* _err = SDL_GetError();

			// bug(?) in some GL drivers : wglCreateContextAttribsARB returns error code without 29 bit (app-defined error code)
			// todo: add this code to WIN_SetErrorFromHRESULT (SDL_windows.c)
#ifdef _WIN32 
			HRESULT _r = GetLastError(); 
			if (_r == 0xc0072095) // ERROR_INVALID_VERSION_ARB
				_err = "Couldn't create OpenGL Context: Invalid version";
			else if (_r == 0xc0072096) // ERROR_INVALID_PROFILE_ARB
				_err = "Couldn't create OpenGL Context: Invalid profile";
#endif
			LOG_ERROR("%s", _err);
			return false;
		}

		// load functions/extensions

		if (!ogl_LoadFunctions())
		{
			LOG_ERROR("Couldn't load OpenGL funtions");
			return false;
		}

		LOG_INFO("OpenGL %s, %s", glGetString(GL_VERSION), glGetString(GL_VENDOR));
		LOG_INFO("Renderer: %s", glGetString(GL_RENDERER));
		LOG_INFO("Shaders: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
		LOG_INFO("GL_EXT_texture_filter_anisotropic: %s", ogl_ext_EXT_texture_filter_anisotropic ? "Yes" : "No");
		LOG_INFO("GL_EXT_texture_compression_s3tc: %s", ogl_ext_EXT_texture_compression_s3tc ? "Yes" : "No");
		LOG_INFO("GL_EXT_texture_compression_rgtc: %s", ogl_ext_EXT_texture_compression_rgtc ? "Yes" : "No");
		LOG_INFO("GL_EXT_direct_state_access: %s", ogl_ext_EXT_direct_state_access ? "Yes" : "No");
		LOG_INFO("GL_ARB_vertex_attrib_binding: %s", (ogl_ext_ARB_vertex_attrib_binding || ogl_IsVersionGEQ(4, 3)) ? "Yes" : "No");
		LOG_INFO("GL_ARB_debug_output: %s", ogl_ext_ARB_debug_output ? "Yes" : "No");
		LOG_INFO("GL_ARB_separate_shader_objects: %s", (ogl_ext_ARB_separate_shader_objects || ogl_IsVersionGEQ(4, 1)) ? "Yes" : "No");
		LOG_INFO("GL_ARB_get_program_binary: %s", (ogl_ext_ARB_get_program_binary || ogl_IsVersionGEQ(4, 1)) ? "Yes" : "No");

		if (!ogl_ext_EXT_direct_state_access || 
			(!ogl_ext_ARB_vertex_attrib_binding && !ogl_IsVersionGEQ(4, 3)) ||
			(!(ogl_ext_ARB_separate_shader_objects && ogl_ext_ARB_get_program_binary) && !ogl_IsVersionGEQ(4, 1)) ||
			!ogl_ext_EXT_texture_filter_anisotropic ||
			!ogl_ext_EXT_texture_compression_s3tc || 
			!ogl_ext_EXT_texture_compression_rgtc)
		{
			LOG_ERROR("One or more OpenGL extensions is not supported");
			return false;
		}

		// init vertex format

		VertexFormat::_InitCache();
		m_vertexFormat = VertexFormat::_GetInstance(0);

		// init shaders

		Shader::_InitCache();

		Ptr<ResourceManager> _smgr = new GenericResourceManager<ShaderSource>(ShaderSource::StaticClassName(), [] { return ResourcePtr(new ShaderSource); });
		gResourceCache->RegisterManager(_smgr);

		// todo: set primitive restart 0xffff

		// todo: reset state

		return true;
	}
	//----------------------------------------------------------------------------//
	void RenderContext::_BeginFrame(void)
	{
		int _w, _h;
		SDL_GetWindowSize(m_window, &_w, &_h);
		glViewport(0, 0, _w, _h);
		glClearColor(0.6f, 0.6f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	//----------------------------------------------------------------------------//
	void RenderContext::_EndFrame(void)
	{
		SDL_GL_SetSwapInterval(1);
		SDL_Delay(16);
		SDL_GL_SwapWindow(m_window);
	}
	//----------------------------------------------------------------------------//
	VertexFormat* RenderContext::AddVertexFormat(const VertexAttrib* _attribs)
	{
		return VertexFormat::_AddInstance(_attribs);
	}
	//----------------------------------------------------------------------------//
	uint RenderContext::ReloadShaders(void)
	{
		gResourceCache->ReloadAllResources<ShaderSource>();
		
		uint _num = 0;
		if (!Shader::s_uncompiledShaders.empty())
		{
			double _st = TimeMs();
			Array<Shader*> _shaders(Shader::s_uncompiledShaders.begin(), Shader::s_uncompiledShaders.end());
			_num = _shaders.size();

			for (Shader* i : _shaders)
				i->_Compile();

			if (_num)
			{
				LOG_EVENT("%u Shaders was recompiled, %.2f ms", _num, TimeMs() - _st);
			}
		}
		return _num;
	}
	//----------------------------------------------------------------------------//
	/*ShaderObjectPtr RenderContext::CreateShader(ShaderType _type)
	{
		return new ShaderObject(_type);
	}
	//----------------------------------------------------------------------------//
	ProgramObjectPtr CreateProgram(ShaderObject* _vs, ShaderObject* _fs, ShaderObject* _gs)
	{
		return nullptr;
	}*/
	//----------------------------------------------------------------------------//
	void RenderContext::SetVertexFormat(VertexFormat* _fmt)
	{
		if (!_fmt)
			_fmt = VertexFormat::_GetInstance(0);

		if (m_vertexFormat != _fmt)
		{
			_fmt->_Bind(m_vertexFormat->m_attribMask);
			m_vertexFormat = _fmt;
		}
	}
	//----------------------------------------------------------------------------//
	void RenderContext::SetVertexBuffer(uint _slot, BufferObject* _buffer, uint _offset, uint _stride)
	{
		assert(_slot < MAX_VERTEX_STREAMS);

		glBindVertexBuffer(_slot, _buffer ? _buffer->m_handle : 0, _offset, _stride);
		m_vertexBuffers[_slot] = _buffer;
	}
	//----------------------------------------------------------------------------//
	void RenderContext::SetIndexFormat(IndexFormat _fmt)
	{
		if (m_indexFormat != _fmt)
		{
			glPrimitiveRestartIndex(_fmt == IF_UShort ? 0xffff : 0xffffffff);
			m_indexFormat = _fmt;
			m_indexFormatGL = m_indexFormat == IF_UShort ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		}
	}
	//----------------------------------------------------------------------------//
	void RenderContext::SetIndexBuffer(BufferObject* _buffer, uint _offset)
	{
		m_indexBuffer = _buffer;
		m_indexBufferOffset = _offset;
		if (_buffer)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffer->m_handle);
	}
	//----------------------------------------------------------------------------//
	void RenderContext::SetPrimitiveType(PrimitiveType _type)
	{
		m_primitiveType = _type;
		m_primitiveTypeGL = PrimitiveType2GL[_type];
	}
	//----------------------------------------------------------------------------//
	void RenderContext::SetDrawIndirectBuffer(BufferObject* _buffer, uint _offset)
	{
		// ...
	}
	//----------------------------------------------------------------------------//
	void RenderContext::Draw(uint _baseVertex, uint _count, uint _numInstances)
	{
		glDrawArraysInstanced(m_primitiveTypeGL, _baseVertex, _count, _numInstances);
	}
	//----------------------------------------------------------------------------//
	void RenderContext::DrawIndexed(uint _baseVertex, uint _baseIndex, uint _count, uint _numInstances)
	{
		if (m_indexBuffer)
		{
			uint _idx = m_indexBufferOffset + _baseIndex * m_indexFormat;
			glDrawElementsInstancedBaseVertex(m_primitiveTypeGL, _count, m_indexFormatGL, (void*)_idx, _numInstances, _baseVertex);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

