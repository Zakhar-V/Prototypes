#include "Core.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// LogNode
	//----------------------------------------------------------------------------//

	namespace
	{
		static THREAD_LOCAL LogNode* s_logNode = nullptr; // top
		static THREAD_LOCAL int s_logDepth = 0;

		uint8 SetCColors(uint8 _color)
		{
#		ifdef WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
			{
				uint8 _pc = csbi.wAttributes & 0xff;
				SetConsoleTextAttribute(hStdOut, _color);
				return _pc;
			}
#		endif
			return 0;
		}

		void _LogWrite(const char* _msg, ...)
		{
			va_list _args;
			va_start(_args, _msg);
			vprintf(_msg, _args);
			va_end(_args);
		}
	}

	//----------------------------------------------------------------------------//
	LogNode::LogNode(bool _writeNow, const char* _func, const char* _title, ...) :
		m_prev(s_logNode),
		m_func(_func),
		m_written(false),
		m_depth(s_logDepth++)
	{
		s_logNode = this;
		va_list _args;
		va_start(_args, _title);
		m_title = StrFormatV(_title, _args);
		va_end(_args);
		if (_writeNow) _Write();
	}
	//----------------------------------------------------------------------------//
	LogNode::~LogNode(void)
	{
		if (m_written)
		{
			for (int i = 0; i < m_depth; ++i) _LogWrite("    ");
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
			m_written = true;
			if (m_prev) m_prev->_Write();
			for (int i = 0; i < m_depth; ++i) _LogWrite("    ");
			_LogWrite("%s\n", m_title.c_str());

			for (int i = 0; i < m_depth; ++i) _LogWrite("    ");
			_LogWrite("{\n");
		}
	}
	//----------------------------------------------------------------------------//
	void LogNode::Message(int _level, const char* _msg, ...)
	{
		if (s_logNode) s_logNode->_Write();

		for (int i = 0; i < s_logDepth; ++i) _LogWrite("    ");

		//time_t _t; time(&_t);
		//struct tm _tm = *localtime(&_t);
		//_LogWrite("[%02d:%02d:%02d]", _tm.tm_hour, _tm.tm_min, _tm.tm_sec);

		uint8 _cc = 0;
		switch (_level)
		{
		case 0: _LogWrite("- "); break;
		case 1: _LogWrite("[WARNING] "); _cc = SetCColors(0x0e); break;
		case 2: _LogWrite("[ERROR] "); _cc = SetCColors(0x0c); break;
		case 3: _LogWrite("[DEBUG] "); _cc = SetCColors(0x08); break;
		}

		va_list _args;
		va_start(_args, _msg);
		String _text = StrFormatV(_msg, _args);
		va_end(_args);
		_LogWrite("%s\n", _text.c_str());

		if (_cc) SetCColors(_cc);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// DataStream
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	DataStream::DataStream(void) :
		m_handle(nullptr),
		m_size(0),
		m_readOnly(true)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::DataStream(DataStream&& _temp) :
		m_name(_temp.m_name),
		m_handle(_temp.m_handle),
		m_size(_temp.m_size),
		m_readOnly(_temp.m_readOnly)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::DataStream(const String& _name, DataStreamHandle* _handle, bool _readOnly) :
		m_name(_name),
		m_handle(_handle),
		m_size(_handle ? _handle->Size() : 0),
		m_readOnly(_readOnly)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::~DataStream(void)
	{
		m_handle = nullptr;
	}
	//----------------------------------------------------------------------------//
	DataStream& DataStream::operator = (DataStream&& _temp)
	{
		Swap(m_handle, _temp.m_handle);
		m_name = _temp.m_name;
		m_size = _temp.m_size;
		m_readOnly = _temp.m_readOnly;
		return *this;
	}
	//----------------------------------------------------------------------------//
	void DataStream::SetPos(int _pos, bool _relative)
	{
		if (m_handle) m_handle->Seek(_pos, _relative ? SEEK_CUR : SEEK_SET);
	}
	//----------------------------------------------------------------------------//
	uint DataStream::GetPos(void)
	{
		return m_handle ? m_handle->Tell() : 0;
	}
	//----------------------------------------------------------------------------//
	void DataStream::ToEnd(void)
	{
		if (m_handle) m_handle->Seek(0, SEEK_END);
	}
	//----------------------------------------------------------------------------//
	bool DataStream::AtAnd(void)
	{
		return !m_handle || m_handle->EoF();
	}
	//----------------------------------------------------------------------------//
	uint DataStream::Read(void* _dst, uint _size)
	{
		ASSERT((_dst && _size) || !_size);
		return m_handle ? m_handle->Read(_dst, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	uint DataStream::Write(const void* _src, uint _size)
	{
		ASSERT((_src && _size) || !_size);
		return m_handle && !m_readOnly ? m_handle->Write(_src, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	void DataStream::Flush(void)
	{
		if (m_handle && !m_readOnly) m_handle->Flush();
	}
	//----------------------------------------------------------------------------//
	String DataStream::ReadString(int _maxLength)
	{
		String _r;
		if (m_handle)
		{
			uint _readed, _length, _pos = GetPos(), _size = _maxLength < 0 ? m_size : _maxLength;
			if (_pos > m_size) _pos = m_size;
			if (_size + _pos > m_size) _size = m_size - _pos;
			Array<char> _buf(_size + 1);
			_readed = Read(&_buf[0], _size);
			_buf[_readed] = 0;
			_length = strlen(&_buf[0]);
			if (_length + 1 < _readed) SetPos(_pos + _length + 1); // null-terminated string
			_r = &_buf[0];
		}
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// FileStreamHandle
	//----------------------------------------------------------------------------//

	class FileStreamHandle : public DataStreamHandle
	{
	public:
		FileStreamHandle(FILE* _handle, bool _readOnly) : m_handle(_handle), m_readOnly(_readOnly) 
		{
			ASSERT(_handle != nullptr); 
#if _MSC_VER < 1900
			m_size = _filelength(m_handle->_file);
#else
			fseek(m_handle, 0, SEEK_END);
			m_size = (uint)ftell(m_handle);
			fseek(m_handle, 0, SEEK_SET);
#endif
		}
		virtual ~FileStreamHandle(void) { fclose(m_handle); }

		void Seek(int _pos, int _origin) override { fseek(m_handle, _pos, _origin); }
		uint Tell(void) override { return (uint)ftell(m_handle); }
		bool EoF(void) override { return feof(m_handle) != 0; }
		uint Size(void) override { return m_size; }
		uint Read(void* _dst, uint _size) override
		{
			if (_size && _dst)
			{
#if _MSC_VER < 1900
				if (_size < (uint)m_handle->_cnt)
				{
					memcpy(_dst, m_handle->_ptr, _size);
					m_handle->_ptr += _size;
					m_handle->_cnt -= _size;
					return _size;
				}
#endif
				return fread(_dst, 1, _size, m_handle);
			}
			return 0;
		}
		uint Write(const void* _src, uint _size) override { return (!m_readOnly && _src && _size) ? fwrite(_src, 1, _size, m_handle) : 0; }
		void Flush(void) override { if (!m_readOnly) fflush(m_handle); }
		void* InternalData(const void* _src, uint _size) { return m_handle; };

	protected:
		FILE* m_handle;
		bool m_readOnly;
		uint m_size;
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

	FileSystem FileSystem::instance;

	//----------------------------------------------------------------------------//
	bool FileSystem::SetRoot(const String& _path)
	{
		if (!_path.empty())
		{
			String _fullPath = MakeFullPath(_path, m_appDir);
			struct stat _st;
			if (stat(_fullPath.c_str(), &_st) == 0 && (_st.st_mode & S_IFDIR))
			{
				m_root = _fullPath;
				if (m_root.back() != '/') m_root += "/";
				if (FS_IGNORE_CASE)	m_root = StrLower(m_root);
				chdir(m_root.c_str());
				return true;
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void FileSystem::AddSearchDir(const String& _path, bool _highPrio, bool _hidden)
	{
		if (!_path.empty())
		{
			SearchDir _sd;
			_sd.path = MakeNormPath(_path, false);
			_sd.fullPath = MakeFullPath(_path, m_root);
			if (FS_IGNORE_CASE) _sd.fullPath = StrLower(_sd.fullPath);
			_sd.hidden = _hidden;
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (i->fullPath == _sd.fullPath)
				{
					if (_highPrio)
					{
						m_dirs.erase(i);
						break;
					}
					else
					{
						i->hidden = _hidden;
						return;
					}
				}
			}
			_highPrio ? m_dirs.push_front(_sd) : m_dirs.push_back(_sd);
		}
	}
	//----------------------------------------------------------------------------//
	void FileSystem::RemoveSearchDir(const String& _path)
	{
		if (!_path.empty())
		{
			String _fullPath = MakeFullPath(_path, m_root);
			if (FS_IGNORE_CASE) _fullPath = StrLower(_fullPath);
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (i->fullPath == _fullPath)
				{
					m_dirs.erase(i);
					return;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetSearchDir(uint _index, SearchDir& _path)
	{
		if (_index < m_dirs.size())
		{
			auto _it = m_dirs.begin();
			while (_index--) ++_it;
			_path = *_it;
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	String FileSystem::Search(const String& _path, uint _mask)
	{
		if (!_path.empty())
		{
			uint _st_mode = 0;
			if (_mask & FF_FILE) _st_mode |= S_IFREG;
			if (_mask & FF_DIR) _st_mode |= S_IFDIR;

			// полный путь
			struct stat _st;
			if (IsFullPath(_path)) return stat(_path.c_str(), &_st) == 0 && (_st.st_mode & _st_mode) ? MakeNormPath(_path) : STR_BLANK;

			// в корневом каталоге
			String _fullPath = MakeFullPath(_path, m_root);
			if (stat(_fullPath.c_str(), &_st) == 0 && (_st.st_mode & _st_mode)) return FS_IGNORE_CASE ? StrLower(_fullPath) : _fullPath;

			// в каталогах поиска
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				_fullPath = MakeFullPath(_path, i->fullPath);
				if (stat(_fullPath.c_str(), &_st) == 0 && (_st.st_mode & _st_mode)) return FS_IGNORE_CASE ? StrLower(_fullPath) : _fullPath;
			}
		}
		return STR_BLANK;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::CreateDir(const String& _path)
	{
		LOG_ERROR("Couldn't create dir \"%s\"", _path.c_str());
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetInfo(const String& _path, FileInfo& _info, uint _mask)
	{
		_info.path = Search(_path, _mask);
		if (!_info.path.empty())
		{
			struct stat _st;
			if (stat(_info.path.c_str(), &_st) == 0)
			{
				struct tm _t;
				_t = *localtime(&_st.st_atime);
				_info.ltime = mktime(&_t);
				_t = *gmtime(&_st.st_atime);
				_info.gtime = mktime(&_t);
				_info.size = (uint)_st.st_size;
				_info.flags = (_st.st_mode & S_IFREG) ? FF_FILE : FF_DIR;
				return true;
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	DataStream FileSystem::Open(const String& _name)
	{
		String _nname = MakeNormPath(_name, false);
		if (!_nname.empty())
		{
			FILE* _handle = nullptr;
			String _path = Search(_name, FF_FILE);
			if (_path.empty())
			{
				LOG_ERROR("Couldn't search file \"%s\"", _nname.c_str());
			}
			else
			{
                _handle = fopen(_path.c_str(), "rb");
				if (!_handle)
				{
					LOG_ERROR("Couldn't open file \"%s\"", _nname.c_str());
				}
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, true) : nullptr, true);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//
	DataStream FileSystem::Create(const String& _name, bool _overwrite)
	{
		String _nname = MakeNormPath(_name, false);
		if (!_nname.empty())
		{

			String _path = MakeNormPath(IsFullPath(_nname) ? MakeFullPath(_nname, m_root) : _nname);
			CreateDir(FilePath(_path));
			FILE* _handle = fopen(_path.c_str(), _overwrite ? "wb" : "r+b");
			if (!_handle)
			{
				LOG_ERROR("Couldn't create file \"%s\"", _nname.c_str());
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, false) : nullptr, false);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//
	FileSystem::FileSystem(void)
	{
#	ifdef WIN32
		{
			char _buf[1024];
			uint _length = GetModuleFileNameA(0, _buf, sizeof(_buf));
			_buf[_length] = 0;
			String _name = MakeNormPath(_buf, false);
			SplitFilename(_name, nullptr, &m_appDir, &m_appName);
		}
#	else
#		error Unknown platform
#	endif

		if (FS_IGNORE_CASE) m_appDir = StrLower(m_appDir);
		if(m_appDir.back() != '/') m_appDir += "/";
		m_root = m_appDir;
		chdir(m_root.c_str());
	}
	//----------------------------------------------------------------------------//
	FileSystem::~FileSystem(void)
	{
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

}
