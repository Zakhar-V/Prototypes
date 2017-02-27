#pragma once

#include "Lib.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// NonCopyable
	//----------------------------------------------------------------------------//

	class NonCopyable
	{
	public:
		NonCopyable(void) { }
		~NonCopyable(void) { }

	private:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};

	//----------------------------------------------------------------------------//
	// Singleton
	//----------------------------------------------------------------------------//

	template <class T> class Singleton : public NonCopyable
	{
	public:
		Singleton(void) { ASSERT(s_instance == 0); s_instance = static_cast<T*>(this); }
		~Singleton(void) { s_instance = 0; }
		static T* Get(void) { return s_instance; }

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = 0;

	//----------------------------------------------------------------------------//
	// RefCounted
	//----------------------------------------------------------------------------//

	class RefCounted : public NonCopyable
	{
	public:
		RefCounted(void) : m_refCounter(0) { }
		int AddRef(void) const { return ++m_refCounter; }
		int Release(void) const { int _refCount = --m_refCounter; if (!_refCount) delete this; return _refCount; }
		int GetRefCount(void) const { return m_refCounter; }

	protected:
		virtual ~RefCounted(void) { }

		mutable int m_refCounter;
	};

	//----------------------------------------------------------------------------//
	// Ptr
	//----------------------------------------------------------------------------//

	template <class T> class Ptr
	{
	public:
		Ptr(void) : ptr(0) { }
		Ptr(const T* _p) : ptr(const_cast<T*>(_p)) { if (ptr) ptr->AddRef(); }
		Ptr(const Ptr& _p) : ptr(const_cast<T*>(_p.ptr)) { if (ptr) ptr->AddRef(); }
		~Ptr(void) { if (ptr) ptr->Release(); }
		Ptr& operator = (const Ptr& _p) { if (_p.ptr) _p.ptr->AddRef(); if (ptr) ptr->Release(); ptr = const_cast<T*>(_p.ptr); return *this; }
		Ptr& operator = (const T* _p) { if (_p) _p->AddRef(); if (ptr) ptr->Release(); ptr = const_cast<T*>(_p); return *this; }
		operator T* (void) const { return const_cast<T*>(ptr); }
		T* operator-> (void) const { return const_cast<T*>(ptr); }
		T& operator* (void) const { return *const_cast<T*>(ptr); }
		T* Get(void) const { return const_cast<T*>(ptr); }
		template <class X> X* StaticCast(void) const { return static_cast<X*>(const_cast<T*>(ptr)); }
		template <class X> X* DynamicCast(void) const { return dynamic_cast<X*>(const_cast<T*>(ptr)); }

	private:
		T* ptr;
	};

	//----------------------------------------------------------------------------//
	// LogNode
	//----------------------------------------------------------------------------//

#define LOG_INFO(msg, ...) LogNode::Message(0, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) LogNode::Message(1, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)	LogNode::Message(2, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) LogNode::Message(3, msg, ##__VA_ARGS__)
#define LOG_NODE(title, ...) LogNode _logNode_(true, __FUNCTION__, title, ##__VA_ARGS__)
#define LOG_NODE_DEFERRED(title, ...) LogNode _logNode_(false, __FUNCTION__, title, ##__VA_ARGS__)

	class LogNode final
	{
	public:
		LogNode(bool _writeNow, const char* _func, const char* _title, ...);
		~LogNode(void);

		static void Message(int _level, const char* _msg, ...);

	private:
		void _Write(void);

		LogNode* m_prev;
		const char* m_func;
		String m_title;
		bool m_written;
		int m_depth;

	private:
		LogNode(const LogNode&) = delete;
		LogNode& operator = (const LogNode&) = delete;
	};

	//----------------------------------------------------------------------------//
	// DataStreamHandle
	//----------------------------------------------------------------------------//

	class DataStreamHandle : public RefCounted
	{
	public:
		virtual void Seek(int _pos, int _origin) abstract;
		virtual uint Tell(void) abstract;
		virtual bool EoF(void) abstract;
		virtual uint Size(void) abstract;
		virtual uint Read(void* _dst, uint _size) abstract;
		virtual uint Write(const void* _src, uint _size) abstract;
		virtual void Flush(void) abstract;
		virtual void* InternalData(const void* _src, uint _size) { return nullptr; };
	};

	//----------------------------------------------------------------------------//
	// DataStream
	//----------------------------------------------------------------------------//

	class DataStream : public NonCopyable
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
		uint WriteString(const String& _str, bool _binary = true) { return Write(_str.empty() ? "" : _str.c_str(), _str.length() + (_binary ? 1 : 0)); }

	protected:
		String m_name;
		Ptr<DataStreamHandle> m_handle;
		uint m_size;
		bool m_readOnly;
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

#define GFileSystem FileSystem::Get()

	struct SearchDir
	{
		String path;
		String fullPath;
		bool hidden; // для просмотра ресурсов
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

	class FileSystem final : public Singleton < FileSystem >
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
		bool CreateDir(const String& _path);
		bool GetInfo(const String& _path, FileInfo& _info, uint _mask = FF_FILE);
		DataStream Open(const String& _name);
		DataStream Create(const String& _name, bool _overwrite = true);

	private:
		FileSystem(void);
		~FileSystem(void);

		String m_appDir;
		String m_appName;
		String m_root;
		List<SearchDir> m_dirs;

		static FileSystem instance;
	};

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	enum ResourceState : uint
	{
		RS_INITIAL,
		RS_UNLOADED,
		RS_QUEUED,
		RS_LOADING,
		RS_LOADED,
		RS_INVALID,
	};

	inline String MakeResourceName(const String& _name) { return MakeNormPath(_name, false); }
	inline uint32 MakeResourceId(const String& _name, bool _ignoreCase = FS_IGNORE_CASE) { return CRC32(MakeNormPath(_name, _ignoreCase)); }

	class Resource : public RefCounted
	{
	public:
		Resource(const String& _name = "") : m_name(MakeResourceName(_name)), m_id(MakeResourceId(_name)), m_state(RS_INITIAL) {}
		virtual ~Resource(void) { }
		const String& GetName(void) { return m_name; }
		const uint32 GetId(void) { return m_id; }
		ResourceState GetState(void) { return m_state; }
		bool IsUnloaded(void) { return m_state == RS_INITIAL || m_state == RS_UNLOADED; }
		bool IsLoading(void) { return m_state == RS_QUEUED || m_state == RS_LOADING; }
		bool IsLoaded(void) { return m_state == RS_LOADED || m_state == RS_INVALID; }
		bool IsValid(void) { return m_state == RS_LOADED; }

	protected:
		String m_name;
		uint32 m_id;
		volatile ResourceState m_state;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
