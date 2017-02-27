#pragma once

#include "Base.hpp"
#include "Object.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//
#ifdef _WIN32
#	define FS_IGNORE_CASE true
#else
#	define FS_IGNORE_CASE false
#endif
#define gFileSystem Engine::FileSystem::Get()

	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

	bool IsFullPath(const char* _path);
	void SplitFilename(const String& _filename, String* _device = nullptr, String* _dir = nullptr, String* _name = nullptr, String* _shortname = nullptr, String* _ext = nullptr);
	void SplitPath(const char* _path, String* _device, StringArray* _items);
	String MakePath(const String& _device, const StringArray& _items);
	String FullPath(const char* _path, const char* _root = nullptr);
	String ShortPath(const char* _path, const char* _root, bool _ignoreCase = FS_IGNORE_CASE);
	inline String FilePath(const String& _filename) { String _r; SplitFilename(_filename, nullptr, &_r); return _r; }
	inline String FileName(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, &_r); return _r; }
	inline String FileNameOnly(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, &_r); return _r; }
	inline String FileExt(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, nullptr, &_r); return _r; }

	//----------------------------------------------------------------------------//
	// FileInfo
	//----------------------------------------------------------------------------//

	enum FileAttribs : uint
	{
		FA_File = 0x1,
		FA_Directory = 0x2,
		FA_Hidden = 0x4,
		FA_All = FA_Hidden | FA_File | FA_Directory,
	};

	struct FileInfo
	{
		uint attribs;
		uint size;
		time_t time; // global
		String name; // name only
		String path; // full path
	};

	typedef Array<FileInfo> FileInfoList;

	//----------------------------------------------------------------------------//
	// FileHandle
	//----------------------------------------------------------------------------//

	enum SeekOrigin : int
	{
		SO_Set = SEEK_SET,
		SO_Current = SEEK_CUR,
		SO_End = SEEK_END,
	};

	class FileHandle : public NonCopyable
	{
	public:
		virtual ~FileHandle(void) { }
		virtual bool IsInMemory(void) { return false; }
		virtual AccessMode Access(void) = 0;
		virtual uint Size(void) = 0;
		virtual uint Tell(void) = 0;
		virtual void Seek(int _pos, int _origin = SO_Set) = 0;
		virtual bool Eof(void) = 0;
		virtual uint Read(void* _dst, uint _size) = 0;
		virtual uint Write(const void* _src, uint _size) = 0;
		virtual void Flush(void) = 0;
	};

	//----------------------------------------------------------------------------//
	// File
	//----------------------------------------------------------------------------//

	class File : public NonCopyable
	{
	public:
		File(void);
		~File(void);
		File(const String& _name, FileHandle* _handle);
		File(File&& _temp);
		File& operator = (File&& _temp);

		operator bool(void) const { return m_handle != nullptr; }

		AccessMode GetAccess(void) { return m_handle ? m_handle->Access() : AM_None; }
		const String& GetName(void) { return m_name; }
		FileHandle* GetHandle(void) { return m_handle; }
		uint GetSize(void);
		void SetPos(int _pos, SeekOrigin _origin = SO_Set);
		uint GetPos(void);
		bool AtAnd(void);
		uint Read(void* _dst, uint _size);
		uint Write(const void* _src, uint _size);
		void Flush(void);
		String ReadString(int _maxLength = -1);
		uint WriteString(const String& _str, bool _binary = true) { return Write(_str.IsEmpty() ? "" : _str, _str.Length() + (_binary ? 1 : 0)); }

	protected:
		String m_name;
		uint m_size;
		FileHandle* m_handle;
		bool m_readOnly;
	};

	//----------------------------------------------------------------------------//
	// VirtualFileSystem
	//----------------------------------------------------------------------------//

	class VirtualFileSystem	: public RefCounted
	{
	public:
		CLASSNAME("VirtualFileSystem");

		const String& GetPath(void) { return m_path; }

		virtual bool Open(const String& _path) = 0;
		virtual void Close(void) = 0;
		bool IsOpened(void) { return m_opened; }

		///\return true if file exists
		///\param[in] _name is normalized relative path.
		virtual bool FileExists(const String& _name) = 0;
		///\param[in] _name is normalized relative path.
		virtual FileHandle* OpenFile(const String& _name, uint _flags = 0) = 0;

	protected:
		String m_path;
		bool m_opened = false;
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

	class FileSystem final : public NonCopyable
	{
	public:

		static FileSystem* Get(void) { return &s_instance; }

		String FindConfigFile(const String& _name = String::Empty);
		bool LoadConfig(const String& _name);
		bool SaveConfig(const String& _name, bool _useRelativePaths = true);
		void SetConfig(const Config& _src, const String& _root = String::Empty);
		void GetConfig(Config& _dst, bool _useRelativePaths = true, const String& _root = String::Empty);

		const String& AppDir(void) { return m_appDir; }
		const String& AppName(void) { return m_appName; }
		const String& AppShortName(void) { return m_appShortName; }
		bool SetRootDir(const String& _path);
		const String& RootDir(void) { return m_rootDir; }
		void SetCacheDir(const String& _path);
		const String& CacheDir(void) { return m_cacheDir; }

		void AddSearchPath(const String& _path, int _priority = 0);

		bool FileExists(const String& _path);
		File OpenFile(const String& _path, AccessMode _access = AM_Read);

		bool GetPhysFileInfo(FileInfo& _fi, const String& _path);
		bool EnumPhysFiles(FileInfoList& _dst, const String& _dir, uint _attribMask = FA_All, const String& _nameMask = String::Empty);

	protected:

		struct Vfs
		{
			Vfs(const String& _name, VirtualFileSystem* _fs, int _priority) : name(FullPath(_name)), fs(_fs), priority(_priority) { }
			bool operator < (const Vfs& _rhs) const { return priority >= _rhs.priority; }

			String name;
			Ptr<VirtualFileSystem> fs;
			int priority = 0;
		};

		FileSystem(void);
		~FileSystem(void);
		void _ReopenVfs(Vfs& _vfs);

		String m_appDir;
		String m_appName;
		String m_appShortName;
		String m_rootDir;
		String m_cacheDirName;
		String m_cacheDir;
		List<Vfs> m_vfss;
		Ptr<VirtualFileSystem> m_rootFs;

		static FileSystem s_instance;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
