#pragma once

#include "String.hpp"
#include "Object.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gFileSystem ge::FileSystem::Get()
#ifdef _WIN32
#	define FS_IGNORE_CASE 1
#else
#	define FS_IGNORE_CASE 0
#endif

	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

	ENGINE_API bool IsFullPath(const char* _path);
	ENGINE_API inline bool IsFullPath(const String& _path) { return IsFullPath(_path.c_str()); }
	ENGINE_API void SplitFilename(const String& _filename, String* _device = nullptr, String* _dir = nullptr, String* _name = nullptr, String* _shortname = nullptr, String* _ext = nullptr);
	ENGINE_API void SplitPath(const char* _path, String* _device, StringArray* _items);
	ENGINE_API inline void SplitPath(const String& _path, String* _device, StringArray* _items) { SplitPath(_path.c_str(), _device, _items); }
	ENGINE_API String MakePath(const String& _device, const StringArray& _items);
	ENGINE_API String MakeFullPath(const char* _path, const char* _root = nullptr);
	ENGINE_API inline String MakeFullPath(const String& _path, const String& _root = String::Empty) { return MakeFullPath(_path.c_str(), _root.c_str()); }
	ENGINE_API inline String FilePath(const String& _filename) { String _r; SplitFilename(_filename, nullptr, &_r); return _r; }
	ENGINE_API inline String FileName(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, &_r); return _r; }
	ENGINE_API inline String FileNameOnly(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, &_r); return _r; }
	ENGINE_API inline String FileExt(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, nullptr, &_r); return _r; }

	//----------------------------------------------------------------------------//
	// DataStreamHandle
	//----------------------------------------------------------------------------//

	class ENGINE_API DataStreamHandle : public RefCounted
	{
	public:
		virtual void Seek(int _pos, int _origin) = 0;
		virtual uint Tell(void) = 0;
		virtual bool EoF(void) = 0;
		virtual uint Size(void) = 0;
		virtual uint Read(void* _dst, uint _size) = 0;
		virtual uint Write(const void* _src, uint _size) = 0;
		virtual void Flush(void) = 0;
		virtual void* InternalData(const void* _src, uint _size) { return nullptr; };
		//TODO: non-copyable
		//TODO: Add GetAccess(), IsInMemory(), Clone
	};

	//----------------------------------------------------------------------------//
	// DataStream
	//----------------------------------------------------------------------------//

	class ENGINE_API DataStream : public NonCopyable
	{
	public:
		DataStream(const String& _name, DataStreamHandle* _handle, bool _readOnly);
		DataStream(void);
		DataStream(DataStream&& _temp);
		~DataStream(void);
		DataStream& operator = (DataStream&& _temp);
		operator bool(void) const { return m_handle != nullptr; }
		const String& GetName(void) { return m_name; }
		uint GetSize(void) { return m_size; }
		void SetPos(int _pos, bool _relative = false);
		uint GetPos(void);
		void ToEnd(void);
		bool AtAnd(void);
		uint Read(void* _dst, uint _size);
		uint Write(const void* _src, uint _size);
		void Flush(void);
		String ReadString(int _maxLength = -1);
		uint WriteString(const String& _str, bool _binary = true) { return Write(_str.IsEmpty() ? "" : _str, _str.Length() + (_binary ? 1 : 0)); }

		// [not implemented yet]
		time_t GetFileTime(void) { return 0; }

	protected:
		String m_name;
		Ptr<DataStreamHandle> m_handle;
		uint m_size;
		bool m_readOnly; // TODO: use AccessMode
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

	struct SearchDir
	{
		String path;
		String fullPath;
		bool hidden; 
	};

	enum FileFlags
	{
		FF_FILE = 0x1,
		FF_DIR = 0x2,
		//FF_PACK = 0x4,
	};

	struct FileInfo
	{
		uint64 ltime;
		uint64 gtime;
		uint flags;
		uint size;
		String path; // full
	};

	class ENGINE_API FileSystem final : public Singleton < FileSystem >
	{
	public:

		const String& AppDir(void) { return m_appDir; }
		const String& AppName(void) { return m_appName; }
		const String& GetRoot(void) { return m_root; }
		bool SetRoot(const String& _path);
		void AddSearchDir(const String& _path, bool _highPrio = true, bool _hidden = false);
		void RemoveSearchDir(const String& _path);
		bool GetSearchDir(uint _index, SearchDir& _path);
		String Search(const String& _path, uint _mask = FF_FILE);
		String SearchFile(const String& _path, const StringArray& _dirs, const StringArray& _exts);
		bool CreateDir(const String& _path);
		bool GetInfo(const String& _path, FileInfo& _info, uint _mask = FF_FILE);
		DataStream Open(const String& _name);
		DataStream Create(const String& _name, bool _overwrite = true);

	private:
		FileSystem(void);
		~FileSystem(void);
		bool _SearchFile(const String& _path, const StringArray& _exts, String& _result);
		bool _SearchFile(const String& _path, const String& _root, const StringArray& _dirs, const StringArray& _exts, String& _result);

		String m_appDir;
		String m_appName;
		String m_root;
		List<SearchDir> m_dirs;

		static FileSystem instance;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
