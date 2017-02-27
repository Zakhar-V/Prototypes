#include "../File.hpp"
#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif
#ifdef _WIN32
#	include <Windows.h>
#endif

namespace Rx
{
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
	String FullPath(const char* _path, const char* _root)
	{
		static const String _P1 = ".";
		static const String _P2 = "..";

		if (_path)
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
	String ShortPath(const char* _path, const char* _root, bool _ignoreCase)
	{
		String _pd, _rd, _sp;
		StringArray _p, _r, _s;

		SplitPath(_path, &_pd, &_p);
		SplitPath(_root, &_rd, &_r);

		if (!_pd.Equals(_rd, _ignoreCase))
			return _path;

		uint _ps = (uint)_p.size();
		uint _rs = (uint)_r.size();
		uint _pos = 0;
		for (uint i = 0; i < _ps && i < _rs; ++i)
		{
			if (_p[_pos].Equals(_r[_pos], _ignoreCase))
				++_pos;
			else
				break;
		}

		for (uint i = _pos; i < _rs; ++i)
			_s.push_back("..");

		for (; _pos < _ps; ++_pos)
			_s.push_back(_p[_pos]);

		if (_s.empty())
			return ".";

		for (uint i = 0, _ss = (uint)_s.size(); i < _ss; ++i)
		{
			if (i > 0)
				_sp += "/";
			_sp += _s[i];
		}

		return _sp;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// File
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	File::File(void) :
		m_size(0),
		m_readOnly(true),
		m_handle(nullptr)
	{
	}
	//----------------------------------------------------------------------------//
	File::~File(void)
	{
		if (m_handle)
			delete m_handle;
	}
	//----------------------------------------------------------------------------//
	File::File(const String& _name, FileHandle* _handle) :
		m_name(_name),
		m_size(0),
		m_readOnly(true),
		m_handle(_handle)
	{
		if (m_handle)
		{
			m_readOnly = !(m_handle->Access() & AM_Write);
			m_size = m_handle->Size();
		}
	}
	//----------------------------------------------------------------------------//
	File::File(File&& _temp) :
		m_name(_temp.m_name),
		m_size(_temp.m_size),
		m_readOnly(_temp.m_readOnly),
		m_handle(_temp.m_handle)
	{
		_temp.m_handle = nullptr;
	}
	//----------------------------------------------------------------------------//
	File& File::operator = (File&& _temp)
	{
		Swap(m_handle, _temp.m_handle);
		m_size = _temp.m_size;
		m_name = _temp.m_name;
		m_readOnly = _temp.m_readOnly;
		return *this;
	}
	//----------------------------------------------------------------------------//
	uint File::GetSize(void)
	{
		return (m_handle && !m_readOnly) ? m_handle->Size() : m_size;
	}
	//----------------------------------------------------------------------------//
	void File::SetPos(int _pos, SeekOrigin _origin)
	{
		if (m_handle)
			m_handle->Seek(_pos, _origin);
	}
	//----------------------------------------------------------------------------//
	uint File::GetPos(void)
	{
		return m_handle ? m_handle->Tell() : 0;
	}
	//----------------------------------------------------------------------------//
	bool File::AtAnd(void)
	{
		return !m_handle || m_handle->Eof();
	}
	//----------------------------------------------------------------------------//
	uint File::Read(void* _dst, uint _size)
	{
		return m_handle ? m_handle->Read(_dst, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	uint File::Write(const void* _src, uint _size)
	{
		return m_handle ? m_handle->Write(_src, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	void File::Flush(void)
	{
		if (m_handle)
			m_handle->Flush();
	}
	//----------------------------------------------------------------------------//
	String File::ReadString(int _maxLength)
	{
		String _r;
		if (m_handle)
		{
			uint _fsize = GetSize();
			uint _readed, _length, _pos = GetPos(), _size = _maxLength < 0 ? _fsize : _maxLength;
			if (_pos > _fsize)
				_pos = _fsize;
			if (_size + _pos > _fsize)
				_size = _fsize - _pos;

			Array<char> _buf(_size + 1);
			_readed = Read(&_buf[0], _size);
			_buf[_readed] = 0;
			_length = (uint)strlen(&_buf[0]);
			_r = &_buf[0];

			if (_length + 1 < _readed)
				SetPos(_pos + _length + 1); // null-terminated string
		}
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// PhysFile
	//----------------------------------------------------------------------------//

	class PhysFile : public FileHandle
	{
	public:

		PhysFile(FILE* _handle, AccessMode _access) :
			m_handle(_handle),
			m_access(_access),
			m_size(0)
		{
			ASSERT(_handle != nullptr);
			ASSERT(_access != AM_None);
			if (!(m_access & AM_Write))
				m_size = _GetSize();
		}

		~PhysFile(void)
		{
			fclose(m_handle);
		}

		AccessMode Access(void) override
		{
			return m_access;
		}
		uint Size(void) override
		{
			return (m_access & AM_Write) ? _GetSize() : m_size;
		}
		uint Tell(void)	override
		{
			return (uint)ftell(m_handle);
		}
		void Seek(int _pos, int _origin = SO_Set) override
		{
			fseek(m_handle, _pos, _origin);
		}
		bool Eof(void) override
		{
			return feof(m_handle) != 0;
		}
		uint Read(void* _dst, uint _size) override
		{
			ASSERT(_dst || !_size);
			return (m_access & AM_Read) ? (uint)fread(_dst, 1, _size, m_handle) : 0;
		}
		uint Write(const void* _src, uint _size) override
		{
			ASSERT(_src || !_size);
			return (m_access & AM_Write) ? (uint)fwrite(_src, 1, _size, m_handle) : 0;
		}
		void Flush(void)
		{
			if (m_access & AM_Write)
				fflush(m_handle);
		}

	protected:

		uint _GetSize(void)	// max 2GB
		{
			long _size, _pos = ftell(m_handle);
			fseek(m_handle, 0, SEEK_END);
			_size = ftell(m_handle);
			fseek(m_handle, _pos, SEEK_SET);
			return (uint)_size;
		}

		FILE* m_handle;
		uint m_size;
		AccessMode m_access;
	};

	//----------------------------------------------------------------------------//
	// PhysFileSystem
	//----------------------------------------------------------------------------//
	
	class PhysFileSystem : public VirtualFileSystem
	{
	public:
		//CLASSNAME(PhysFileSystem);

		PhysFileSystem(void)
		{
		}
		~PhysFileSystem(void)
		{
		}

		bool Open(const String& _path) override
		{
			String _test = FullPath(_path, gFileSystem->RootDir());
			String _fpath = _test + "/";
			if (m_path.Equals(_fpath, true))
				return m_opened; // no changes

			Close();

			m_path = _fpath;

			struct stat _st;
			if (stat(_test, &_st) || !(_st.st_mode & S_IFDIR))
			{
				LOG_ERROR("Couldn't open PhysFileSystem '%s' : No directory", *m_path);
				m_opened = false;
				return false;
			}
				
			m_opened = true;
			return true;
		}
		void Close(void) override
		{
			m_opened = false;
		}

		bool FileExists(const String& _name) override
		{
			struct stat _st;
			return !stat(_name, &_st) && (_st.st_mode & S_IFREG);
		}
		FileHandle* OpenFile(const String& _name, uint _flags) override
		{
			if (_name.IsEmpty())
				return nullptr;

			FILE* _f = fopen(m_path + _name, "rb");
			return _f ? new PhysFile(_f, AM_Read) : nullptr;
		}

	protected:
	};

	//----------------------------------------------------------------------------//
	// ZipFileSystem
	//----------------------------------------------------------------------------//

	class ZipFileSystem : public VirtualFileSystem
	{
	public:
		//CLASSNAME(ZipFileSystem);

		// ...
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

	FileSystem FileSystem::s_instance;

	//----------------------------------------------------------------------------//
	FileSystem::FileSystem(void)
	{
#	ifdef _WIN32
		char _buf[4096];
		uint _length = GetModuleFileNameA(0, _buf, sizeof(_buf));
		_buf[_length] = 0;
		SplitFilename(FullPath(_buf), nullptr, &m_appDir, &m_appName, &m_appShortName);
#	else
		NOT_IMPLEMENTED_YET("Get app name");
#	endif

		chdir(m_appDir);
		m_appDir += "/";
		m_rootDir = m_appDir;
		m_cacheDirName = "Cache";
		m_cacheDir = m_rootDir + m_cacheDirName + "/";

		m_rootFs = new PhysFileSystem();
		m_rootFs->Open(m_rootDir);

		m_vfss.push_back({ ".", m_rootFs, 0 });

		TODO("use rwmutex");
	}
	//----------------------------------------------------------------------------//
	FileSystem::~FileSystem(void)
	{
		m_vfss.clear();
		m_rootFs = nullptr;
	}
	//----------------------------------------------------------------------------//
	String FileSystem::FindConfigFile(const String& _name)
	{
		String _nname = FullPath(_name);
		if (_nname.IsEmpty())
			_nname = m_appShortName + ".fs";

		String _testPath = _nname;
		String _path = _testPath;
		struct stat _st;
		if (!IsFullPath(_testPath))
		{
			for (uint i = 0; i < 3; ++i)
			{
				if (!stat(_path, &_st) && (_st.st_mode & S_IFREG))
					break;
				_testPath = "../" + _testPath;
				_path = FullPath(_testPath, m_appDir);
			}
		}

		if (stat(_path, &_st) || !(_st.st_mode & S_IFREG))
			String::Empty;

		return _path;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::LoadConfig(const String& _name)
	{
		String _fpath = FindConfigFile(_name);

		// open file
		FILE* _f = fopen(_fpath, "rb");
		if (!_f)
		{
			LOG_WARNING("Couldn't configure FileSystem : Couldn't open file '%s'", *_fpath);
			return false;
		}

		// read file
		fseek(_f, 0, SEEK_END);
		uint _size = (uint)ftell(_f);
		fseek(_f, 0, SEEK_SET);
		Array<char> _text(_size + 1);
		_size = fread(&_text[0], 1, _size, _f);
		_text[_size] = 0;
		fclose(_f);

		// parse file
		Config _cfg;
		String _errorString;
		int _errorLine;
		if (!_cfg.Parse(&_text[0], &_errorString, &_errorLine))
		{
			LOG_ERROR("Couldn't configure FileSystem :\n> %s(%d) : %s", *_fpath, *_errorString, _errorLine);
			return false;
		}

		SetConfig(_cfg, FilePath(_fpath));

		return true;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::SaveConfig(const String& _name, bool _useRelativePaths)
	{
		Config _cfg;
		String _fpath = FullPath(_name, m_appDir);
		GetConfig(_cfg, _useRelativePaths, FilePath(_fpath));

		FILE* _f = fopen(_fpath, "wb");
		if (!_f)
		{
			LOG_ERROR("Couldn't save FileSystem configuration to '%s'", *_fpath);
			return false;
		}

		String _text = _cfg.Print();
		fwrite(_text.Ptr(), 1, _text.Length(), _f);
		fclose(_f);

		return true;
	}
	//----------------------------------------------------------------------------//
	void FileSystem::SetConfig(const Config& _src, const String& _root)
	{
		String _fsRoot = FullPath(_root, m_appDir);

		SetRootDir(FullPath(_src["RootDir"], _fsRoot));
		SetCacheDir(_src["CacheDir"]);
		const Config& _paths = _src["Paths"];
		if (_paths.IsNode())
		{
			for (uint i = 0, n = _paths.Size(); i < n; ++i)
			{
				const Config& _item = _paths.Child(i);
				AddSearchPath(_item["Path"], _item["Priority"]);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void FileSystem::GetConfig(Config& _dst, bool _useRelativePaths, const String& _root)
	{
		String _fsRoot = FullPath(_root, m_appDir);
		_dst("RootDir") = _useRelativePaths ? ShortPath(m_rootDir, _fsRoot) : m_rootDir;
		_dst("CacheDir") = m_cacheDirName;

		Config& _paths = _dst.Add("Paths");
		for (Vfs& _vfs : m_vfss)
		{
			if (_vfs.fs != m_rootFs)
			{
				Config& _dir = _paths.Append();
				_dir("Path") = _vfs.name;
				_dir("Priority") = _vfs.priority;
			}
		}
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::SetRootDir(const String& _path)
	{
		String _fpath = FullPath(_path, m_appDir);
		
		struct stat _st;
		if (stat(_fpath, &_st) || !(_st.st_mode & S_IFDIR))
		{
			LOG_ERROR("Couldn't change root directory to '%s'", *_fpath);
			return false;
		}

		if (!m_rootFs->Open(_fpath))
		{
			LOG_ERROR("Couldn't open root directory '%s'", *_fpath);
			return false;
		}
		LOG_EVENT("Root directory was changed to '%s'", *_fpath);

		chdir(_fpath);
		m_rootDir = _fpath + "/";
		m_cacheDir = FullPath(m_cacheDirName, m_rootDir) + "/";

		for (Vfs& _vfs : m_vfss)
		{
			if (_vfs.fs != m_rootFs)
				_ReopenVfs(_vfs);
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	void FileSystem::SetCacheDir(const String& _path)
	{
		m_cacheDirName = FullPath(_path);
		m_cacheDir = FullPath(m_cacheDirName, m_rootDir) + "/";
	}
	//----------------------------------------------------------------------------//
	void FileSystem::AddSearchPath(const String& _path, int _priority)
	{
		String _nname = FullPath(_path);
		String _fpath = FullPath(_nname, m_rootDir);
		if (_nname.IsEmpty())
			return;

		for (Vfs& _vfs : m_vfss)
		{
			if (_vfs.name.Equals(_nname, true) || _vfs.fs->GetPath().Equals(_fpath, true))
				return;	// already exists. priority cannot be changed.
		}

		Vfs _fs(_nname, nullptr, _priority);
		_ReopenVfs(_fs); // open or create

		for (auto i = m_vfss.begin();; ++i)
		{
			if (i == m_vfss.end())
			{
				m_vfss.push_back(_fs);
				break;
			}
			if (i->priority <= _priority)
			{
				m_vfss.insert(i, _fs);
				break;
			}
		}
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::FileExists(const String& _path)
	{
		if (_path.IsEmpty())
			return false;

		String _npath = FullPath(_path);

		if (IsFullPath(_npath))
		{
			struct stat _st;
			return !stat(_npath, &_st) && (_st.st_mode & S_IFREG);
		}

		for (auto& _vfs : m_vfss)
		{
			if (_vfs.fs->FileExists(_npath))
				return true;
		}

		return false;
	}
	//----------------------------------------------------------------------------//
	File FileSystem::OpenFile(const String& _path, AccessMode _access)
	{
		if (_path.IsEmpty() || _access == AM_None)
			return File(_path, nullptr);

		String _npath = FullPath(_path, (_access & AM_Write) ? m_rootDir : nullptr);

		if (IsFullPath(_npath))
		{
			FILE* _f;
			if (_access & AM_Write)
			{
				TODO("create directory");

				_f = fopen(_path, (_access & AM_Read) ? "w+b" : "wb");
				if (!_f)
				{
					LOG_ERROR("Couldn't create file '%s'", *_path);
				}
			}
			else
			{
				_f = fopen(_path, "rb");
				if (!_f)
				{
					LOG_ERROR("Couldn't open file '%s': File not was found", *_path);
				}
			}

			return File(_path, _f ? new PhysFile(_f, _access) : nullptr);
		}

		FileHandle* _fh = nullptr;
		for (auto& _vfs : m_vfss)
		{
			_fh = _vfs.fs->OpenFile(_npath);
			if (_fh)
				break;
		}

		if (!_fh)
		{
			LOG_ERROR("Couldn't open file '%s': File not was found", *_path);
		}

		return File(_npath, _fh);
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetPhysFileInfo(FileInfo& _fi, const String& _path)
	{
		_fi.path = FullPath(_path, m_rootDir);
		_finddata_t _fd;
		intptr_t _fh = _findfirst(_fi.path, &_fd);
		if (_fh == -1)
			return false;

		_fi.attribs = (_fd.attrib & _A_HIDDEN) ? FA_Hidden : 0;
		_fi.attribs |= (_fd.attrib & _A_SUBDIR) ? FA_Directory : FA_File;
		_fi.name = _fd.name;
		_fi.time = _fd.time_write;
		_fi.size = (uint)_fd.size;

		_findclose(_fh);
		return true;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::EnumPhysFiles(FileInfoList& _dst, const String& _dir, uint _attribMask, const String& _nameMask)
	{
		FileInfo _fi;
		_finddata_t _fd;
		String _fpath = FullPath(_dir, m_rootDir) + "/";
		intptr_t _fh = _findfirst(_fpath + "*", &_fd);
		if (_fh == -1)
			return false;

		bool _isDir, _isHidden;
		bool _enumFiles = (_attribMask & FA_File) != 0;
		bool _enumDirs = (_attribMask & FA_Directory) != 0;
		bool _enumHidden = (_attribMask & FA_Hidden) != 0;

		while (!_findnext(_fh, &_fd))
		{
			_isDir = (_fd.attrib & _A_SUBDIR) != 0;
			_isHidden = (_fd.attrib & _A_HIDDEN) != 0;

			if (_isDir ? !_enumDirs : !_enumFiles)
				continue;
			if (_isHidden && !_enumHidden)
				continue;
			if (strcmp(_fd.name, ".") && strcmp(_fd.name, ".."))
				continue;
			if (_nameMask.NonEmpty() && !_nameMask.Match(_fd.name))
				continue;

			_fi.attribs = _isHidden ? FA_Hidden : 0;
			_fi.attribs |= _isDir ? FA_Directory : FA_File;
			_fi.name = _fd.name;
			_fi.time = _fd.time_write;
			_fi.path = _fpath + _fi.name;
			_fi.size = (uint)_fd.size;
		}

		_findclose(_fh);
		return true;
	}
	//----------------------------------------------------------------------------//
	void FileSystem::_ReopenVfs(Vfs& _vfs)
	{
		TODO("Поддержка архивов");

		String _fpath = FullPath(_vfs.name, m_rootDir);

		struct stat _st;
		if (stat(_fpath, &_st))
		{
			if (!_vfs.fs)
			{
				_vfs.fs = new PhysFileSystem();
			}

			if (_vfs.fs)
			{
				_vfs.fs->Close();
			}
			LOG_ERROR("Couldn't open virtual file system '%s' : No file", *_fpath);
			return;
		}

		if (_st.st_mode & _S_IFDIR)
		{
			/*if (!_vfs.fs || !_vfs.fs->IsClass<PhysFileSystem>())
			{
				_vfs.fs = new PhysFileSystem();
			} */
		}
		else
		{
			if (!_vfs.fs)
			{
				_vfs.fs = new PhysFileSystem();
			}
			//TODO: classify type of archive
			ASSERT(false, "ToDo");
		}

		_vfs.fs->Open(_fpath);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
