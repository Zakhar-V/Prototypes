#include "File.hpp"
#include "Log.hpp"
#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif
#include "PlatformIncludes.hpp"

namespace ge
{
	TODO_EX("File", "Refactoring");
	TODO_EX("File", "Zip archives");
	TODO_EX("File", "Thread-safe");

	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool IsFullPath(const char* _path)
	{
#ifdef _WIN32
		return _path && _path[1] == ':';
#else // linux
		return _path && _path[0] == '/';
#endif
	}
	//----------------------------------------------------------------------------//
	void SplitFilename(const String& _filename, String* _device, String* _dir, String* _name, String* _shortname, String* _ext)
	{
		uint _l = _filename.Length();
		if (_device)
		{
#ifdef _WIN32
			const char* c = strchr(_filename, ':');
			if (c)
				*_device = String(_filename, c);
#else
			const char* c = _filename.c_str();
			if (*c && strchr("/\\", *c))
			{
				while (*++c && !strchr("/\\", *c));
				*_device = String(_filename.Ptr(1), c);
			}
#endif
		}
		if (_dir)
		{
			uint c = _l;
			for (; c > 0 && strchr("/\\", _filename[c - 1]); --c);
			for (; c > 0 && !strchr("/\\", _filename[c - 1]); --c);
			*_dir = _filename.SubStr(0, c);
		}
		if (_name || _ext || _shortname)
		{
			uint _np = _l;
			for (; _np > 0 && !strchr("/\\", _filename[_np - 1]); --_np);
			String _n = _filename.SubStr(_np);
			if (_name)
				*_name = _n;
			if (_ext || _shortname)
			{
				uint _pp = _l - _np;
				for (; _pp > 0 && _n[_pp] != '.'; --_pp);
				if (_shortname)
					*_shortname = _pp > 0 ? _n.SubStr(0, _pp) : _n;
				if (_ext)
					*_ext = _pp > 0 ? _n.SubStr(_pp + 1) : String::Empty;
			}
		}
	}
	//----------------------------------------------------------------------------//
	void SplitPath(const char* _path, String* _device, StringArray* _items)
	{
		static const String _P1 = ".";
		static const String _P2 = "..";

		if (_path && *_path)
		{
			const char* p = _path;

#ifdef _WIN32
			const char* c = strchr(_path, ':');
			if (c)
			{
				if (_device)
					*_device = String(p, c);
				p = ++c;
			}
			else c = _path;
#else
			const char* c = _path;
			if (strchr("/\\", *c))
			{
				while (*++c && !strchr("/\\", *c));
				if (_device)
					*_device = String(p + 1, c);
				p = ++c;
			}
#endif
			if (_items)
			{
				_items->clear();
				_items->reserve(10);
				String _item;
				for (;;)
				{
					if (!*c || strchr("\\/", *c))
					{
						if (c != p)
						{
							_item = String(p, (uint)(c - p));
							if (_item == _P2)
							{
								if (_items->empty() || _items->back() == _P2)
									_items->push_back(_P2);
								else
									_items->pop_back();
							}
							else if (_item != _P1)
								_items->push_back(_item);
						}
						p = c + 1;
					}
					if (!*c++)
						break;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	String MakePath(const String& _device, const StringArray& _items)
	{
		String _path;
#ifdef _WIN32
		if (_device.NonEmpty())
			_path += _device + ":/";
#else
		if (_device.NonEmpty())
			_path += "/" + _device + "/";
#endif
		//if (_items.IsEmpty()) _path += ".";
		for (uint i = 0; i < _items.size(); ++i)
		{
			if (i > 0)
				_path += "/";
			_path += _items[i];
		}
		return _path;
	}
	//----------------------------------------------------------------------------//
	String MakeFullPath(const char* _path, const char* _root)
	{
		static const String _P1 = ".";
		static const String _P2 = "..";

		if (_path && *_path)
		{
			String _d, _pd;
			StringArray _p, _f;
			if (IsFullPath(_path) || !(_root && *_root))
			{
				char _last = _path[strlen(_path) - 1];
				if (!strchr(_path, '\\') &&
					!strstr(_path, "..") &&
					!strstr(_path, "./") &&
					!strstr(_path, "/.") &&
					!strstr(_path, "//") &&
					_last != '/')
					return _path;

				SplitPath(_path, &_pd, &_p);
				return MakePath(_pd, _p);
			}

			SplitPath(_path, &_pd, &_p);
			SplitPath(_root, &_d, &_f);
			for (uint i = 0; i < _p.size(); ++i)
			{
				if (_p[i] == _P2)
				{
					if (_f.empty() || _f.back() == _P2)
						_f.push_back(_P2);
					else
						_f.pop_back();
				}
				else if (_p[i] != _P1)
					_f.push_back(_p[i]);
			}
			return MakePath(_d, _f);
		}
		return String::Empty;
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
			_sd.path = MakeFullPath(_path);
			_sd.fullPath = MakeFullPath(_path, m_root);
			_sd.hidden = _hidden;
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (!String::Compare(i->fullPath, _sd.fullPath, FS_IGNORE_CASE))
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
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (!String::Compare(i->fullPath, _fullPath, FS_IGNORE_CASE))
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
		if (_path.NonEmpty())
		{
			uint _st_mode = 0;
			if (_mask & FF_FILE)
				_st_mode |= S_IFREG;
			if (_mask & FF_DIR) 
				_st_mode |= S_IFDIR;

			// full path
			struct stat _st;
			if (IsFullPath(_path)) 
				return stat(_path, &_st) == 0 && (_st.st_mode & _st_mode) ? MakeFullPath(_path) : String::Empty;

			// in directories
			String _fullPath, _npath = MakeFullPath(_path);
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				_fullPath = i->fullPath + "/" + _npath;
				if (stat(_fullPath, &_st) == 0 && (_st.st_mode & _st_mode)) 
					return _fullPath;
			}

			// in root directory
			_fullPath = m_root + "/" + _npath;
			if (stat(_fullPath, &_st) == 0 && (_st.st_mode & _st_mode)) 
				return _fullPath;
		}
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	String FileSystem::SearchFile(const String& _path, const StringArray& _dirs, const StringArray& _exts)
	{
		if (_path.NonEmpty())
		{
			// full path
			struct stat _st;
			if (IsFullPath(_path))
				return stat(_path.c_str(), &_st) == 0 && (_st.st_mode & S_IFREG) ? MakeFullPath(_path) : String::Empty;

			// in directories
			String _result, _npath = MakeFullPath(_path);;
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (_SearchFile(_npath, i->fullPath, _dirs, _exts, _result))
					return _result;
			}

			// in root directory
			//if (_SearchFile(_npath, m_root, _dirs, _exts, _result))
			//	return true;
		}
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::_SearchFile(const String& _path, const StringArray& _exts, String& _result)
	{
		struct stat _st;
		_result = _path;
		if (stat(_result, &_st) == 0 && (_st.st_mode & S_IFREG))
			return true;

		for (size_t i = 0, s = _exts.size(); i < s; ++i)
		{
			_result = _path + "." + _exts[i];
			if (stat(_result, &_st) == 0 && (_st.st_mode & S_IFREG))
				return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::_SearchFile(const String& _path, const String& _root, const StringArray& _dirs, const StringArray& _exts, String& _result)
	{
		struct stat _st;
		if (stat(_root, &_st) == 0 && (_st.st_mode & S_IFDIR))
		{
			if (_SearchFile(_root + "/" + _path, _exts, _result))
				return true;

			String _dir;
			for (size_t i = 0, s = _dirs.size(); i < s; ++i)
			{
				_dir = _root + "/" + _dirs[i];
				if (stat(_dir, &_st) == 0 && (_st.st_mode & S_IFDIR))
				{
					if (_SearchFile(_dir + "/" + _path, _exts, _result))
						return true;
				}
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::CreateDir(const String& _path)
	{
		LOG_ERROR("Couldn't create dir \"%s\"", _path.CStr());
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetInfo(const String& _path, FileInfo& _info, uint _mask)
	{
		_info.path = Search(_path, _mask);
		if (_info.path.NonEmpty())
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
		String _nname = MakeFullPath(_name);
		if (_nname.NonEmpty())
		{
			FILE* _handle = nullptr;
			String _path = Search(_name, FF_FILE);
			if (_path.IsEmpty())
			{
				LOG_ERROR("Couldn't search file \"%s\"", _nname.CStr());
			}
			else
			{
				_handle = fopen(_path, "rb");
				if (!_handle)
				{
					LOG_ERROR("Couldn't open file \"%s\"", _nname.CStr());
				}
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, true) : nullptr, true);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//
	DataStream FileSystem::Create(const String& _name, bool _overwrite)
	{
		String _nname = MakeFullPath(_name);
		if (_nname.NonEmpty())
		{
			String _path = IsFullPath(_nname) ? MakeFullPath(_nname, m_root) : _nname;
			CreateDir(FilePath(_path));
			FILE* _handle = fopen(_path, _overwrite ? "wb" : "r+b");
			if (!_handle)
			{
				LOG_ERROR("Couldn't create file \"%s\"", _nname.CStr());
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, false) : nullptr, false);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//
	FileSystem::FileSystem(void)
	{
#	ifdef _WIN32
	{
		char _buf[1024];
		uint _length = GetModuleFileNameA(0, _buf, sizeof(_buf));
		_buf[_length] = 0;
		SplitFilename(MakeFullPath(_buf), nullptr, &m_appDir, &m_appName);
	}
#	else
#		error Unknown platform
#	endif

		m_root = m_appDir;
		chdir(m_root);
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
