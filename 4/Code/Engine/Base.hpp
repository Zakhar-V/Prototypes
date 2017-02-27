/*!
* \file Base.hpp
* \author Zakhar Volkov
* \version 0.3.1a
* \date 2015-2016

Contain the 
-strings (String, WString, CharStream)
-template functions
-template containers
-utilities
-atomic operations (Atomic, AtomicFlag)
-patterns (NonCopyable, Singleton)
-RC	(RefCounted, SharedPtr, WeakPtr)

**/
#pragma once

#include "Common.hpp"

#define HAS_STL 1 // temp flag 

#if HAS_STL
#include <algorithm>
#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#endif

namespace Rx
{
	//----------------------------------------------------------------------------//
	// Definitions
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

	enum AccessMode : uint8
	{
		AM_None = 0,
		AM_Read = 0x1,
		AM_Write = 0x2,
		AM_ReadWrite = AM_Read | AM_Write,
	};

	template <class T> struct Atomic;
	typedef Atomic<int> AtomicInt;
	typedef Atomic<bool> AtomicBool;

	/*template <class T> class InitializerList;
	template <class A, class B> class Pair;
	template <class T> class Array;
	template <class T> class List;
	template <class T, class U> class HashMap;
	template <class T> class HashSet;
	template <class T, class U> class TreeMap;
	template <class T> class TreeSet;

	class String;
	typedef Array<String> StringArray;
	typedef Pair<String, String> StringPair;*/

	//----------------------------------------------------------------------------//
	// Move semantics
	//----------------------------------------------------------------------------//

	template <class T> struct RRef { typedef T Type; };
	template <class T> struct RRef<T&> { typedef T Type; };
	template <class T> struct RRef<T&&> { typedef T Type; };

	//! Get a rvalue reference.
	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type& _ref) { return static_cast<T&&>(_ref); }
	//! Get a rvalue reference.
	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type&& _ref) { return static_cast<T&&>(_ref); }
	//! Get a rvalue reference.
	template <class T> inline typename RRef<T>::Type&& Move(T&& _ref) { return static_cast<RRef<T>::Type&&>(_ref); }

	//! Swap elements.
	template <class T> void Swap(T& _a, T& _b)
	{
#if HAS_STL
		std::swap(_a, _b);
#else
		T _c = Move(_a);
		_a = Move(_b);
		_b = Move(_c);
#endif
	}

	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	enum MemoryOrder
	{
		MO_Relaxed,
		MO_Consume,
		MO_Acquire,
		MO_Release,
		MO_AcquireRelease,
		MO_Default, // sequential consistency
	};

	RX_API int8 AtomicGet(int8& _atom);
	RX_API void AtomicSet(int8& _atom, int8 _value);
	RX_API int8 AtomicAdd(int8& _atom, int8 _value); //!<\return previous value
	RX_API int8 AtomicExchange(int8& _atom, int8 _value);
	RX_API bool AtomicCompareExchange(int8& _atom, int8& _exp, int8 _value);

	RX_API int16 AtomicGet(int16& _atom);
	RX_API void AtomicSet(int16& _atom, int16 _value);
	RX_API int16 AtomicAdd(int16& _atom, int16 _value); //!<\return previous value
	RX_API int16 AtomicExchange(int16& _atom, int16 _value);
	RX_API bool AtomicCompareExchange(int16& _atom, int16& _exp, int16 _value);

	RX_API int32 AtomicGet(int32& _atom);
	RX_API void AtomicSet(int32& _atom, int32 _value);
	RX_API int32 AtomicAdd(int32& _atom, int32 _value);	//!<\return previous value
	RX_API int32 AtomicExchange(int32& _atom, int32 _value);
	RX_API bool AtomicCompareExchange(int32& _atom, int32& _exp, int32 _value);

	RX_API int64 AtomicGet(int64& _atom);
	RX_API void AtomicSet(int64& _atom, int64 _value);
	RX_API int64 AtomicAdd(int64& _atom, int64 _value);	//!<\return previous value
	RX_API int64 AtomicExchange(int64& _atom, int64 _value);
	RX_API bool AtomicCompareExchange(int64& _atom, int64& _exp, int64 _value);

	template <int S> struct AtomicType;
	template <> struct AtomicType<1> { typedef int8 Type; };
	template <> struct AtomicType<2> { typedef int16 Type; };
	template <> struct AtomicType<4> { typedef int32 Type; };
	template <> struct AtomicType<8> { typedef int64 Type; };

	template <class T> struct Atomic
	{
	public:
		typedef typename AtomicType<sizeof(T)>::Type BaseType;

		Atomic(T _value = static_cast<T>(0)) : m_value(static_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (T _value) { AtomicSet(m_value, static_cast<BaseType>(_value)); return *this; }
		operator T (void) const { return static_cast<T>(AtomicGet(m_value)); }

		T operator += (T _value) { return static_cast<T>(AtomicAdd(m_value, static_cast<BaseType>(_value)) + static_cast<BaseType>(_value)); }
		T operator -= (T _value) { return static_cast<T>(AtomicAdd(m_value, static_cast<BaseType>(-_value)) - static_cast<BaseType>(_value)); }

		T operator ++ (void) { return static_cast<T>(AtomicAdd(m_value, 1) + 1); }
		T operator ++ (int) { return static_cast<T>(AtomicAdd(m_value, 1)); }
		T operator -- (void) { return static_cast<T>(AtomicAdd(m_value, -1) - 1); }
		T operator -- (int) { return static_cast<T>(AtomicAdd(m_value, -1)); }

		T Exchange(T _value) { return static_cast<T>(AtomicExchange(m_value, static_cast<BaseType>(_value))); }
		bool CompareExchange(T* _exp, T _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(_exp), static_cast<BaseType>(_value)); }
		bool CompareExchange(T _exp, T _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), static_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	template <> struct Atomic<bool>
	{
	public:
		typedef AtomicType<sizeof(bool)>::Type BaseType;

		Atomic(bool _value = false) : m_value(static_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (bool _value) { AtomicSet(m_value, static_cast<BaseType>(_value)); return *this; }
		operator bool(void) const { return AtomicGet(m_value) != 0; }

		//TODO: bitwise operators

		bool Exchange(bool _value) { return AtomicExchange(m_value, static_cast<BaseType>(_value)) != 0; }
		bool CompareExchange(bool* _exp, bool _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(_exp), static_cast<BaseType>(_value)); }
		bool CompareExchange(bool _exp, bool _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), static_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	template <class T> struct Atomic<T*>
	{
	public:
		typedef typename AtomicType<sizeof(T*)>::Type BaseType;

		Atomic(T* _value = nullptr) : m_value(reinterpret_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (T* _value) { AtomicSet(m_value, reinterpret_cast<BaseType>(_value)); return *this; }
		operator T* (void) const { return reinterpret_cast<T*>(AtomicGet(m_value)); }

		T* operator += (size_t _value) { return reinterpret_cast<T*>(AtomicAdd(m_value, static_cast<BaseType>(_value * sizeof(T)))); }
		T* operator -= (size_t _value) { return reinterpret_cast<T*>(AtomicAdd(m_value, static_cast<BaseType>(-_value * sizeof(T)))); }

		T* operator ++ (void) { return reinterpret_cast<T*>(AtomicAdd(m_value, sizeof(T))); }
		T* operator ++ (int) { return reinterpret_cast<T*>(AtomicAdd(m_value, sizeof(T)) - sizeof(T)); }
		T* operator -- (void) { return reinterpret_cast<T*>(AtomicAdd(m_value, -sizeof(T))); }
		T* operator -- (int) { return reinterpret_cast<T*>(AtomicAdd(m_value, -sizeof(T)) + sizeof(T)); }

		T* Exchange(T* _value) { return reinterpret_cast<T*>(AtomicExchange(m_value, reinterpret_cast<BaseType>(_value))); }
		bool CompareExchange(T** _exp, T* _value) { return AtomicCompareExchange(m_value, **reinterpret_cast<BaseType*>(&_exp), reinterpret_cast<BaseType>(_value)); }
		bool CompareExchange(T* _exp, T* _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), reinterpret_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	//----------------------------------------------------------------------------//
	// InitializerList 
	//----------------------------------------------------------------------------//

	template <class T> using InitializerList = std::initializer_list<T>;

	//----------------------------------------------------------------------------//
	// Pair 
	//----------------------------------------------------------------------------//

	template <class A, class B> using Pair = std::pair<A, B>;

	//----------------------------------------------------------------------------//
	// Array 
	//----------------------------------------------------------------------------//

	template <class T> using Array = std::vector<T>;

	//----------------------------------------------------------------------------//
	// List 
	//----------------------------------------------------------------------------//

	template <class T> using List = std::list<T>;

	//----------------------------------------------------------------------------//
	// HashMap 
	//----------------------------------------------------------------------------//

	template <class T, class U> using HashMap = std::unordered_map<T, U>;

	//----------------------------------------------------------------------------//
	// HashMultiMap 
	//----------------------------------------------------------------------------//

	//template <class K, class V> using HashMultiMap = std::unordered_multimap<K, V>;

	//----------------------------------------------------------------------------//
	// HashSet
	//----------------------------------------------------------------------------//

	template <class T> using HashSet = std::unordered_set<T>;

	//----------------------------------------------------------------------------//
	// TreeMap 
	//----------------------------------------------------------------------------//

	template <class T, class U> using TreeMap = std::map<T, U>;
	
	//----------------------------------------------------------------------------//
	// TreeSet
	//----------------------------------------------------------------------------//

	template <class T> using TreeSet = std::set<T>;

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	class String;
	typedef Array<String> StringArray;
	typedef Pair<String, String> StringPair;

	class RX_API String
	{
	public:
		String(void) : m_buffer(_Null()) { }
		~String(void) { _Release(m_buffer); }
		String(const String& _other) : m_buffer(_AddRef(_other.m_buffer)) { }
		String(String&& _temp) : m_buffer(_temp.m_buffer) { _temp.m_buffer = _Null(); }
		String(int _ch) : m_buffer(_ch ? _New((const char*)&_ch, 1) : _Null()) { }
		String(char _ch) : m_buffer(_ch ? _New(&_ch, 1) : _Null()) { }
		String(const char* _str, int _length = -1) : m_buffer(_New(_str, _Length(_str, _length))) { }
		String(const char* _first, const char* _last) : m_buffer(_first == _last ? _Null() : _New(_first, (uint)(_last - _first))) { }
		String(uint _count, char _ch) : m_buffer(_New(nullptr, _count)) { while (_count--) m_buffer->str[_count] = _ch; }

		String& operator = (const String& _str) { _AddRef(_str.m_buffer); _Release(m_buffer); m_buffer = _str.m_buffer; return *this; }
		String& operator = (String&& _temp) { Swap(m_buffer, _temp.m_buffer); return *this; }
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
		const char* Ptr(uint _offset) const { ASSERT(_offset <= m_buffer->length); return m_buffer->str + _offset; }
		uint32 Hash(uint32 _hash = 0) const { return Hash(m_buffer->str, _hash); }
		DEPRECATED uint32 Hashi(uint32 _hash = 0) const { return Hashi(m_buffer->str, _hash); }
		DEPRECATED uint32 IHash(uint32 _hash = 0) const { return IHash(m_buffer->str, _hash); }
		String Trim(const char* _cset, bool _left = true, bool _right = true) const;
		StringArray Split(const char* _delimiters) const { StringArray _dst; Split(m_buffer->str, _delimiters, _dst); return Move(_dst); }
		int Compare(const char* _rhs, bool _ignoreCase = false) const { return Compare(*this, _rhs, _ignoreCase); }
		bool Equals(const char* _rhs, bool _ignoreCase = false) const { return Compare(*this, _rhs, _ignoreCase) == 0; }
		bool Match(const char* _pattern, bool _ignoreCase = true) const { return Match(*this, _pattern, _ignoreCase); }
		String ToLower(void) const { return ToLower(*this); }
		String ToUpper(void) const { return ToUpper(*this); }

		static String Format(const char* _fmt, ...);
		static String FormatV(const char* _fmt, va_list _args);
		static uint32 Hash(const char* _str, uint32 _hash = 0);
		DEPRECATED static uint32 Hashi(const char* _str, uint32 _hash = 0);
		static uint32 IHash(const char* _str, uint32 _hash = 0);
		static int Compare(const char* _a, const char* _b, bool _ignoreCase = false);
		static bool Equals(const char* _a, const char* _b, bool _ignoreCase = false) { return Compare(_a, _b, _ignoreCase) == 0; }
		static bool Match(const char* _str, const char* _pattern, bool _ignoreCase = true);
		static const char* Find(const char* _str1, const char* _str2, bool _ignoreCase = false);
		static String Trim(const char* _str, const char* _cset, bool _left = true, bool _right = true);
		static void Split(const char* _str, const char* _delimiters, StringArray& _dst);

		static char ToLower(char _ch) { return ((_ch >= 'A' && _ch <= 'Z') || (_ch >= 'a' && _ch <= 'z') || ((uint8)_ch >= 0xc0)) ? (_ch | 0x20) : _ch; }
		static char ToUpper(char _ch) { return ((_ch >= 'A' && _ch <= 'Z') || (_ch >= 'a' && _ch <= 'z') || ((uint8)_ch >= 0xc0)) ? (_ch & ~0x20) : _ch; }
		static String ToLower(const char* _str);
		static String ToUpper(const char* _str);
		static uint16 DecodeUtf8(const char*& _utf8);
		//static WString DecodeUtf8(const String& _utf8);
		static void EncodeUtf8(String& _outUtf8, const wchar_t _inUcs2);
		static String EncodeUtf8(const wchar_t* _ucs2);

		static uint GetTotalMemoryUse(void) { return s_memory; }

		static const String Empty;

	protected:

		struct Buffer
		{
			Buffer(uint _size, uint _length) : refs(1), size(_size), length(_length) { str[_length] = 0; }
			int refs;
			uint size;
			uint length;
			char str[1];
		};

		static Buffer* _Null(void);
		static Buffer* _New(const char* _str, uint _length, uint _size = 0);
		static Buffer* _Unique(Buffer*& _buffer, uint _newLength, bool _quantizeMemory);
		static Buffer* _AddRef(Buffer* _buffer);
		static void _Release(Buffer* _buffer, int _refs = -1);
		static uint _Length(const char* _str, int _length);

		Buffer* m_buffer;
		static int s_memory;
	};

	typedef Array<String> StringArray;
	typedef Pair<String, String> StringPair;

	//----------------------------------------------------------------------------//
	// NameHash 
	//----------------------------------------------------------------------------//

	struct NameHash
	{
		NameHash(void) = default;
		NameHash(uint _value) : hash(_value) { }
		NameHash(const String& _str, uint _hash = 0) : hash(String::IHash(_str, _hash)) { }
		NameHash(const char* _str, uint _hash = 0) : hash(String::IHash(_str, _hash)) { }

		NameHash& operator += (const char* _str) { hash = String::IHash(_str, hash); return *this; }
		NameHash& operator += (const String& _str) { hash = String::IHash(_str, hash); return *this; }
		NameHash operator + (const char* _rhs) const { return NameHash(_rhs, hash); }
		NameHash operator + (const String& _rhs) const { return NameHash(_rhs, hash); }

		operator uint (void) const { return hash; }

		/*bool operator == (const NameHash& _rhs) const { return hash == _rhs.hash; }
		bool operator != (const NameHash& _rhs) const { return hash != _rhs.hash; }
		bool operator < (const NameHash& _rhs) const { return hash < _rhs.hash; }
		bool operator <= (const NameHash& _rhs) const { return hash <= _rhs.hash; }
	   */
		uint hash = 0;
	};

	//----------------------------------------------------------------------------//
	// Name
	//----------------------------------------------------------------------------//

	DEPRECATED class RX_API Name
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
		const char* CStr(void) const { return m_item->name.CStr(); }
		const NameHash& Hash(void) const { return m_item->hash; }
		operator const String& (void) const { return m_item->name; }
		operator const NameHash& (void) const { return m_item->hash; }
		operator uint (void) const { return m_item->hash; }

	protected:
		static const Item* _AddItem(const String& _name);

		const Item* m_item;

		static const Item s_empty;
		static HashMap<uint, const Item*> s_names; // <StrHash, Item>
		static int s_mutex;
	};

	//----------------------------------------------------------------------------//
	// CharStream
	//----------------------------------------------------------------------------//

	/// Char stream for any parsers
	struct RX_API CharStream
	{
		CharStream(const CharStream& _other) : s(_other.s), l(_other.l), c(_other.c) { }
		CharStream(const char* _str) : s(_str), l(1), c(1) { }
		operator const char* (void) { return s; }
		char operator [] (int i) { return s[i]; }
		const char* operator ++ (void) { Next(); return s; }
		const char* operator ++ (int) { const char* p = s; Next(); return p; }
		const char* operator += (int _c) { ASSERT(_c >= 0); Next(_c); return s; }

		void Next(uint _c = 1);
		uint Skip(const char* _cset, bool _op = true);
		bool AnyOf(const char* _cset, bool _op = true);

		const char* s; // stream
		uint l, c; // line, column
	};

	//----------------------------------------------------------------------------//
	// Checksum
	//----------------------------------------------------------------------------//

	extern RX_API const uint32 Crc32Table[256];

	inline uint32 Crc32(uint32 _crc, const void* _buf, uint _size)
	{
		ASSERT(_buf || !_size);
		const uint8* p = (const uint8*)_buf;
		_crc = _crc ^ ~0u;
		while (_size--)
			_crc = Crc32Table[(_crc ^ *p++) & 0xff] ^ (_crc >> 8);
		return _crc ^ ~0u;
	}
	inline uint32 Crc32(const void* _buf, uint _size) { return Crc32(0, _buf, _size); }
	inline uint32 Crc32(const char* _str, int _length = -1, uint32 _crc = 0) { return _str ? Crc32(_crc, _str, _length < 0 ? (uint)strlen(_str) : _length) : _crc; }
	inline uint32 Crc32(const String& _str, uint32 _crc = 0) { return Crc32(_crc, _str, _str.Length()); }
	template <typename T> uint32 Crc32(const T& _obj, uint32 _crc = 0) { return Crc32(_crc, &_obj, sizeof(_obj)); }

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

 namespace std
 {
	 template<> struct hash<Rx::String>
	 {
		 size_t operator()(const Rx::String& _Keyval) const
		 {
			 return _Keyval.Hash();
		 }
	 };

	 template<> struct hash<Rx::Name> : public std::hash<Rx::uint>
	 {
		 size_t operator()(const Rx::Name& _Keyval) const
		 {
			 return std::hash<Rx::uint>::operator()(_Keyval);
		 }
	 };
 }

 namespace Rx
 {
	 //----------------------------------------------------------------------------//
	 // NonCopyable
	 //----------------------------------------------------------------------------//

	 //!
	 class RX_API NonCopyable
	 {
	 public:
		 //!
		 NonCopyable(void) = default;
		 //!
		 ~NonCopyable(void) = default;

	 private:
		 //!
		 NonCopyable(const NonCopyable&) = delete;
		 //!
		 NonCopyable& operator = (const NonCopyable&) = delete;
	 };

	 //----------------------------------------------------------------------------//
	 // Singleton
	 //----------------------------------------------------------------------------//

	 //!
	 template <class T> class Singleton : public NonCopyable
	 {
	 public:

		 //!
		 Singleton(void) { ASSERT(s_instance == nullptr); s_instance = static_cast<T*>(this); }
		 //!
		 ~Singleton(void) { s_instance = nullptr; }

		 //!
		 static T* Get(void) { return s_instance; }
		 //!
		 template<class X> static X* Get(void) { return static_cast<X*>(s_instance); }

	 protected:

		 //!
		 static T* s_instance;
	 };

	 template <class T> T* Singleton<T>::s_instance = nullptr;

	 //----------------------------------------------------------------------------//
	 // RC helper 
	 //----------------------------------------------------------------------------//

#define SAFE_ADDREF(p) if (p) { p->AddRef(); }
#define SAFE_RELEASE(p) if (p) { p->Release(); }
#define SAFE_ASSIGN(lhs, rhs) SAFE_ADDREF(rhs) SAFE_RELEASE(lhs) lhs = rhs

	 //----------------------------------------------------------------------------//
	 // RefCounter 
	 //----------------------------------------------------------------------------//

	 class RX_API RefCounter : public NonCopyable
	 {
	 public:
		 //! Increments the counter of weak references
		 void AddRef(void);
		 //! Decrements the counter of weak references and destroy this when he equals zero.
		 void Release(void);

		 //! Get the number of strong references.
		 int GetNumReferences(void) const { return m_refs; }
		 //! Get the number of weak references.
		 int GetNumWeakReferences(void) const { return m_refs; }
		 //! \return true if no object. 
		 bool Expired(void) const;
		 //! Get the object. \warning Returned object can be destroyed at any moment asynchronously. Use Rx::RefCounter::_Lock instead for access to the object.
		 class RefCounted* _Get(void) const { return const_cast<RefCounted*>(m_object); }
		 //! Lock the object. Increments the counter of references of object. \warning You must release the returned object (if he is not null).
		 class RefCounted* _Lock(void);

		 //! Get the number of all instances.
		 static int GetNumInstances(void);

	 private:
		 friend class RefCounted;

		 //! Construct.
		 RefCounter(RefCounted* _object);
		 //! Destroy.
		 ~RefCounter(void);

		 //! The object.
		 volatile RefCounted* m_object;
		 //! The counter of strong references.
		 mutable int m_refs;
		 //! The counter of weak references.
		 int m_weakRefs;

		 //! The number of all instances.
		 static int s_numInstances;
	 };

	 //----------------------------------------------------------------------------//
	 // RefCounted 
	 //----------------------------------------------------------------------------//

	 class RX_API RefCounted : public NonCopyable
	 {
	 public:
		 //! Contruct.
		 RefCounted(void);
		 //! Destroy.
		 virtual ~RefCounted(void);

		 //! Increments the counter of references
		 void AddRef(void);
		 //! Decrements the counter of references and destroy this when he equals zero.
		 void Release(void);

		 //! Get the counter of references.
		 RefCounter* GetRef(void) const { return m_refCounter; }
		 //! Get the number of strong references.
		 int GetNumReferences(void) const { return m_refCounter->m_refs; }

		 //! Get the number of all instances.
		 static int GetNumInstances(void);

	 protected:
		 //! Delete this. You can override this method for customized algorithm of deletion. 
		 virtual void _DeleteThis(void) { delete this; }

		 //! The counter of references.
		 RefCounter* m_refCounter;

		 //! The number of all instances.
		 static int s_numInstances;
	 };

	 //----------------------------------------------------------------------------//
	 // Ptr 
	 //----------------------------------------------------------------------------//

	 enum _NoAddRef
	 {
		 NoAddRef
	 };

	 //! The shared pointer.
	 template <class T> class SharedPtr
	 {
	 public:
		 //!
		 SharedPtr(void) : m_ptr(nullptr) { }
		 //!
		 SharedPtr(const SharedPtr& _other) : m_ptr(_other.m_ptr) { SAFE_ADDREF(m_ptr); }
		 //!
		 SharedPtr(SharedPtr&& _other) : m_ptr(_other.m_ptr) { _other.m_ptr = nullptr; }
		 //!
		 SharedPtr(const T* _ptr) : m_ptr(const_cast<T*>(_ptr)) { SAFE_ADDREF(m_ptr); }
		 //!
		 SharedPtr(const T* _ptr, _NoAddRef) : m_ptr(const_cast<T*>(_ptr)) { }
		 //!
		 ~SharedPtr(void) { SAFE_RELEASE(m_ptr); }

		 //!
		 SharedPtr& operator = (const SharedPtr& _rhs) { SAFE_ASSIGN(m_ptr, _rhs.m_ptr); return *this; }
		 //!
		 SharedPtr& operator = (SharedPtr&& _rhs) { Swap(m_ptr, _rhs.m_ptr); return *this; }
		 //!
		 SharedPtr& operator = (const T* _rhs) { SAFE_ASSIGN(m_ptr, const_cast<T*>(_rhs)); return *this; }

		 //!
		 operator T* (void) const { return m_ptr; }
		 //!
		 T* operator -> (void) const { ASSERT(m_ptr != nullptr); return m_ptr; }
		 //!
		 T& operator * (void) const { ASSERT(m_ptr != nullptr); return *m_ptr; }

		 //!
		 T* Get(void) const { return m_ptr; }
		 //! Cast to another type.
		 template <class X> X* Cast(void) const { return static_cast<X*>(m_ptr); }
		 //! Dynamic cast to another type.
		 template <class X> X* DynamicCast(void) const { return dynamic_cast<X*>(m_ptr); }

		 //! Access to pointer.
		 T*& _Ptr(void) { return m_ptr; }

	 private:

		 //!
		 T* m_ptr;
	 };

	 //----------------------------------------------------------------------------//
	 // Ref 
	 //----------------------------------------------------------------------------//

	 //! The weak reference.
	 template <class T> class WeakRef
	 {
	 public:

		 //! Construct from counter. \note For internal usage.
		 WeakRef(T* _ptr, RefCounter* _ref) : m_ref(_ref) { SAFE_ADDREF(m_ref); }
		 //!
		 WeakRef(void) : m_ref(nullptr) { }
		 //!
		 WeakRef(const WeakRef& _other) : m_ref(_other.m_ref) { SAFE_ADDREF(m_ref); }
		 //!
		 WeakRef(WeakRef&& _other) : m_ref(nullptr) { Swap(m_ref, _other.m_ref); }
		 //!
		 WeakRef(const SharedPtr<T>& _ptr) : m_ref(_ptr ? _ptr->GetRef() : nullptr) { SAFE_ADDREF(m_ref); }
		 //!
		 WeakRef(const T* _ptr) : m_ref(_ptr ? _ptr->GetRef() : nullptr) { SAFE_ADDREF(m_ref); }
		 //!
		 ~WeakRef(void) { SAFE_RELEASE(m_ref); }

		 //!
		 WeakRef& operator = (const WeakRef& _rhs) { SAFE_ASSIGN(m_ref, _rhs.m_ref); return *this; }
		 //!
		 WeakRef& operator = (WeakRef&& _rhs) { Swap(m_ref, _rhs.m_ref); return *this; }
		 //!
		 WeakRef& operator = (const SharedPtr<T>& _ptr) { RefCounter* _ref = _ptr ? _ptr->GetRef() : nullptr; SAFE_ASSIGN(m_ref, _ref); return *this; }
		 //!
		 WeakRef& operator = (const T* _ptr) { RefCounter* _ref = _ptr ? _ptr->GetRef() : nullptr; SAFE_ASSIGN(m_ref, _ref); return *this; }

		 //! For compare.
		 operator void* (void) const { return Get(); }
		 //! Automatic lock. \see Rx::RefCounter::_Lock
		 operator SharedPtr<T>(void) const { return Lock().Cast<T>(); }

		 //! Get the object. \note It is not thread-safe. Use ::Lock instead. \see Rx::RefCounter::_Get
		 T* Get(void) const { return m_ref ? static_cast<T*>(m_ref->_Get()) : nullptr; }
		 //! Lock the object. \see Rx::RefCounter::_Lock
		 SharedPtr<T> Lock(void) const { return SharedPtr<T>(m_ref ? static_cast<T*>(m_ref->_Lock()) : nullptr, NoAddRef); }
		 //! Cast to another type.
		 template <class X> WeakRef<X> Cast(void) const { return Ref<X>(static_cast<X*>(Get()), m_ref); }

		 RefCounter* _GetRef(void) const { return m_ref; }

	 private:
		 //!
		 RefCounter* m_ref;
	 };

 }

 namespace std
 {
	 template<class T> struct hash<Rx::SharedPtr<T>> : public std::hash<T*>
	 {
	 };

	 template<class T> struct hash<Rx::WeakRef<T>> : public std::hash<Rx::RefCounter*>
	 {
		 size_t operator()(const Rx::WeakRef<T>& _Keyval) const
		 {
			return std::hash<Rx::RefCounter*>::operator()(_Keyval._GetRef());
		 }
	 };
 }

 namespace Rx
 {
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

	class RX_API Config
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
	// 
	//----------------------------------------------------------------------------//
}

