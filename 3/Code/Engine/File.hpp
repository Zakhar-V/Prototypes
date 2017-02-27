#pragma once

#include "Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gFileSystem Engine::FileSystem::Get()
#ifdef _WIN32
#	define FS_IGNORE_CASE true
#else
#	define FS_IGNORE_CASE false
#endif

	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

	bool IsFullPath(const char* _path);
	void SplitFilename(const String& _filename, String* _device = nullptr, String* _dir = nullptr, String* _name = nullptr, String* _shortname = nullptr, String* _ext = nullptr);
	void SplitPath(const char* _path, String* _device, StringArray* _items);
	String MakePath(const String& _device, const StringArray& _items);
	String MakeFullPath(const char* _path, const char* _root = nullptr);
	inline String FilePath(const String& _filename) { String _r; SplitFilename(_filename, nullptr, &_r); return _r; }
	inline String FileName(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, &_r); return _r; }
	inline String FileNameOnly(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, &_r); return _r; }
	inline String FileExt(const String& _filename) { String _r; SplitFilename(_filename, nullptr, nullptr, nullptr, nullptr, &_r); return _r; }

	//----------------------------------------------------------------------------//
	// Config 
	//----------------------------------------------------------------------------//

	enum ConfigType : uint8
	{
		CT_Null,
		CT_Bool,
		CT_Number,
		CT_String,
		CT_Array,
		CT_Object,
	};

	class Config
	{
	public:

		struct Node
		{
			Node(void) { }
			Node(const Node& _other) : childs(_other.childs), names(_other.names), indices(_other.indices) { }
			Node(Node&& _temp) : childs(std::move(_temp.childs)), names(std::move(_temp.names)), indices(std::move(_temp.indices)) { }
			Node& operator = (const Node& _other) { childs = _other.childs; names = _other.names; indices = _other.indices; return *this; }
			Node& operator = (Node&& _temp) { childs = std::move(_temp.childs); names = std::move(_temp.names); indices = std::move(_temp.indices); return *this; }

			Array<Config> childs;
			HashMap<uint, std::pair<String, String>> names; // for simple and associative arrays <index in childs, pair<name,type>>
			HashMap<String, uint> indices; // for associative arrays  <name, index in childs>
		};

		Config(void) : m_type(CT_Null), m_raw(0) { }
		~Config(void);
		Config(const Config& _other);
		Config(Config&& _temp);
		Config(bool _value) : m_type(CT_Bool), m_num((double)_value) { }
		Config(int _value) : m_type(CT_Number), m_num((double)_value) { }
		Config(uint _value) : m_type(CT_Number), m_num((double)_value) { }
		Config(float _value) : m_type(CT_Number), m_num((double)_value) { }
		Config(double _value) : m_type(CT_Number), m_num((double)_value) { }
		Config(const char* _str) : m_type(CT_String), m_str(new String(_str)) { }
		Config(const String& _str) : m_type(CT_String), m_str(new String(_str)) { }

		Config& operator = (const Config& _other);
		Config& operator = (Config&& _temp);

		const Config& operator [] (const char* _name) const { return Child(_name); }
		const Config& operator [] (const String& _name) const { return Child(_name); }
		const Config& operator [] (int _index) const { return Child((uint)_index); }

		/// Get or add child. Becomes an object if was not an object or an array before.
		Config& operator () (const String& _name, const String& _type = String::Empty) { return Add(_name, _type); }
		/// Add new child to an array. Becomes an array if was not before.
		Config& operator += (const Config& _rhs) { Append() = _rhs; return *this; }
		/// Add new child to an array. Becomes an array if was not before.
		Config& operator += (Config&& _rhs) { Append() = Move(_rhs); return *this; }

		operator bool(void) const { return AsBool(); }
		operator char(void) const { return (char)AsNumber(); }
		operator int(void) const { return (int)AsNumber(); }
		operator uint(void) const { return (uint)AsNumber(); }
		operator float(void) const { return (float)AsNumber(); }
		operator double(void) const { return AsNumber(); }
		operator const String(void) const { return AsString(); }
		operator const char*(void) const { return AsString(); }

		ConfigType Type(void) const { return m_type; }
		bool IsNull(void) const { return m_type == CT_Null; }
		bool IsBool(void) const { return m_type == CT_Bool; }
		bool IsNumber(void) const { return m_type == CT_Number; }
		bool IsString(void) const { return m_type == CT_String; }
		bool IsArray(void) const { return m_type == CT_Array; }
		bool IsObject(void) const { return m_type == CT_Object; }
		bool IsNode(void) const { return m_type >= CT_Array; } //!<\return true if this is an array or an object

		///	Get value as boolean. Returns false if this is not a number or a boolean.
		bool AsBool(void) const { return (m_type == CT_Bool || m_type == CT_Number) ? m_num != 0 : false; }
		///	Get value as number. Returns zero if this is not a number or a boolean.
		double AsNumber(void) const { return (m_type == CT_Bool || m_type == CT_Number) ? m_num : 0.0; }
		///	Get value as string. Returns empty string if this is not string.
		const String& AsString(void) const { return m_type == CT_String ? *m_str : String::Empty; }

		/// Get existent child with name.
		Config* Search(const String& _name, uint* _index = nullptr) const;
		///	Get child.
		const Config& Child(const String& _name) const;
		///	Get child.
		const Config& Child(uint _index) const;
		///	Get name of child.
		const String& Name(uint _index) const;
		///	Get type of child.
		const String& Type(const String& _name) const;
		///	Get type of child.
		const String& Type(uint _index) const;
		///	Get size of an object or an array. Returns zero otherwise.
		uint Size(void) const { return (m_type >= CT_Array) ? (uint)m_node->childs.size() : 0; }

		/// Get or add child. Becomes an object if was not an object or an array before.
		Config& Add(const String& _name, const String& _type = String::Empty);
		/// Add new child to an array. Becomes an array if was not before.
		Config& Append(const String& _name = String::Empty, const String& _type = String::Empty);

		/// Change type to null.
		Config& SetNull(void);
		///	Set default value without changing type.
		Config& Clear(void);

		/// Create from string. Does not cleanup of content.
		bool Parse(const String& _str, String* _errorString = nullptr, int* _errorLine = nullptr);
		/// Save to string.
		String Print(void) const;

		static const Config Null;
		static const String Unnamed; //!< Default name of child of object. Value is "__unnamed".

	protected:

		union
		{
			void* m_ptr;
			uint64 m_raw;
			double m_num;
			String* m_str;
			Node* m_node;
		};
		ConfigType m_type;
	};

	//----------------------------------------------------------------------------//
	// DataStreamHandle
	//----------------------------------------------------------------------------//

	class DataStreamHandle : public NonCopyable
	{
	public:
		virtual ~DataStreamHandle(void) {}
		virtual AccessMode Access(void) = 0;
		//virtual time_t Time(void) = 0;
		virtual uint Size(void) = 0;
		virtual uint Tell(void) = 0;
		virtual bool EoF(void) = 0;
		virtual void Seek(int _pos, int _origin) = 0;
		virtual uint Read(void* _dst, uint _size) = 0;
		virtual uint Write(const void* _src, uint _size) = 0;
		virtual void Flush(void) = 0;
	};

	//----------------------------------------------------------------------------//
	// FileStreamHandle
	//----------------------------------------------------------------------------//

	class FileStreamHandle : public DataStreamHandle
	{
	public:
		FileStreamHandle(FILE* _handle, AccessMode _access);
		~FileStreamHandle(void);
		AccessMode Access(void) override { return m_access; }
		uint Size(void) override;
		uint Tell(void) override;
		bool EoF(void) override;
		void Seek(int _pos, int _origin) override;
		uint Read(void* _dst, uint _size) override;
		uint Write(const void* _src, uint _size)override;
		void Flush(void) override;

	protected:

		AccessMode m_access;
		uint m_size;
		FILE* m_handle;
	};

	//----------------------------------------------------------------------------//
	// MemoryStreamHandle
	//----------------------------------------------------------------------------//

	class MemoryStreamHandle : public DataStreamHandle
	{
		// not implemented yet
	};

	//----------------------------------------------------------------------------//
	// DataStream
	//----------------------------------------------------------------------------//

	class DataStream : public NonCopyable
	{
	public:
		DataStream(const String& _name, DataStreamHandle* _handle);
		DataStream(void);
		DataStream(DataStream&& _temp);
		~DataStream(void);
		DataStream& operator = (DataStream&& _temp);

		operator bool(void) const { return m_handle != nullptr; }
		
		AccessMode GetAccess(void) { return m_handle ? m_handle->Access() : AM_None; }
		const String& GetName(void) { return m_name; }
		//time_t GetFileTime(void) { return m_handle ? m_handle->Time() : 0; }
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

	protected:
		String m_name;
		DataStreamHandle* m_handle;
		uint m_size;
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
		FF_File = 0x1,
		FF_Dir = 0x2,
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

	class FileSystem final : public NonCopyable
	{
	public:
		static FileSystem* Get(void) { return &s_instance; }

		const String& AppDir(void) { return m_appDir; }
		const String& AppName(void) { return m_appName; }
		const String& GetRoot(void) { return m_root; }
		bool SetRoot(const String& _path);
		void AddSearchDir(const String& _path, bool _highPrio = true, bool _hidden = false);
		void RemoveSearchDir(const String& _path);
		bool GetSearchDir(uint _index, SearchDir& _path);
		String Search(const String& _path, uint _mask = FF_File);
		String SearchFile(const String& _path, const StringArray& _dirs, const StringArray& _exts);
		bool CreateDir(const String& _path);
		bool GetInfo(const String& _path, FileInfo& _info, uint _mask = FF_File);
		DataStream Open(const String& _name);
		DataStream Create(const String& _name, bool _overwrite = true);

	protected:
		FileSystem(void);
		~FileSystem(void);
		bool _SearchFile(const String& _path, const StringArray& _exts, String& _result);
		bool _SearchFile(const String& _path, const String& _root, const StringArray& _dirs, const StringArray& _exts, String& _result);

		String m_appDir;
		String m_appName;
		String m_root;
		List<SearchDir> m_dirs;

		static FileSystem s_instance;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
