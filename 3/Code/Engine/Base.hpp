#pragma once

//----------------------------------------------------------------------------//
// Compiler and Platform
//----------------------------------------------------------------------------//

#define _QUOTE( x ) #x
#define _QUOTE_IN_PLACE( x ) _QUOTE( x )
#define __FILELINE__ __FILE__"(" _QUOTE_IN_PLACE( __LINE__ ) ")"

#ifdef _MSC_VER
#   define COMPILER_MESSAGE(_prefix, _message) __pragma( message( __FILELINE__ " : "_prefix ": " _message ) )
#   define DEPRECATED __declspec( deprecated( "It will be removed or changed in closest time" ) )
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS
#   endif
#   define PACK __pragma( pack( push, 1 ) )
#   define PACKED
#   define UNPACK ;__pragma( pack( pop ) )
#	define THREAD_LOCAL __declspec(thread)
#	define NOINLINE __declspec(noinline)
#	pragma warning( disable : 4251 ) // dll interface
#	pragma warning( disable : 4275 ) // dll interface
#	pragma warning( disable : 4201 ) // unnamed union
#	pragma warning( disable : 4100 ) // unused arg
#	pragma warning(disable : 4996)	// The POSIX name
#elif defined(__GNUC__)
#   define COMPILER_MESSAGE(_prefix, _message) __pragma( message( __FILELINE__ " : "_prefix ": " _message ) )
#   define DEPRECATED __declspec( deprecated( "It will be removed or changed in closest time" ) )
#   define PACK
#   define PACKED __attribute__((packed))
#   define UNPACK
#   define THREAD_LOCAL __thread
#	define NOINLINE __attribute__((noinline))
#	define abstract =0
#else
#   define COMPILER_MESSAGE(_prefix, _message)
#   define DEPRECATED
#   define PACK
#   define PACKED
#   define UNPACK
#   define THREAD_LOCAL
#	define NOINLINE
#endif

#define COMPILER_MESSAGE_EX(_prefix, _source, _message) COMPILER_MESSAGE(_prefix, _source " : " _message)
#define WARNING_EX(_source, _message) COMPILER_MESSAGE_EX("Warning", _source, _message)
#define WARNING(_message) WARNING_EX(__FUNCTION__, _message)
#define FIXME_EX(_source, _message) COMPILER_MESSAGE_EX("FixMe", _source, _message)
#define FIXME(_message) FIXME_EX(__FUNCTION__, _message)
#define TODO_EX(_source, _message) COMPILER_MESSAGE_EX("ToDo", _source, _message)
#define TODO(_message) TODO_EX(__FUNCTION__, _message)
#define NOT_IMPLEMENTED_YET() FIXME("Not implemented yet")
#define NOT_IMPLEMENTED_YET_EX(_source) FIXME_EX(_source, "Not implemented yet")

//----------------------------------------------------------------------------//
// Assert
//----------------------------------------------------------------------------//

#if defined(_DEBUG) && !defined(DEBUG)
#	define DEBUG
#endif
#if !defined(DEBUG) && !defined(NDEBUG)
#	define NDEBUG
#endif
#ifdef _DEBUG
//#	include <assert.h>
//#	define ASSERT(x, ...) assert(x && ##__VA_ARGS__ "")
#	define ASSERT(x, ...) ((x) ? ((void)0) : (Engine::LogMsg(Engine::LL_Assert, __FUNCTION__, __FILE__, __LINE__, #x " (" ##__VA_ARGS__ ")")))
#else
#	define ASSERT(x, ...)
#endif

//----------------------------------------------------------------------------//
// Includes
//----------------------------------------------------------------------------//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Basic types
	//----------------------------------------------------------------------------//

	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int64_t int64;
	typedef uint64_t uint64;
	typedef unsigned int uint;

	template <class T> using Array = std::vector < T >;
	//template <class T, uint C> using FixedArray = std::array < T, C >;
	template <class T> using List = std::list < T >;
	//template <class K, class V> using Map = std::map < K, V >;
	template <class K, class V> using HashMap = std::unordered_map < K, V >;
	template <class T> using HashSet = std::unordered_set < T >;

	class String;
	typedef Array<String> StringArray;

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	enum AccessMode : uint8
	{
		AM_None = 0,
		AM_Read = 0x1,
		AM_Write = 0x2,
		AM_ReadWrite = AM_Read | AM_Write,
	};

	int32 AtomicAdd(volatile int32& _atomic, int32 _value); // in Thread.cpp
	void AtomicLock(volatile int& _atomic); // in Thread.cpp
	bool AtomicTryLock(volatile int& _atomic); // in Thread.cpp
	void AtomicUnlock(volatile int& _atomic); // in Thread.cpp

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

	template <class T> struct RRef { typedef T Type; };
	template <class T> struct RRef<T&> { typedef T Type; };
	template <class T> struct RRef<T&&> { typedef T Type; };

	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type& _ref) { return static_cast<T&&>(_ref); }
	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type&& _ref) { return static_cast<T&&>(_ref); }
	template <class T> inline typename RRef<T>::Type&& Move(T&& _ref) { return static_cast<RRef<T>::Type&&>(_ref); }

	template <typename T> void Swap(T& _a, T& _b)
	{
		T _c = Move(_a);
		_a = Move(_b);
		_b = Move(_c);
	}

	//----------------------------------------------------------------------------//
	// Log
	//----------------------------------------------------------------------------//

#define LOG_MSG(level, msg, ...) Engine::LogMsg(level, __FUNCTION__, __FILE__, __LINE__, Engine::String::Format(msg, ##__VA_ARGS__))

	enum LogLevel : int
	{
		LL_Assert = 0,
		LL_Fatal,
		LL_Error,
		LL_Warning,
		LL_Event,
		LL_Info,
		LL_Debug,
	};

	void LogMsg(int _level, const char* _func, const char* _file, int _line, const char* _msg);	// in Logger.cpp

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	inline char ToLower(char _ch) { return ((_ch >= 'A' && _ch <= 'Z') || (_ch >= 'a' && _ch <= 'z') || ((uint8)_ch >= 0xc0)) ? (_ch | 0x20) : _ch; }
	inline char ToUpper(char _ch) { return ((_ch >= 'A' && _ch <= 'Z') || (_ch >= 'a' && _ch <= 'z') || ((uint8)_ch >= 0xc0)) ? (_ch & ~0x20) : _ch; }

	class String
	{
	public:
		String(void) : m_buffer(_Null()) { }
		~String(void) { _Release(m_buffer); }
		String(const String& _other) : m_buffer(_AddRef(_other.m_buffer)) { }
		String(int _ch) : m_buffer(_ch ? _New((const char*)&_ch, 1) : _Null()) { }
		String(char _ch) : m_buffer(_ch ? _New(&_ch, 1) : _Null()) { }
		String(const char* _str, int _length = -1) : m_buffer(_New(_str, _Length(_str, _length))) { }
		String(const char* _first, const char* _last) : m_buffer(_first == _last ? _Null() : _New(_first, (uint)(_last - _first))) { }
		String(uint _count, char _ch) : m_buffer(_New(nullptr, _count)) { while (_count--) m_buffer->str[_count] = _ch; }

		String& operator = (const String& _str) { _AddRef(_str.m_buffer); _Release(m_buffer); m_buffer = _str.m_buffer; return *this; }
		String& operator = (const char* _str) { return Clear().Append(_str, -1, false); }
		String& operator = (char _ch) { return Clear().Append(&_ch, 1, false); }

		operator const char* (void) const { return m_buffer->str; }
		char operator [] (int _idx) const { return ((uint)_idx) < m_buffer->length ? m_buffer->str[_idx] : 0; }
		const char* operator * (void) const { return m_buffer->str; }

		bool operator == (const String& _lhs) const { return m_buffer == _lhs.m_buffer || (m_buffer->length == _lhs.m_buffer->length && Compare(m_buffer->str, _lhs.m_buffer->str) == 0); }
		bool operator == (const char* _lhs) const { return Compare(m_buffer->str, _lhs) == 0; }
		bool operator != (const String& _lhs) const { return !(*this == _lhs); }
		bool operator != (const char* _lhs) const { return !(*this == _lhs); }
		bool operator < (const String& _lhs) const { return Compare(m_buffer->str, _lhs.m_buffer->str) < 0; }
		bool operator < (const char* _lhs) const { return Compare(m_buffer->str, _lhs) < 0; }
		bool operator <= (const String& _lhs) const { return Compare(m_buffer->str, _lhs.m_buffer->str) <= 0; }
		bool operator <= (const char* _lhs) const { return Compare(m_buffer->str, _lhs) <= 0; }
		bool operator > (const String& _lhs) const { return Compare(m_buffer->str, _lhs.m_buffer->str) > 0; }
		bool operator > (const char* _lhs) const { return Compare(m_buffer->str, _lhs) > 0; }
		bool operator >= (const String& _lhs) const { return Compare(m_buffer->str, _lhs.m_buffer->str) >= 0; }
		bool operator >= (const char* _lhs) const { return Compare(m_buffer->str, _lhs) >= 0; }

		String& operator += (const String& _lhs) { return Append(_lhs); }
		String& operator += (char _lhs) { return Append(_lhs); }
		String& operator += (const char* _lhs) { return Append(_lhs); }

		String operator + (const String& _lhs) const { return String(*this).Append(_lhs); }
		String operator + (char _lhs) const { return String(*this).Append(_lhs); }
		String operator + (const char* _lhs) const { return String(*this).Append(_lhs); }
		friend String operator+ (char _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }
		friend String operator+ (const char* _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }

		String& Clear(void);
		String& Reserve(uint _size);
		String& Append(char _ch) { return _ch ? Append(&_ch, 1) : *this; }
		String& Append(uint _count, char _ch);
		String& Append(const String& _str) { return Append(_str.m_buffer->str, _str.m_buffer->length); }
		String& Append(const char* _str, int _length = -1, bool _quantizeMemory = true);
		String& Append(const char* _first, const char* _last) { ASSERT(_first <= _last); return Append(_first, (uint)(_last - _first)); }
		String SubStr(uint _offset, int _length = -1) const
		{
			ASSERT(_length < 0 || (uint)(_offset + _length) < m_buffer->length);
			return String(m_buffer->str + _offset, _length);
		}

		bool IsEmpty(void) const { return m_buffer->length == 0; }
		bool NonEmpty(void) const { return m_buffer->length != 0; }
		uint Length(void) const { return m_buffer->length; }
		char At(uint _idx) const { return _idx < m_buffer->length ? m_buffer->str[_idx] : 0; }
		char Back(void) const { return m_buffer->length > 1 ? m_buffer->str[m_buffer->length - 1] : 0; }
		const char* CStr(void) const { return m_buffer->str; }
		const char* Ptr(void) const { return m_buffer->str; }
		const char* Ptr(uint _offset) const { ASSERT(_offset < m_buffer->length); return m_buffer->str + _offset; }
		uint32 Hash(uint32 _hash = 0) const { return Hash(m_buffer->str, _hash); }
		uint32 Hashi(uint32 _hash = 0) const { return Hashi(m_buffer->str, _hash); }
		StringArray Split(const char* _delimiters) const { StringArray _dst; Split(m_buffer->str, _delimiters, _dst); return Move(_dst); }
		bool Equals(const char* _rhs, bool _ignoreCase = false) const { return Compare(*this, _rhs, _ignoreCase) == 0; }
		String Trim(const char* _cset, bool _left = true, bool _right = true) const
		{
			if (!_cset)
				return *this;

			const char* _s = m_buffer->str;
			const char* _e = m_buffer->str + m_buffer->length;
			while (_left && _s > _e && strchr(_cset, *_s))
				++_s;
			while (_right && _s > _e && strchr(_cset, *_e))
				--_e;

			uint _length = (uint)(_e - _s);
			if (_length == m_buffer->length)
				return *this;
			if (_length == 0)
				return Empty;
			return String(_s, _e);
		}

		static String Format(const char* _fmt, ...);
		static String FormatV(const char* _fmt, va_list _args);
		static uint32 Hash(const char* _str, uint32 _hash)
		{
			if (_str) while (*_str)
				_hash = *_str++ + (_hash << 6) + (_hash << 16) - _hash;
			return _hash;
		}
		static uint32 Hashi(const char* _str, uint32 _hash)
		{
			if (_str) while (*_str)
				_hash = ToLower(*_str++) + (_hash << 6) + (_hash << 16) - _hash;
			return _hash;
		}
		static int Compare(const char* _a, const char* _b, bool _ignoreCase = false)
		{
			_a = _a ? _a : "";
			_b = _b ? _b : "";
			if (_ignoreCase)
			{
				while (*_a && ToLower(*_a++) == ToLower(*_b++));
				return *_a - *_b;
			}
			return strcmp(_a, _b);
		}
		static const char* Find(const char* _str1, const char* _str2, bool _ignoreCase = false)
		{
			_str1 = _str1 ? _str1 : "";
			_str2 = _str2 ? _str2 : "";
			if (_ignoreCase)
			{
				for (; *_str1; *_str1++) 
				{
					for (const char *a = _str1, *b = _str2; ToLower(*a++) == ToLower(*b++);)
					{
						if (!*b)
							return _str1;
					}
				}
				return nullptr;
			}
			return strstr(_str1, _str2);
		}
		static void Split(const char* _str, const char* _delimiters, StringArray& _dst);

		static uint GetTotalMemoryUse(void) { return s_memory; }

		static const String Empty;

	protected:

		struct Buffer
		{
			Buffer(uint _size, uint _length) : refs(1), size(_size), length(_length) { str[_length] = 0; }
			volatile int refs;
			uint size;
			uint length;
			char str[1];
		};

		static Buffer* _Null(void);						 
		static Buffer* _New(const char* _str, uint _length, uint _size = 0);
		static Buffer* _Unique(Buffer*& _buffer, uint _newLength, bool _quantizeMemory);
		static Buffer* _AddRef(Buffer* _buffer)
		{
			AtomicAdd(_buffer->refs, 1);
			return _buffer;
		}
		static void _Release(Buffer* _buffer, int _refs = -1)
		{
			ASSERT(_refs < 0);
			if (AtomicAdd(_buffer->refs, _refs) == 0)
			{
				AtomicAdd(s_memory, -int(_buffer->size));
				delete[] reinterpret_cast<uint8*>(_buffer);
			}
		}
		static uint _Length(const char* _str, int _length)
		{
			return _str ? (_length < 0 ? (uint)strlen(_str) : _length) : 0;
		}

		Buffer* m_buffer;
		static volatile int s_memory;
	};

	//----------------------------------------------------------------------------//
	// NameHash 
	//----------------------------------------------------------------------------//

	struct NameHash
	{
		NameHash(void) : hash(0) { }
		NameHash(uint32 _value) : hash(_value) { }
		NameHash(const String& _str, uint32 _hash = 0) : hash(String::Hashi(_str, _hash)) { }
		NameHash(const char* _str, uint32 _hash = 0) : hash(String::Hashi(_str, _hash)) { }

		NameHash& operator += (const char* _str) { hash = String::Hashi(_str, hash); return *this; }
		NameHash& operator += (const String& _str) { hash = String::Hashi(_str, hash); return *this; }
		NameHash operator + (const char* _rhs) const { return NameHash(_rhs, hash); }
		NameHash operator + (const String& _rhs) const { return NameHash(_rhs, hash); }

		operator uint32 (void) const { return hash; }

		bool operator == (const NameHash& _rhs) const { return hash == _rhs.hash; }
		bool operator != (const NameHash& _rhs) const { return hash != _rhs.hash; }
		bool operator < (const NameHash& _rhs) const { return hash < _rhs.hash; }
		bool operator <= (const NameHash& _rhs) const { return hash <= _rhs.hash; }

		uint32 hash;
	};

	//----------------------------------------------------------------------------//
	// Name
	//----------------------------------------------------------------------------//

	class Name
	{
	public:
		struct Item
		{
			Item(const String& _name) : name(_name), hash(_name) { }
			NameHash hash;
			String name;
		};

		Name(void) : m_item(&s_empty) { }
		Name(const String& _str) : m_item(_AddItem(_str)) { }
		Name& operator = (const String& _str) { m_item = _AddItem(_str); return *this; }
		bool operator == (const Name& _rhs) const { return m_item == _rhs.m_item || m_item->hash == _rhs.m_item->hash; }
		bool operator != (const Name& _rhs) const { return !(*this == _rhs); }
		bool operator == (const NameHash& _rhs) const { return m_item->hash == _rhs; }
		bool operator != (const NameHash& _rhs) const { return !(*this == _rhs); }
		const String& Str(void) const { return m_item->name; }
		const char* CStr(void) const { return m_item->name; }
		const char* operator * (void) const { return m_item->name; }
		const NameHash& Hash(void) const { return m_item->hash; }
		operator const String& (void) const { return m_item->name; }
		operator const NameHash& (void) const { return m_item->hash; }
		operator uint (void) const { return m_item->hash; }

	protected:
		static const Item* _AddItem(const String& _name);

		const Item* m_item;

		static const Item s_empty;
		static HashMap<uint, const Item*> s_names; // <StrHash, Item>
		static volatile int s_lock;
	};

	//----------------------------------------------------------------------------//
	// SArray
	//----------------------------------------------------------------------------//

	template <class T, class SizeType = uint16> class SArray
	{
	public:

		static const SizeType MaxSize = (SizeType)(-1);
		static_assert(MaxSize > 0, "signed type for size");

		SArray(void) : m_size(0), m_used(0), m_data(nullptr)
		{
		}
		SArray(const SArray& _other) : m_size(0), m_used(0), m_data(nullptr)
		{
			PushBack(_other.m_data, _other.m_size, false);
		}
		SArray(SArray&& _temp) : m_size(_temp.m_size), m_used(_temp.m_used), m_data(_temp.m_data)
		{
			_temp.m_used = 0, _temp.m_data = nullptr;
		}
		~SArray(void)
		{
			Clear();
			delete[] reinterpret_cast<uint8_t*>(m_data);
		}
		SArray& operator = (const SArray& _other)
		{
			if (m_data != _other)
			{
				Clear();
				PushBack(_other.m_data, _other.m_size, false);
			}
			return *this;
		}
		SArray& operator = (SArray&& _temp)
		{
			m_size = _temp.m_size;
			Swap(m_used, _temp.m_used);
			Swap(m_data, _temp.m_data);
			return *this;
		}
		T& operator [] (uint _index) { return At(_index); }
		const T& operator [] (uint _index) const { return At(_index); }
		bool operator == (const SArray& _rhs) const
		{
			if (m_data != _rhs.m_data && m_used == _rhs.m_used)
			{
				for (T *a = m_data, *b = _rhs.m_data, *e = m_data + m_used; a < e;)
				{
					if (*a != *b)
						return false;
				}
			}
			return m_used == _rhs.m_used;
		}
		bool operator != (const SArray& _rhs) const { return !(*this == _rhs); }
		void PushBack(const T* _e, uint _count, bool _quantize = true)
		{
			size_t _newSize = m_used + _count;
			if (_newSize > m_size)
			{
				assert(_newSize < MaxSize);
				if (_quantize)
				{
					_newSize <<= 1; // quantize	memory
					if (_newSize > MaxSize)
						_newSize = MaxSize;
				}
				_Realloc(_newSize);
			}
			T* _p = m_data + m_used;
			m_used += _count;
			T* _end = m_data + m_used;
			while (_p < _end)
				new(_p++) T(*_e++);
		}
		void PushBack(const T& _e)
		{
			PushBack(&_e, 1);
		}
		void PopBack(void)
		{
			if (m_used > 0)
				m_data[m_used--].~T();
		}
		void Reserve(uint _newSize)
		{
			if (_newSize > m_size)
			{
				assert(_newSize < MaxSize);
				_Realloc(_newSize);
			}
		}
		void Resize(uint _newSize)
		{
			if (_newSize < m_used)
			{
				while (m_used > _newSize)
					m_data[m_used--].~T();
			}
			else if (_newSize > m_used)
			{
				size_t _count = _newSize - m_used;
				if (_newSize > m_size)
				{
					assert(_newSize < MaxSize);
					_Realloc(_newSize);
				}
				T* _p = m_data + m_used;
				m_used += static_cast<SizeType>(_count);
				while (_count--)
					new(_p++) T();
			}
		}
		void Resize(uint _newSize, const T& _sample)
		{
			if (_newSize < m_used)
			{
				while (m_used > _newSize)
					m_data[m_used--].~T();
			}
			else if (_newSize > m_used)
			{
				size_t _count = _newSize - m_used;
				if (_newSize > m_size)
				{
					assert(_newSize < MaxSize);
					_Realloc(_newSize);
				}
				T* _p = m_data + m_used;
				m_used += static_cast<SizeType>(_count);
				while (_count--)
					new(_p++) T(_sample);
			}
		}
		void Clear(void)
		{
			while (m_used > 0)
				m_data[m_used--].~T();
		}
		void Remove(const T& _e)
		{
			T *_r, *_b = m_data + m_used;
			for (SizeType i = 0; i < m_used; ++i)
			{
				_r = m_data + i;
				if (*_r == _e)
				{
					if (_r != _b)
						*_r = Move(*_b);
					_b->~T();
					--m_used;
					break;
				}
			}
		}
		uint Size(void) const { return m_used; }
		uint ReservedSize(void) const { return m_size; }
		bool IsEmpty(void) const { return m_used == 0; }
		bool NonEmpty(void) const { return m_used != 0; }
		T* Ptr(void) { return m_data; }
		const T* Ptr(void) const { return m_data; }
		T& Back(void) { ASSERT(m_used > 0); return *(m_data + m_used); }
		const T& Back(void) const { assert(m_used > 0); return *(m_data + m_used); }
		T& At(uint _index) { ASSERT(_index < m_used); return m_data[_index]; }
		const T& At(uint _index) const { ASSERT(_index < m_used); return m_data[_index]; }

	protected:

		void _Realloc(size_t _newSize)
		{
			T* _newData = reinterpret_cast<T*>(new uint8_t[_newSize * sizeof(T)]);
			for (SizeType i = 0; i < m_used; ++i)
			{
				new(_newData + i) T(Move(m_data[i]));
				m_data[i].~T();
			}
			delete[] reinterpret_cast<uint8_t*>(m_data);
			m_data = _newData;
			m_size = static_cast<SizeType>(_newSize);
		}

		SizeType m_size;
		SizeType m_used;
		T* m_data;
	};

	//----------------------------------------------------------------------------//
	// Checksum
	//----------------------------------------------------------------------------//

	uint32 Crc32(uint32 _crc, const void* _buf, uint _size);
	inline uint32 Crc32(const void* _buf, uint _size) { return Crc32(0, _buf, _size); }
	inline uint32 Crc32(const char* _str, int _length = -1, uint32 _crc = 0) { return _str ? Crc32(_crc, _str, _length < 0 ? (uint)strlen(_str) : _length) : _crc; }
	inline uint32 Crc32(const String& _str, uint32 _crc = 0) { return Crc32(_crc, _str, _str.Length()); }
	template <typename T> uint32 Crc32(const T& _obj, uint32 _crc = 0) { return Crc32(_crc, &_obj, sizeof(_obj)); }

	//----------------------------------------------------------------------------//
	// CharStream
	//----------------------------------------------------------------------------//

	/// Char stream for any parsers
	struct CharStream
	{
		CharStream(const CharStream& _other) : s(_other.s), l(_other.l), c(_other.c) { }
		CharStream(const char* _str) : s(_str), l(1), c(1) { }
		operator const char* (void) { return s; }
		char operator [] (int i) { return s[i]; }
		const char* operator ++ (void) { Next(); return s; }
		const char* operator ++ (int) { const char* p = s; Next(); return p; }
		const char* operator += (int _c) { assert(_c >= 0); Next(_c); return s; }

		void Next(uint _c = 1)
		{
			while (*s && _c--)
			{
				if (strchr("\n\r", *s))
				{
					if (s[0] == '\r' && s[1] == '\n')
						++s;
					c = 0;
					++l;
				}
				c += (*s == '\t') ? (4 - c % 4) : 1; // tab
				++s;
			}
		}
		uint Skip(const char* _cset, bool _op = true)
		{
			assert(_cset != nullptr);
			const char* _start = s;
			while (*s && (strchr(_cset, *s) != 0) == _op)
				Next();
			return (uint)(s - _start);
		}
		bool AnyOf(const char* _cset, bool _op = true)
		{
			assert(_cset != nullptr);
			return *s && (strchr(_cset, *s) != 0) == _op;
		}

		const char* s; // stream
		uint l, c; // line, column
	};

	//----------------------------------------------------------------------------//
	// Function
	//----------------------------------------------------------------------------//

	template <class F> void* FuncPtr(F _func) { union { F f; void* p; }_fp = { _func }; return _fp.p; }
	template <class F> F FuncCast(void* _func) { union { void* p; F f; }_fp = { _func }; return _fp.f; }

	// example:
	// auto func = Function<void(int)>(Func); // c-func
	// func = Function<void(int)>(&var, &MyClass::Func); // method
	// func(0);	// call

	template <class F> struct Function;
	template <class R, class... A> struct Function<R(A...)>
	{
		// TODO: calling convention
		typedef R(*Invoke)(void*, void*, A&&...);

		typedef R(*Ptr)(A...);
		typedef R(Type)(A...);

		Invoke invoke;
		void* func;
		void* self;

		Function(void) : invoke(nullptr), func(nullptr), self(nullptr) { }
		Function(R(*_func)(A...)) : invoke(InvokeFunc), func(FuncPtr(_func)), self(nullptr) { }
		template <class C> Function(C* _self, R(C::*_func)(A...)) : invoke(InvokeMethod<C>), func(FuncPtr(_func)), self(_self) { ASSERT(_self != nullptr); }
		template <class C> Function(const C* _self, R(C::*_func)(A...) const) : invoke(InvokeConstMethod<C>), func(FuncPtr(_func)), self(const_cast<C*>(_self)) { ASSERT(_self != nullptr); }
		operator bool(void) const { return func != nullptr; }

		R operator () (A... _args) const
		{
			ASSERT(func != nullptr);
			return invoke(self, func, Forward<A>(_args)...);
		}

		static R InvokeFunc(void* _self, void* _func, A&&... _args)
		{
			typedef R(*Func)(A...);
			return FuncCast<Func>(_func)(Forward<A>(_args)...);
		}

		template <class C> static R InvokeMethod(void* _self, void* _func, A&&... _args)
		{
			ASSERT(_self != nullptr);
			typedef R(C::*Func)(A...);
			return (*((C*)_self).*FuncCast<Func>(_func))(Move(_args)...);
		}

		template <class C> static R InvokeConstMethod(void* _self, void* _func, A&&... _args)
		{
			ASSERT(_self != nullptr);
			typedef R(C::*Func)(A...) const;
			return (*((const C*)_self).*FuncCast<Func>(_func))(Move(_args)...);
		}
	};

	template <class R, class... A> Function<R(A...)> MakeFunction(R(*_func)(A...))
	{
		return Function<R(A...)>(_func);
	}
	template <class C, class R, class... A> Function<R(A...)> MakeFunction(C* _self, R(C::*_func)(A...))
	{
		return Function<R(A...)>(_self, _func);
	}
	template <class C, class R, class... A> Function<R(A...)> MakeFunction(const C* _self, R(C::*_func)(A...) const)
	{
		return Function<R(A...)>(_self, _func);
	}

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
	// RCBase
	//----------------------------------------------------------------------------//

#define CLASSNAME(T) \
	static uint32 ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; } \
	static const String& ClassNameStatic(void) { static const String _clsname(#T); return _clsname; } \
	uint32 ClassId(void) override { return ClassIdStatic(); } \
	const String& ClassName(void) override  { return ClassNameStatic(); } \
	bool IsClass(const NameHash& _id) override { return _id.hash == ClassIdStatic() || __super::IsClass(_id); }

#define SAFE_ADDREF(p) if (p) { p->AddRef(); }
#define SAFE_RELEASE(p) if (p) { p->Release(); }
#define SAFE_ASSIGN(lhs, rhs) SAFE_ADDREF(rhs) SAFE_RELEASE(lhs) lhs = rhs

	class RCBase : public NonCopyable
	{
	public:
		static uint32 ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; }
		static const String& ClassNameStatic(void) { static const String _clsname("RCBase"); return _clsname; }
		virtual uint32 ClassId(void) { return ClassIdStatic(); }
		virtual const String& ClassName(void) { return ClassNameStatic(); }
		virtual bool IsClass(const NameHash& _id) { return _id.hash == ClassIdStatic(); }

		RCBase(void) : m_refCount(0) { }
		virtual ~RCBase(void) { }
		int AddRef(void) { return AtomicAdd(m_refCount, 1) + 1; }
		int Release(void)
		{
			int _r = AtomicAdd(m_refCount, -1);
			if (_r == 1)
				_ReleaseImpl();
			return _r - 1;
		}
		int GetRefCount(void) { return m_refCount; }

	protected:
		volatile int m_refCount;

		virtual void _ReleaseImpl(void) { delete this; }
	};

	//----------------------------------------------------------------------------//
	// RCObject
	//----------------------------------------------------------------------------//

	class RCObject : public RCBase
	{
	public:
		CLASSNAME(RCObject);

		/// The weak reference.
		///\warning weak reference is non threadsafe.
		struct WeakRef : public NonCopyable
		{
			void AddRef(void) { AtomicAdd(m_refCount, 1); }
			void Release(void) { if (AtomicAdd(m_refCount, -1) == 1) delete this; }
			RCObject* GetPtr(void) const { return m_object; }

		private:
			friend class RCObject;
			WeakRef(RCObject* _object) : m_refCount(1), m_object(_object) { }
			void _Reset(void) { m_object = nullptr; Release(); }

			volatile int m_refCount;
			RCObject* m_object;
		};

		RCObject(void) : m_weakRef(nullptr) { }
		virtual ~RCObject(void)
		{
			if (m_weakRef)
				m_weakRef->_Reset();
		}
		WeakRef* GetRef(void) const
		{
			if (!m_weakRef)
				m_weakRef = new WeakRef(const_cast<RCObject*>(this));
			return m_weakRef;
		}

	private:
		mutable WeakRef* m_weakRef;
	};

	//----------------------------------------------------------------------------//
	// Ptr
	//----------------------------------------------------------------------------//

	template <class T> class Ptr
	{
		T* p;

	public:
		Ptr(void) : p(nullptr) { }
		Ptr(const Ptr& _p) : p(const_cast<T*>(_p.p)) { SAFE_ADDREF(p); }
		Ptr(const T* _p) : p(const_cast<T*>(_p)) { SAFE_ADDREF(p); }
		~Ptr(void) { SAFE_RELEASE(p); }
		Ptr& operator = (const Ptr& _p) { SAFE_ASSIGN(p, const_cast<T*>(_p.p)); return *this; }
		Ptr& operator = (const T* _p) { SAFE_ASSIGN(p, const_cast<T*>(_p)); return *this; }
		T& operator * (void) const { ASSERT(p != nullptr, "Null pointer"); return *const_cast<T*>(p); }
		T* operator -> (void) const { ASSERT(p != nullptr, "Null pointer"); return const_cast<T*>(p); }
		operator T* (void) const { return const_cast<T*>(p); }
		T* Get(void) const { return const_cast<T*>(p); }
		template <class X> X* Cast(void) const { return static_cast<X*>(const_cast<T*>(p)); }
		template <class X> X* DynamicCast(void) const { return dynamic_cast<X*>(const_cast<T*>(p)); }
	};

	//----------------------------------------------------------------------------//
	// Ref
	//----------------------------------------------------------------------------//

	template <class T> class Ref
	{
		Ptr<RCObject::WeakRef> p;
		typedef Ptr<T> Ptr;
	public:

		Ref(void) : p(nullptr) { }
		Ref(const Ref& _other) : p(_other.p) { }
		Ref(const Ptr& _object) : p(GetRef(_object)) { }
		Ref(const T* _object) : p(GetRef(_object)) { }
		Ref& operator = (const Ref& _rhs) { p = _rhs.p; return *this; }
		Ref& operator = (const Ptr& _rhs) { p = GetRef(_rhs); return *this; }
		Ref& operator = (const T* _rhs) { p = GetRef(_rhs); return *this; }
		T& operator* (void) const { ASSERT(GetPtr(p) != nullptr, "Null pointer"); return *GetPtr(p); }
		T* operator-> (void) const { ASSERT(GetPtr(p) != nullptr, "Null pointer"); return GetPtr(p); }
		operator T* (void) const { return GetPtr(p); }
		T* Get(void) const { return GetPtr(p); }
		RCObject::WeakRef* GetRef(void) const { return p; }
		template <class X> X* Cast(void) const { return static_cast<X*>(GetPtr(p)); }
		template <class X> X* DyncamicCast(void) const { return dynamic_cast<X*>(GetPtr(p)); }
		static RCObject::WeakRef* GetRef(const T* _object) { return _object ? _object->GetRef() : nullptr; }
		static T* GetPtr(const RCObject::WeakRef* _weakRef) { return _weakRef ? static_cast<T*>(_weakRef->GetPtr()) : nullptr; }
	};

	//----------------------------------------------------------------------------//
	// Singleton
	//----------------------------------------------------------------------------//

	template <class T> class Singleton : public NonCopyable
	{
	public:
		Singleton(void)
		{
			ASSERT(s_instance == nullptr, "Instance of this class already exists");
			s_instance = static_cast<T*>(this);
		}
		~Singleton(void)
		{
			s_instance = nullptr;
		}

		/// Get instance.
		static T* Get(void) { return s_instance; }
		/// Get instance.
		template <class X> static X* Get(void) { return static_cast<X*>(s_instance); }

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = 0;

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

namespace std
{
	template<> struct hash<Engine::String>
	{
		size_t operator()(const Engine::String& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};

	template<class T> struct hash<Engine::Ptr<T>> : public std::hash<T*>
	{
	};

	template<class T> struct hash<Engine::Ref<T>> : public std::hash<T*>
	{
		size_t operator()(const Engine::Ref<T>& _Keyval) const
		{
			return std::hash<T*>::operator()(_Keyval.GetRef());
		}
	};

	template<> struct hash<Engine::Name> : public std::hash<Engine::uint>
	{
		size_t operator()(const Engine::Name& _Keyval) const
		{
			return std::hash<Engine::uint>::operator()(_Keyval);
		}
	};
}
