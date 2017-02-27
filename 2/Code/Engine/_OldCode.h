//----------------------------------------------------------------------------//
// ShaderGenerator
//----------------------------------------------------------------------------//

class ShaderGenerator : public RefCounted
{
public:
	virtual const char* GetType(void) = 0;
	virtual String Parse(Shader* _shader) = 0;
	virtual String Generate(Shader* _shader, ShaderType _type, const ShaderDefines& _defs) = 0;
};

//----------------------------------------------------------------------------//
// Shader
//----------------------------------------------------------------------------//

class Shader : public RefCounted
{
public:

	const String& GetRawSource(void) { return m_rawSource; }
	const String& GetLog(void) { return m_log; }

protected:
	friend class RenderContext;

	Shader(uint16 _name, uint _uid, const String& _fileName);
	~Shader(void);
	void _CreateFromFile(const String& _filename);
	void _CreateFromSource(const String& _source);
	void _SetSource(const String& _src);
	void _Invalidate(bool _force = false);
	bool _Load(bool _checkFileTime = false);
	bool _IsIncluded(Shader* _include);
	bool _LogError(const char* _err, ...);
	bool _Parse(void);

	bool m_valid;
	bool m_processed = false;
	bool m_loaded = false;
	bool m_createdFromFile = false;
	uint16 m_name;
	uint m_uid;
	uint m_changes;
	time_t m_fileTime;
	String m_fileName;
	String m_log;
	String m_rawSource; // raw
	String m_source;
	HashSet<ShaderPtr> m_includes;
	HashSet<Shader*> m_dependents;

	Ptr<ShaderGenerator> m_generator;

	static bool _InitMgr(void);
	static void _DestroyMgr(void);
	static uint _ReloadAll(void);
	static Shader* _AddInstance(const String& _name);
	static uint16 _AddName(const String& _name, uint _hash = 0);
	static String _ParseGLSLLog(const char* _log);

	static Array<String> s_names;
	static HashMap<uint, uint16> s_namesTable;
	static HashMap<uint, ShaderPtr> s_instances;
};


//----------------------------------------------------------------------------//
// Shader
//----------------------------------------------------------------------------//

Array<String> Shader::s_names;
HashMap<uint, uint16> Shader::s_namesTable;
HashMap<uint, ShaderPtr> Shader::s_instances;

//----------------------------------------------------------------------------//
Shader::Shader(uint16 _name, uint _uid, const String& _fileName) :
	m_name(_name),
	m_uid(_uid),
	m_valid(true),
	m_processed(true),
	m_loaded(false),
	m_fileTime(-1),
	m_fileName(_fileName),
	m_changes(0),
	m_log("Source code is empty\n")
{
	_Load();
}
//----------------------------------------------------------------------------//
Shader::~Shader(void)
{
	for (Shader* i : m_includes)
		i->m_dependents.erase(this);
	m_includes.clear();
}
//----------------------------------------------------------------------------//
void Shader::_CreateFromFile(const String& _filename)
{
	if (m_createdFromFile)
	{
		if (StrCompare(m_fileName.c_str(), _filename.c_str(), true))
			_Load(false);
	}
	else
	{
		m_createdFromFile = true;
		m_fileName = _filename;
		m_fileTime = -1;
		_Load(true);
	}
}
//----------------------------------------------------------------------------//
void Shader::_CreateFromSource(const String& _source)
{
	m_createdFromFile = false;
	_SetSource(_source);
}
//----------------------------------------------------------------------------//
void Shader::_SetSource(const String& _src)
{
	for (Shader* i : m_includes)
		i->m_dependents.erase(this);
	m_includes.clear();
	m_rawSource = _src;
	m_source.clear();
	m_processed = false;
	m_log = "Source code not was processed\n";
	m_valid = true;
	m_changes++;
	_Invalidate(true);
}
//----------------------------------------------------------------------------//
void Shader::_Invalidate(bool _force)
{
	if (m_processed || _force)
	{
		m_processed = false;
		for (Shader* i : m_dependents)
			i->_Invalidate();
	}
}
//----------------------------------------------------------------------------//
bool Shader::_Load(bool _checkFileTime)
{
	if (!m_createdFromFile)
		return false; // do not load

	if (m_loaded && !_checkFileTime)
		return false; // already loaded

	m_loaded = true;

	time_t _ft = gFileSystem->FileTime(m_fileName);
	if (_ft == m_fileTime)
		return true;

	m_fileTime = _ft;
	File _f = gFileSystem->ReadFile(m_fileName);

	_SetSource(_f.AsString());

	if (!_f)
	{
		m_log = "File '" + m_fileName + "' not was found\n";
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

	String _generatedCode;
	if (m_generator)
	{
		_generatedCode = m_generator->Parse(this);
		s = _generatedCode.c_str();
	}

	if (!s || !s[0])
	{
		m_valid = true;
		m_log = "Source code is empty\n";
		return true;
	}

	uint l = 1;
	String _nameStr = s_names[m_name];

	m_source += StrFormat("#line %d %d // %s\n", l, m_name, _nameStr.c_str());

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
					return _LogError("%s(%d): Error: Unexpected End of file in multiline comment\n", _nameStr.c_str(), l);
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

				ShaderPtr _inc = _AddInstance(_name);
				if (!_inc)
					return _LogError("%s(%d): Error: File '%s' not was found\n", _nameStr.c_str(), l, _name.c_str());

				if (!_inc || _inc == this || _inc->_IsIncluded(this))
					return _LogError("%s(%d): Error: Unable to include file '%s'\n", _nameStr.c_str(), l, _name.c_str());

				m_includes.insert(_inc);
				_inc->m_dependents.insert(this);

				if (!_inc->_Parse())
				{
					m_log += StrFormat("%s(%d): Error: Invalid included file '%s'\n", _nameStr.c_str(), l, _name.c_str());
					m_log += _inc->m_log;
					return false;
				}

				m_source += _inc->m_source; // include common section
				m_source += StrFormat("\n#line %d %d // %s\n", l, m_name, _nameStr.c_str());
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
bool Shader::_InitMgr(void)
{
	return true;
}
//----------------------------------------------------------------------------//
void Shader::_DestroyMgr(void)
{
	s_instances.clear();
}
//----------------------------------------------------------------------------//
uint Shader::_ReloadAll(void)
{
	uint _num = 0;

	for (auto _src : s_instances)
		_num += _src.second->_Load() ? 1 : 0;

	for (auto _src : s_instances)
	{
		if (!_src.second->m_processed && !_src.second->_Parse())
			LOG_ERROR("Shader '%s' was reloaded with errors\n%s", s_names[_src.second->m_name].c_str(), _src.second->m_log.c_str());
	}

	if (_num > 0)
		LOG_INFO("%u shaders was reloaded", _num);

	return _num;
}
//----------------------------------------------------------------------------//
Shader* Shader::_AddInstance(const String& _name)
{
	if (_name.empty())
		return nullptr;

	uint _hash = NameHash(_name);
	{
		auto _exists = s_instances.find(_hash);
		if (_exists != s_instances.end())
			return _exists->second;
	}

	uint16 _nameId = _AddName(_name, _hash);
	ShaderPtr _newInstance = new Shader(_nameId, _hash, "Shaders/" + _name);
	s_instances[_hash] = _newInstance;

	return _newInstance;
}
//----------------------------------------------------------------------------//
uint16 Shader::_AddName(const String& _name, uint _hash = 0)
{
	if (!_hash)
		_hash = NameHash(_name);

	auto _exists = s_namesTable.find(_hash);
	if (_exists != s_namesTable.end())
		return _exists->second;

	if (s_names.size() >= 0xffff)
	{
		LOG_ERROR("Shader names table overflow");
		assert(s_names.size() < 0xffff);
	}

	uint16 _id = (uint16)s_names.size();
	s_namesTable[_hash] = _id;
	s_names.push_back(_name);

	return _id;
}
//----------------------------------------------------------------------------//
String Shader::_ParseGLSLLog(const char* _log)
{
	String _r;
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
				if (_id < s_names.size())
				{
					const char* _start2 = _log;
					while (*_log && *_log >= '0' && *_log <= '9')
						++_log;
					if (_start2 != _log)
					{
						_r.append(s_names[_id]);
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

