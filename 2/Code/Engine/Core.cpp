#include "Core.hpp"
#include <SDL.h>
#include <SDL.h>
#ifdef _WIN32
#	include <Windows.h>
#endif
#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Log
	//----------------------------------------------------------------------------//
	
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
	void LogMessage(int _level, const char* _msg, ...)
	{
		va_list _args;
		va_start(_args, _msg);
		uint _cc = 0;
		switch (_level)
		{
		case LL_Info:
			printf("Info: ");
			break;
		case LL_Event:
			printf("Event: ");
			break;
		case LL_Warning:
			_cc = _SetCColors(0x0e);
			printf("Warning: ");
			break;
		case LL_Error:
			_cc = _SetCColors(0x0c);
			printf("Error: ");
			break;
		case LL_Debug:
			_cc = _SetCColors(0x08);
			printf("Debug: ");
			break;
		}
		vprintf(_msg, _args);
		printf("\n");
		va_end(_args);

		if (_cc)
			_SetCColors(_cc);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Time
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	double TimeS(void)
	{
		double _f = 1.0 / (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c * _f;
	}
	//----------------------------------------------------------------------------//
	double TimeMs(void)
	{
		double _f = 1000.0 / (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c * _f;
	}
	//----------------------------------------------------------------------------//
	double TimeUs(void)
	{
		double _f = 1000000.0 / (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c * _f;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// File
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	File::File(void) :
		m_handle(nullptr),
		m_size(0),
		m_readOnly(true)
	{
	}
	//----------------------------------------------------------------------------//
	File::~File(void)
	{
		if (m_handle)
			fclose(m_handle);
	}
	//----------------------------------------------------------------------------//
	File::File(const String& _name, FILE* _handle, bool _readOnly) :
		m_name(_name),
		m_handle(_handle),
		m_size(0),
		m_readOnly(_readOnly)
	{
		if (m_handle && m_readOnly)
			m_size = _ReadSize();
	}
	//----------------------------------------------------------------------------//
	File::File(File&& _temp) :
		m_name(_temp.m_name),
		m_handle(_temp.m_handle),
		m_size(_temp.m_size),
		m_readOnly(_temp.m_readOnly)
	{
		_temp.m_handle = nullptr;
	}
	//----------------------------------------------------------------------------//
	File& File::operator = (File&& _temp)
	{
		m_name = _temp.m_name;
		std::swap(m_handle, _temp.m_handle);
		m_size = _temp.m_size;
		m_readOnly = _temp.m_readOnly;
		return *this;
	}
	//----------------------------------------------------------------------------//
	void File::Seek(int _pos, int _origin)
	{
		if (m_handle)
			fseek(m_handle, _pos, _origin);
	}
	//----------------------------------------------------------------------------//
	uint File::Tell(void)
	{
		return m_handle ? (uint)ftell(m_handle) : 0;
	}
	//----------------------------------------------------------------------------//
	bool File::Eof(void)
	{
		return !m_handle || feof(m_handle) != 0;
	}
	//----------------------------------------------------------------------------//
	uint File::Read(void* _dst, uint _size)
	{
		assert(_dst || !_size);
		return m_handle ? (uint)fread(_dst, 1, _size, m_handle) : 0;
	}
	//----------------------------------------------------------------------------//
	uint File::Write(const void* _src, uint _size)
	{
		assert(_src || !_size);
		return !m_readOnly && m_handle ? (uint)fwrite(_src, 1, _size, m_handle) : 0;
	}
	//----------------------------------------------------------------------------//
	void File::Flush(void)
	{
		if (m_handle && !m_readOnly)
			fflush(m_handle);
	}
	//----------------------------------------------------------------------------//
	String File::AsString(void)
	{
		if (!m_handle)
			return "";

		Array<char> _buff;
		uint _pos = Tell();
		uint _maxLength = Size() - _pos;
		_buff.resize(_maxLength + 1);
		uint _readed = Read(&_buff[0], _maxLength);
		_buff[_readed] = 0;
		return &_buff[0];
	}
	//----------------------------------------------------------------------------//
	uint File::_ReadSize(void)
	{
		if (!m_handle)
			return 0;
		long _pos = ftell(m_handle);
		fseek(m_handle, 0, SEEK_END);
		uint _size = (uint)ftell(m_handle);
		fseek(m_handle, _pos, SEEK_SET);
		return _size;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//
	
	FileSystem FileSystem::s_instance;

	//----------------------------------------------------------------------------//
	FileSystem::FileSystem(void)
	{
		char* _path = SDL_GetBasePath();
		m_appDir = MakeFullPath(_path) + "/";
		m_rootDir = m_appDir + "/";
		SDL_free(_path);
		chdir(m_rootDir.c_str());
	}
	//----------------------------------------------------------------------------//
	FileSystem::~FileSystem(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::SetRoot(const String& _path)
	{
		String _fpath = MakeFullPath(_path, m_appDir);
		if (chdir(_fpath.c_str()) == 0)
		{
			m_rootDir = _fpath + "/";
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::AddPath(const String& _path)
	{
		String _fpath = MakeFullPath(_path, m_rootDir);

		for (const String& _dir : m_paths)
		{
			if (StrCompare(_fpath.c_str(), _dir.c_str(), true) == 0)
				return true; // already exists
		}

		struct stat _st;
		if (!stat(_fpath.c_str(), &_st) && (_st.st_mode & _S_IFDIR))
		{
			m_paths.push_front(_fpath + "/");
			return true;
		}

		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::FileExists(const String& _path)
	{
		return !FindFile(_path).empty();
	}
	//----------------------------------------------------------------------------//
	String FileSystem::FindFile(const String& _path)
	{
		String _npath = MakeFullPath(_path);
		struct stat _st;
		if (IsFullPath(_npath))
		{
			if (!stat(_npath.c_str(), &_st))
			{
				return (_st.st_mode & _S_IFREG) ? _npath : "";
			}
		}

		String _test = _npath;
		for (const String& _dir : m_paths)
		{
			_test = MakeFullPath(_npath, _dir);
			if (!stat(_test.c_str(), &_st))
			{
				return (_st.st_mode & _S_IFREG) ? _test : "";
			}
		}

		return "";
	}
	//----------------------------------------------------------------------------//
	time_t FileSystem::FileTime(const String& _path)
	{
		String _npath = MakeFullPath(_path);
		struct stat _st;
		if (IsFullPath(_path))
		{
			if (!stat(_npath.c_str(), &_st))
			{
				return (_st.st_mode & _S_IFREG) ? _st.st_mtime : 0;
			}
		}

		String _test = _npath;
		for (const String& _dir : m_paths)
		{
			_test = MakeFullPath(_npath, _dir);
			if (!stat(_test.c_str(), &_st))
			{
				return (_st.st_mode & _S_IFREG) ? _st.st_mtime : 0;
			}
		}

		return 0;
	}
	//----------------------------------------------------------------------------//
	File FileSystem::ReadFile(const String& _name)
	{
		String _nname = MakeFullPath(_name);
		String _path = FindFile(_nname);
		FILE* _file = nullptr;
		if (!_path.empty())
			_file = fopen(_path.c_str(), "rb");

		if (!_file)
			LOG_ERROR("Couldn't open file '%s'", _name.c_str());

		return File(_nname, _file, true);
	}
	//----------------------------------------------------------------------------//
	File FileSystem::WriteFile(const String& _name, bool _overwrite)
	{
		String _nname = MakeFullPath(_name, m_writeDir);
		CreateDir(FilePath(_nname));

		FILE* _file = fopen(_nname.c_str(), _overwrite ? "w+b" : "r+b");

		if (!_file)
			LOG_ERROR("Couldn't create file '%s'", _name.c_str());

		return File(_nname, _file, false);
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::CreateDir(const String& _path)
	{
		String _fpath = MakeFullPath(_path, m_rootDir);
		if (!_CreateDir(_fpath))
		{
			LOG_ERROR("Couldn't create directory '%s'", _fpath.c_str());
			return false;
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::_CreateDir(const String& _path)
	{
		struct stat _st;
		if (!stat(_path.c_str(), &_st))
			return (_st.st_mode & _S_IFDIR) != 0;

		String _dir, _dev;
		Array<String> _items;
		SplitPath(_path, &_dev, &_items);
#ifdef _WIN32
		_dir = _dev + ":/";
#else // linux (not tested)
		_dir = "/" + _dev;
#endif
		if (stat(_dir.c_str(), &_st) || !(_st.st_mode & _S_IFDIR))
			return false;

		for (const String& i : _items)
		{
			_dir += i + "/";
			if (!stat(_dir.c_str(), &_st))
			{
				if (!(_st.st_mode & _S_IFDIR))
					return false;
			}
			else if (mkdir(_dir.c_str()))
				return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Config::Lexer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool Config::Lexer::operator()(String& _token, bool& _isString)
	{
		_isString = false;
		_token.clear();

		while (*s)
		{
			if (strchr(" \t", *s)) // white space
			{
				++s;
			}
			else if (strchr("\n\r", *s)) // new line
			{
				if (s[0] == '\r' && s[1] == '\n')
					++s;
				++s;
				++l;
			}
			else if (*s == '/' && strchr("/*", s[1])) // comment
			{
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
					s += 2;
				}
			}
			else if (*s == '\"') // cstring
			{
				++s;
				while (*s)
				{
					if (*s == '\\') // escape sequence
					{
						if (s[1] == 'n')
							_token += '\n';
						else if (s[1] == 'r')
							_token += '\r';
						else if (s[1] == 't')
							_token += '\t';
						else if (s[1] == '\'')
							_token += '\'';
						else if (s[1] == '\"')
							_token += '\"';
						else if (s[1] == '\\')
							_token += '\\';
						else
						{
							//error = "Unknown escape sequence";
							return false;
						}
						s += 2;
					}
					else if (*s == '"') // end of string
					{
						++s;
						break;
					}
					else if (strchr("\n\r", *s)) // new line
					{
						//error = "New line in string";
						return false;
					}
					else // other
						_token += *s++;
				}
				_isString = true;
				break;
			}
			else if (*s == '\'') // vstring
			{
				++s;
				while (*s)
				{
					if (s[0] == '\\' && s[1] == '\'') // apostrophe
					{
						_token += '\'';
						s += 2;
					}
					else if (strchr("\n\r", *s)) // new line
					{
						_token += '\n';
						if (s[0] == '\r' && s[1] == '\n')
							++s;
						++s;
						++l;
					}
					else if (*s == '\'') // end of string
					{
						++s;
						break;
					}
					else // other
						_token += *s++;
				}
				_isString = true;
				break;
			}
			else if (strchr("{}=", *s)) // operator
			{
				_token.push_back(*s++);
				break;
			}
			else // identifier
			{
				while (*s && !strchr(" \t\n\r={}\"\'", *s))
				{
					if (*s == '/' && strchr("/*", s[1])) // comment
						break;
					_token.push_back(*s++);
				}
				break;
			}
		}
		return true;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Config
	//----------------------------------------------------------------------------//

	/*
	CSTRING = '"', { ALL_SYMBOLS - ('"' | NEW_LINE) }, '"'; (*C-Like strings with escape sequences*)
	VSTRING = "'", { ALL_SYMBOLS - "'" }, "'"; (*symbol "'" can be written as "\'" *)
	STRING = CSTRING | VSTRING;
	IDENTIFIER = (ALL_SYMBOLS - ('"' | "'" | '=' | '{' | '}' | NEW_LINE | SPACE | COMMENT));
	VALUE = IDENTIIFIER | STRING ;
	NAME = IDENTIIFIER | STRING;
	BLOCK = NAME ['=', VALUE] ['{', [{BLOCK}], '}'];
	DOC = [{BLOCK}];

	example:

	name
	{
		value = '0 0 0'
		value = "it's string"
		value = files/object.mesh
		value = 0.1
		child = files/object2.mesh
		{
			pos = '0 0 0'
		}
	}

	*/

	const Config Config::Empty;

	//----------------------------------------------------------------------------//
	Config::Config(const Config& _other) :
		m_name(_other.m_name),
		m_value(_other.m_value),
		m_childs(_other.m_childs)
	{
	}
	//----------------------------------------------------------------------------//
	Config::Config(Config&& _temp) :
		m_name(std::move(_temp.m_name)),
		m_value(std::move(_temp.m_value)),
		m_childs(std::move(_temp.m_childs))
	{
	}
	//----------------------------------------------------------------------------//
	Config& Config::operator = (Config&& _temp)
	{
		m_name = std::move(_temp.m_name);
		m_value = std::move(_temp.m_value);
		m_childs = std::move(_temp.m_childs);
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config& Config::operator = (const Config& _rhs)
	{
		m_name = _rhs.m_name;
		m_value = _rhs.m_value;
		m_childs = _rhs.m_childs;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config::Config(const String& _name, const String& _value) :
		m_name(_name),
		m_value(_value)
	{
	}
	//----------------------------------------------------------------------------//
	Config& Config::SetValueF(const char* _fmt, ...)
	{
		va_list _args;
		va_start(_args, _fmt);
		m_value = StrFormatV(_fmt, _args);
		va_end(_args);
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config& Config::AddChild(const String& _name, const String& _value)
	{
		m_childs.push_back({ _name, _value });
		return m_childs.back();
	}
	//----------------------------------------------------------------------------//
	Config* Config::FindChild(const String& _name, Config* _prev)
	{
		if (!m_childs.empty())
		{
			size_t _idx = _prev ? _prev - &m_childs[0] : 0;
			for (size_t _size = m_childs.size(); _idx < _size; ++_idx)
			{
				if (m_childs[_idx].m_name == _name)
					return &m_childs[_idx];
			}
		}
		return nullptr;
	}
	//----------------------------------------------------------------------------//
	const Config& Config::GetChild(const String& _name) const
	{
		for (const Config& _child : m_childs)
		{
			if (_child.m_name == _name)
				return _child;
		}
		return Empty;
	}
	//----------------------------------------------------------------------------//
	Config& Config::RemoveAllChilds(void)
	{
		m_childs.clear();
		return *this;
	}
	//----------------------------------------------------------------------------//
	bool Config::Parse(const String& _text, uint* _errorLine)
	{
		return Parse(_text.c_str(), _errorLine);
	}
	//----------------------------------------------------------------------------//
	bool Config::Parse(const char* _text, uint* _errorLine)
	{
		Lexer _lex = { _text, 0 };
		if (_Parse(_lex))
			return true;
		if (_errorLine)
			*_errorLine = _lex.l;
		return false;
	}
	//----------------------------------------------------------------------------//
	String Config::Print(void) const
	{
		// note: name and value for root node cannot be saved
		String _r;
		_Print(_r, 0);
		return _r;
	}
	//----------------------------------------------------------------------------//
	bool Config::_Parse(Lexer& _lex)
	{
		m_childs.clear();
		if (!_lex.s)
			return true;

		bool _isString;
		String _token, _name, _value;
		while (*_lex.s)
		{
			if (!_lex(_token, _isString)) // read name
				return false; // parse error

			if (_token == "}" && !_isString) // end of node
				return true;

		_nextChild:
			_name = _token;
			if (!_lex(_token, _isString)) // '=' or '{'
				return false;

			if (_token == "=") // child value 
			{
				if (!_lex(_token, _isString)) // read value	
					return false; // parse error

				if (!_isString && (_token == "=" || _token == "{" || _token == "}"))
					return false; // unexpected token

				_value = _token;
			}
			else if (_token == "{") // child node, without of value
			{
				Config& _child = AddChild(_name);
				if (!_child._Parse(_lex))
					return false;
				continue;
			}
			else
			{
				return false; // unexpected token
			}

			Config& _child = AddChild(_name, _value);

			if (!_lex(_token, _isString)) // probably '{' or next child
				return false; // parse error

			if (!_isString)
			{
				if (_token == "{") // child node
				{
					if (!_child._Parse(_lex))
						return false;
					continue;
				}
				else if (_token == "}") // end of node
					return true;
				else if (_token == "=") // unexpected token
					return false;
				else if (*_lex.s) // next child
					goto _nextChild;
			}
			else // next child
			{
				goto _nextChild;
			}
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	String Config::_PrintValue(const String& _value)
	{
		if (_value.empty())
			return "''";

		// " \t\n\r={}\"\'"

		if (strchr(_value.c_str(), '\n') || strchr(_value.c_str(), '\r') || // new line
			strchr(_value.c_str(), ' ') || strchr(_value.c_str(), '\t') || // white space
			strstr(_value.c_str(), "//") || strstr(_value.c_str(), "/*") || // comment
			strchr(_value.c_str(), '\'') || strchr(_value.c_str(), '\"') || // string
			strchr(_value.c_str(), '=') || strchr(_value.c_str(), '{') || strchr(_value.c_str(), '}')) // operator
		{
			String _r;
			_r.reserve(_value.length() + 2);
			_r += "'";
			if (strchr(_value.c_str(), '\''))
			{
				for (char c : _value)
				{
					if (c == '\'')
						_r += "\'";
					else
						_r.push_back(c);
				}
			}
			else
			{
				_r += _value;
			}
			_r += "'";
			return _r;
		}

		return _value;
	}
	//----------------------------------------------------------------------------//
	void Config::_Print(String& _dst, int _offset) const
	{
		for (const Config& _child : m_childs)
		{
			if (_child.IsEmpty())
				continue;

			_dst.append(_offset, '\t');
			_dst += _PrintValue(_child.m_name);
			if (_child.m_childs.empty() && _child.m_name.empty())
			{
				_dst += " = ''";
			}
			else
			{
				if (!_child.m_value.empty())
				{
					_dst += " = ";
					_dst += _PrintValue(_child.m_value);
				}
				if (!_child.m_childs.empty())
				{
					_dst += "\n";
					_dst.append(_offset, '\t');
					_dst += "{\n";
					_child._Print(_dst, _offset + 1);
					_dst.append(_offset, '\t');
					_dst += "}";
				}
			}
			_dst += "\n";
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Resource::Resource(void) :
		m_id(0),
		m_resourceMgr(nullptr),
		m_resourceFlags(0),
		m_resourceState(RS_Unloaded),
		m_fileTime(0),
		m_valid(true),
		m_trackingEnabled(false),
		m_qnext(nullptr),
		m_qprev(nullptr)
	{
		m_trackingEnabled = gResourceCache->IsTrackingEnabled();
	}
	//----------------------------------------------------------------------------//
	Resource::~Resource(void)
	{
		if (m_resourceMgr)
			m_resourceMgr->_RemoveRefs(this);
	}
	//----------------------------------------------------------------------------//
	bool Resource::BeginReload(void)
	{
		if (m_resourceFlags & RF_Manually)
			return false;

		time_t _time = gFileSystem->FileTime(m_fileName);
		return _time > m_fileTime;
	}
	//----------------------------------------------------------------------------//
	void Resource::Load(uint _flags)
	{
		if (m_resourceFlags & RF_Manually)
			return;

		ResourcePtr _rc = this;

		if ((_flags & RLF_Reload) && ((_flags & RLF_ForceReload) || BeginReload()))
		{
			_ChangeState(m_resourceState == RS_Unloaded ? RS_Unloaded : RS_Loaded, RS_Queued);
		}

		if (!(_flags & RLF_Async))
		{
			if (_ChangeState(RS_Queued, RS_Loading) || _ChangeState(RS_Unloaded, RS_Loading)) // load now
			{
				m_valid = _Load();
				m_resourceState = RS_Loaded;
				gResourceCache->_Signal(this);
			}
			else if (m_resourceState == RS_Loading)	// wait loading
			{
				gResourceCache->_Wait(this);
			}
		}
		else
		{
			_ChangeState(RS_Unloaded, RS_Queued); // add to queue
		}
	}
	//----------------------------------------------------------------------------//
	bool Resource::Touch(bool _wait)
	{
		if (m_resourceState != RS_Loaded)
		{
			Load(_wait ? RLF_Wait : RLF_Async);
		}
		return m_resourceState == RS_Loaded;
	}
	//----------------------------------------------------------------------------//
	void Resource::_InitResource(ResourceManager* _mgr, const String& _name, uint _uid, uint _flags)
	{
		m_name = _name;
		m_id = _uid ? _uid : NameHash(_name);
		m_resourceFlags = _flags;
		m_resourceMgr = _mgr;
		_GetFileName();
	}
	//----------------------------------------------------------------------------//
	bool Resource::_Load(void)
	{
		double _st = TimeMs();

		File _f = gFileSystem->ReadFile(m_fileName);
		m_fileTime = gFileSystem->FileTime(m_fileName);
		if (!_Load(_f))
		{
			//LOG_ERROR("Couldn't load %s '%s'", ClassName().c_str(), m_name.c_str());
			return false;
		}

		if (m_trackingEnabled)
		{
			double _t = TimeMs() - _st;
			LOG_DEBUG("Load %s '%s', %.2f ms", ClassName().c_str(), m_name.c_str(), _t);
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	bool Resource::_ChangeState(ResourceState _exp, ResourceState _state)
	{
		if (Cas(m_resourceState, _exp, _state))
		{
			if (_exp == RS_Queued && _state != RS_Queued)
			{
				gResourceCache->_Unqueue(this);
			}
			else if (_exp != RS_Queued && _state == RS_Queued)
			{
				gResourceCache->_Queue(this);
			}
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void Resource::_DeleteThis(void)
	{
		if (m_trackingEnabled)
		{
			LOG_DEBUG("Delete %s '%s'", ClassName().c_str(), m_name.c_str());
		}
		delete this;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceManager
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ResourceManager::ResourceManager(const String& _class) :
		m_class(_class),
		m_classID(NameHash(_class)),
		m_reloading(0)
	{
	}
	//----------------------------------------------------------------------------//
	ResourceManager::~ResourceManager(void)
	{
		_RemoveAllResources();
		assert(m_resourcesToReload.empty());
	}
	//----------------------------------------------------------------------------//
	Resource* ResourceManager::AddResource(const String& _name, uint _flags)
	{
		if (_name.empty())
			return nullptr;

		bool _deferredLoading = gResourceCache->IsDeferredLoading();

		String _nname = MakeFullPath(_name); // normalize
		uint _id = NameHash(_nname);
		ResourcePtr _r;

		// find existent resource
		{
			auto _exists = m_resources.find(_id);
			if (_exists != m_resources.end())
			{
				_r = _exists->second;
				_r->m_resourceFlags |= (_flags & RF_Persistent);
				_r->Load(!_deferredLoading); // load or add to queue
				return _r;
			}
		}

		// add new resource
		_r = _Create(_name, _id, _flags);

		m_resources[_id] = _r;
		if (!(_flags & RF_Temp))
			m_cache[_id] = _r;

		if (!(_flags & RF_Manually))
			_r->Load(_deferredLoading ? RLF_Async : RLF_Wait); // load or add to queue

		return _r;
	}
	//----------------------------------------------------------------------------//
	void ResourceManager::RemoveResource(Resource* _r, bool _unqueue)
	{
		if (_r)
		{
			ResourcePtr _rc = _r;

			m_resources.erase(_r->m_id);
			if (!(_r->m_resourceFlags & RF_Temp))
				m_cache.erase(_r->m_id);

			if (_unqueue)
				_r->_ChangeState(RS_Queued, RS_Unloaded);
		}
	}
	//----------------------------------------------------------------------------//
	uint ResourceManager::RemoveUnusedResources(void)
	{
		uint _total = 0, _num = 0;
		do
		{
			_num = 0;
			for (auto i = m_cache.begin(); i != m_cache.end();)
			{
				Resource* _r = i->second;
				if (!(_r->m_resourceFlags & (RF_Manually | RF_Persistent | RF_Temp)))
				{
					uint _refs = _r->GetRefs();
					if (_refs == 2 && _r->IsQueued())
					{
						_r->_ChangeState(RS_Queued, RS_Unloaded); // unqueue
						_refs = _r->GetRefs();
					}

					if (_refs == 1)
					{
						i = m_cache.erase(i);
						++_num;
					}
					else
						++i;
				}
				else
					++i;
			}
			_total += _num;

		} while (_num > 0);

		return _total;
	}
	//----------------------------------------------------------------------------//
	uint ResourceManager::ReloadResources(bool _wait)
	{
		uint _flags = RLF_ForceReload | (_wait ? RLF_Wait : RLF_Async);
		uint _num = 0;
		double _st = TimeMs();

		// begin reloading
		if (!m_reloading++)
		{
			_BeginReloading(_wait);
		}

		// get resources for reloading
		for (auto& i : m_resources)
		{
			if (!i.second->IsQueued() && i.second->BeginReload())
				m_resourcesToReload.insert(i.second);
		}

		// reload resources
		for (auto& i : m_resourcesToReload)
		{
			i->Load(_flags);
		}

		// end of reloading
		if (!--m_reloading)
		{
			_EndReloading(_wait);
			assert(m_reloading == 0);

			_num = (uint)m_resourcesToReload.size();
			m_resourcesToReload.clear();
		}

		if (_num)
		{
			LOG_EVENT("%u %s(s) was %s, %.2f ms", _num, m_class.c_str(), _wait ? "reloaded" : "queued for reloading", TimeMs() - _st);
		}

		return _num;
	}
	//----------------------------------------------------------------------------//
	void ResourceManager::AddResourceForReload(Resource* _r)
	{
		if (_r && m_reloading > 0)
		{
			m_resourcesToReload.insert(_r);
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceManager::_RemoveRefs(Resource* _r)
	{
		m_resources.erase(_r->m_id);
	}
	//----------------------------------------------------------------------------//
	void ResourceManager::_RemoveAllResources(void)
	{
		for (auto i : m_resources)
		{
			i.second->_ChangeState(RS_Queued, RS_Unloaded); // remove from queue
			i.second->m_resourceMgr = nullptr;
		}

		m_resources.clear();
		m_cache.clear();
	}
	//----------------------------------------------------------------------------//
	ResourcePtr	ResourceManager::_Create(const String& _name, uint _id, uint _flags)
	{
		ResourcePtr _r = _Factory();
		_r->_InitResource(this, _name, _id, _flags);
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ResourceCache::ResourceCache(void) :
		m_qstart(nullptr),
		m_qend(nullptr),
		m_deferredLoading(false),
		m_trackingEnabled(false)
	{
		LOG_EVENT("Create ResourceCache");
#ifdef _DEBUG
		m_trackingEnabled = true;
#endif
	}
	//----------------------------------------------------------------------------//
	ResourceCache::~ResourceCache(void)
	{
		LOG_EVENT("Destroy ResourceCache");
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Shutdown(void)
	{
		LOG_EVENT("Cleaning ResourceCache");

		//m_mutex.Lock();
		while (m_qstart) // unqueue all resources
			m_qstart->_ChangeState(RS_Queued, RS_Unloaded);
		//m_mutex.Unlock();

		for (auto i : m_managers)
			i.second->_RemoveAllResources();

		m_managers.clear();
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::RegisterManager(ResourceManager* _manager)
	{
		if (_manager)
		{
			auto _exists = m_managers.find(_manager->GetResourceClassID());
			if (_exists == m_managers.end())
			{
				m_managers[_manager->GetResourceClassID()] = _manager;
				LOG_EVENT("Register %s manager (%s)", _manager->GetResourceClassName().c_str(), _manager->ClassName().c_str());
			}
			else
			{
				LOG_ERROR("%s manager already registered", _manager->GetResourceClassName().c_str());
			}
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::UnregisterManager(uint _id)
	{
		ResourceManager* _mgr = GetManager(_id);
		if (_mgr)
		{
			LOG_EVENT("Unregister %s manager", _mgr->ClassName().c_str());
			_mgr->_RemoveAllResources();
			m_managers.erase(_id);
		}
	}
	//----------------------------------------------------------------------------//
	ResourceManager* ResourceCache::GetManager(uint _id)
	{
		auto _it = m_managers.find(_id);
		return _it == m_managers.end() ? nullptr : _it->second;
	}
	//----------------------------------------------------------------------------//
	ResourcePtr ResourceCache::LoadResource(uint _type, const String& _name, uint _flags)
	{
		if (_name.empty())
			return nullptr;
		ResourceManager* _mgr = GetManager(_type);
		if (!_mgr)
			return nullptr;
		return _mgr->AddResource(_name, _flags);
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::SetDeferredLoading(bool _state)
	{
		m_deferredLoading = _state;
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::LoadQueuedResources(void)
	{
		double _st = TimeMs();
		uint _num = 0;
		while (LoadOneResource())
			++_num;

		if (_num)
		{
			LOG_EVENT("%u queued resources was loaded, %.2f ms", _num, TimeMs() - _st);
		}
		return _num;
	}
	//----------------------------------------------------------------------------//
	bool ResourceCache::LoadOneResource(String* _name)
	{
		//m_mutex.Lock();
		ResourcePtr _p = m_qend;
		//m_mutex.Unlock();
		if (_p)
		{
			if (_name)
				*_name = _p->GetName();
			_p->Load(RLF_Wait);
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::RemoveUnusedResources(void)
	{
		uint _num = 0;
		for (auto i : m_managers)
			_num += i.second->RemoveUnusedResources();
		if (_num && m_trackingEnabled)
		{
			LOG_EVENT("%u Resources was removed", _num);
		}
		return _num;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::ReloadAllResources(uint _type)
	{
		ResourceManager* _mgr = GetManager(_type);
		if (_mgr)
			_mgr->ReloadResources(!m_deferredLoading);
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Signal(Resource* _r)
	{
		//m_mutex.Lock();
		//m_loadedResource = _r;
		//m_mutex.Unlock();
		//m_signal.Signal();
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Wait(Resource* _r)
	{
		/*
		for(Resource* _l = nullptr; _l != _r && _r->m_state != RS_Loading;)
		{
		m_mutex.Lock();
		m_signal.Wait(m_mutex, 1);
		_l = m_loadedResource;
		m_mutex.Unlock();
		}
		*/
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Queue(Resource* _r)
	{
		//m_mutex.Lock();
		assert(_r->m_qprev == nullptr);
		_r->m_qnext = m_qstart;
		if (m_qstart)
			m_qstart->m_qprev = _r;
		else
			m_qend = _r;
		m_qstart = _r;
		_r->AddRef();
		//m_mutex.Unlock();
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Unqueue(Resource* _r)
	{
		//m_mutex.Lock();
		if (_r->m_qnext)
			_r->m_qnext->m_qprev = _r->m_qprev;
		else
			m_qend = _r->m_qprev;
		if (_r->m_qprev)
			_r->m_qprev->m_qnext = _r->m_qnext;
		else
			m_qstart = _r->m_qnext;
		_r->m_qprev = nullptr;
		_r->m_qnext = nullptr;
		//m_mutex.Unlock();
		_r->Release();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

