#pragma once

//----------------------------------------------------------------------------//
// Compiler
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
// Debug
//----------------------------------------------------------------------------//

#if defined(_DEBUG) && !defined(DEBUG)
#	define DEBUG
#endif
#if !defined(DEBUG) && !defined(NDEBUG)
#	define NDEBUG
#endif
#ifdef _DEBUG
#	define ASSERT(x, ...) assert(x && ##__VA_ARGS__ "")
#else
#	define ASSERT(x, ...)
#endif
//#define StaticAssert(cond, desc) static_assert(conde, desc)


//----------------------------------------------------------------------------//
// Platfom
//----------------------------------------------------------------------------//

#if defined(_WIN32) && !defined(WIN32)
#	define WIN32
#endif

#ifdef WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#else
#	error unknown platform
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

#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace ge
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
	typedef std::string String;
	template <class T> using Array = std::vector < T >;
	template <class T> using List = std::list < T >;
	template <class K, class V> using Map = std::map < K, V >;
	template <class K, class V> using HashMap = std::unordered_map < K, V >;
	template <class T> using HashSet = std::unordered_set < T >;

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

	template < typename T > void Swap(T& _a, T& _b) { T _c = _a; _a = _b; _b = _c; }

	//----------------------------------------------------------------------------//
	// String utils
	//----------------------------------------------------------------------------//

	const String STR_BLANK = "";
	typedef Array<String> StringArray;

	String StrFormat(const char* _fmt, ...);
	String StrFormatV(const char* _fmt, va_list _args);
	String StrUpper(const char* _str);
	inline String StrUpper(const String& _str) { return StrUpper(_str.c_str()); }
	String StrLower(const char* _str);
	inline String StrLower(const String& _str) { return StrLower(_str.c_str()); }
	void SplitString(const char* _str, const char* _delimiters, StringArray& _dst);
	inline void SplitString(const String& _str, const char* _delimiters, StringArray& _dst) { SplitString(_str.c_str(), _delimiters, _dst); }

	//----------------------------------------------------------------------------//
	// CStream
	//----------------------------------------------------------------------------//

	struct CStream
	{
		CStream(const char* _str) : s(_str), l(1), c(1) { }

		operator const char* (void) { return s; }
		char operator [] (int i) { return s[i]; }
		const char* operator ++ (void) { Next(); return s; }
		const char* operator ++ (int) { const char* p = s; Next(); return p; }
		const char* operator += (int _c) { ASSERT(_c >= 0); Next(_c); return s; }

		void Next(uint _c = 1)
		{
			while (*s && _c--)
			{
				if (strchr("\n\r", *s))
				{
					if (s[0] == '\r' && s[1] == '\n') ++s;
					c = 0;
					++l;
				}
				//c += (*s == '\t') ? (4) : 1;
				++c;
				++s;
			}
		}

		const char* s; // stream
		uint l, c; // line, column
	};

	//----------------------------------------------------------------------------//
	// CheckSum
	//----------------------------------------------------------------------------//

	extern const uint32 CRC32Table[256];

	inline uint32 CRC32(uint32 _crc, const void* _buf, uint _size)
	{
		ASSERT((_buf && _size) || !_size);
		const uint8* p = (const uint8*)_buf;
		_crc = _crc ^ ~0u;
		while (_size--) _crc = CRC32Table[(_crc ^ *p++) & 0xff] ^ (_crc >> 8);
		return _crc ^ ~0u;
	}
	inline uint32 CRC32(const void* _buf, uint _size) { return CRC32(0, _buf, _size); }
	inline uint32 CRC32(const char* _str, int _length = -1, uint32 _crc = 0) { return _str ? CRC32(_crc, _str, _length < 0 ? (uint)strlen(_str) : _length) : _crc; }
	inline uint32 CRC32(const String& _str, uint32 _crc = 0) { return CRC32(_crc, _str.c_str(), (uint)_str.length()); }

	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

#ifdef WIN32
#	define FS_IGNORE_CASE true
#else
#	define FS_IGNORE_CASE false
#endif

	bool IsFullPath(const String& _path, uint* _dp = nullptr);
	void SplitFilename(const String& _filename, String* _device = nullptr, String* _dir = nullptr, String* _name = nullptr, String* _shortname = nullptr, String* _ext = nullptr);
	inline String FilePath(const String& _filename) { String _r; SplitFilename(_filename, 0, &_r); return _r; }
	inline String FileName(const String& _filename) { String _r; SplitFilename(_filename, 0, 0, &_r); return _r; }
	inline String FileNameOnly(const String& _filename) { String _r; SplitFilename(_filename, 0, 0, 0, &_r); return _r; }
	inline String FileExt(const String& _filename) { String _r; SplitFilename(_filename, 0, 0, 0, 0, &_r); return _r; }
	void SplitPath(const String& _path, String* _device = nullptr, StringArray* _items = nullptr);
	String MakeFilename(const String& _name, const String& _ext);
	String MakePath(const String& _device, const StringArray& _items);
	String MakeFullPath(const String& _path, const String& _root = STR_BLANK);
	String MakeNormPath(const String& _path, bool _toLower = FS_IGNORE_CASE);

	//----------------------------------------------------------------------------//
	// Math
	//----------------------------------------------------------------------------//

	static const float EPSILON = 1e-6f;
	static const float EPSILON2 = 1e-12f;
	static const float PI = 3.1415926535897932384626433832795f;
	static const float DEGREES = 57.295779513082320876798154814105f;
	static const float RADIANS = 0.01745329251994329576923690768489f;

	template <typename T> const T& Min(const T& _a, const T& _b) { return _a < _b ? _a : _b; }
	template <typename T> const T& Min(const T& _a, const T& _b, const T& _c) { return _a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); }
	template <typename T> const T& Max(const T& _a, const T& _b) { return _a > _b ? _a : _b; }
	template <typename T> const T& Max(const T& _a, const T& _b, const T& _c) { return _a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); }
	template <typename T> const T Clamp(T _x, T _l, T _u) { return _x > _l ? (_x < _u ? _x : _u) : _l; }
	template <typename T> T Mix(const T& _a, const T& _b, float _t) { return _a + (_b - _a) * _t; }
	template <typename T> T Abs(T _x) { return abs(_x); }
	template <typename T> T Radians(T _val) { return _val * RADIANS; }
	template <typename T> T Degrees(T _val) { return _val * DEGREES; }
	template <typename T> T Sqr(T _x) { return _x * _x; }
	inline float Sqrt(float _x) { return sqrt(_x); }
	inline float RSqrt(float _x) { return 1 / sqrt(_x); }
	inline float Sin(float _x) { return sin(_x); }
	inline float Cos(float _x) { return cos(_x); }
	inline void SinCos(float _a, float& _s, float& _c) { _s = sin(_a), _c = cos(_a); }
	inline float Tan(float _x) { return tan(_x); }
	inline float ASin(float _x) { return asin(_x); }
	inline float ACos(float _x) { return acos(_x); }
	inline float ATan2(float _y, float _x) { return atan2(_y, _x); }
	inline float Log2(float _x) { return log2(_x); }
	inline int Log2i(int _x) { return (int)log2f((float)_x); }
	inline uint32 FirstPow2(uint32 _val) { --_val |= _val >> 16, _val |= _val >> 8, _val |= _val >> 4, _val |= _val >> 2, _val |= _val >> 1; return ++_val; }
	inline bool IsPow2(uint32 _val) { return (_val & (_val - 1)) == 0; }

	//----------------------------------------------------------------------------//
	// Vec2T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec2T
	{
		Vec2T(void) { }
		Vec2T(T _xy) : x(_xy), y(_xy) { }
		Vec2T(T _x, T _y) : x(_x), y(_y) { }
		Vec2T Copy(void) const { return *this; }
		template <typename X> explicit Vec2T(X _x, X _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }
		template <typename X> X Cast(void) const { return X(x, y); }

		const T operator [] (uint _index) const { return v[_index]; }
		T& operator [] (uint _index) { return v[_index]; }
		const T* operator * (void) const { return v; }
		T* operator * (void) { return v; }

		Vec2T operator - (void) const { return Vec2T(-x, -y); }

		Vec2T operator + (const Vec2T& _rhs) const { return Vec2T(x + _rhs.x, y + _rhs.y); }
		Vec2T operator - (const Vec2T& _rhs) const { return Vec2T(x - _rhs.x, y - _rhs.y); }
		Vec2T operator * (T _rhs) const { return Vec2T(x * _rhs, y * _rhs); }
		Vec2T operator / (T _rhs) const { return Vec2T(x / _rhs, y / _rhs); }
		Vec2T& operator += (const Vec2T& _rhs) { x += _rhs.x, y += _rhs.y; return *this; }
		Vec2T& operator -= (const Vec2T& _rhs) { x -= _rhs.x, y -= _rhs.y; return *this; }
		Vec2T& operator *= (T _rhs) { x *= _rhs, y *= _rhs; return *this; }
		Vec2T& operator /= (T _rhs) { x /= _rhs, y /= _rhs; return *this; }

		bool operator == (const Vec2T& _rhs) const { return x == _rhs.x && y == _rhs.y; }
		bool operator != (const Vec2T& _rhs) const { return x != _rhs.x || y != _rhs.y; }

		Vec2T& Set(T _xy) { x = _xy, y = _xy; return *this; }
		Vec2T& Set(T _x, T _y) { x = _x, y = _y; return *this; }
		Vec2T& Set(const Vec2T& _other) { return *this = _other; }

		union
		{
			struct { T x, y; };
			T v[2];
		};

		static const Vec2T ZERO;
		static const Vec2T ONE;
		static const Vec2T UNIT_X;
		static const Vec2T UNIT_Y;
	};

	template <typename T> const Vec2T<T> Vec2T<T>::ZERO(0);
	template <typename T> const Vec2T<T> Vec2T<T>::ONE(1);
	template <typename T> const Vec2T<T> Vec2T<T>::UNIT_X(1, 0);
	template <typename T> const Vec2T<T> Vec2T<T>::UNIT_Y(0, 1);

	typedef Vec2T<float> Vec2;
	typedef Vec2T<int> Vec2i;
	typedef Vec2T<uint> Vec2ui;

	//----------------------------------------------------------------------------//
	// Vec3T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec3T
	{
		Vec3T(void) { }
		Vec3T(T _xyz) : x(_xyz), y(_xyz), z(_xyz) { }
		Vec3T(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }
		Vec3T Copy(void) const { return *this; }
		template <typename X> X Cast(void) const { return X(x, y, z); }

		const T operator [] (uint _index) const { return v[_index]; }
		T& operator [] (uint _index) { return v[_index]; }
		const T* operator * (void) const { return v; }
		T* operator * (void) { return v; }

		Vec3T operator - (void) const { return Vec3T(-x, -y, -z); }

		Vec3T operator + (const Vec3T& _rhs) const { return Vec3T(x + _rhs.x, y + _rhs.y, z + _rhs.z); }
		Vec3T operator - (const Vec3T& _rhs) const { return Vec3T(x - _rhs.x, y - _rhs.y, z - _rhs.z); }
		Vec3T operator * (const Vec3T& _rhs) const { return Vec3T(x * _rhs.x, y * _rhs.y, z * _rhs.z); }
		Vec3T operator / (const Vec3T& _rhs) const { return Vec3T(x / _rhs.x, y / _rhs.y, z / _rhs.z); }
		Vec3T operator * (T _rhs) const { return Vec3T(x * _rhs, y * _rhs, z * _rhs); }
		Vec3T operator / (T _rhs) const { return Vec3T(x / _rhs, y / _rhs, z / _rhs); }
		Vec3T& operator += (const Vec3T& _rhs) { x += _rhs.x, y += _rhs.y, z += _rhs.z; return *this; }
		Vec3T& operator -= (const Vec3T& _rhs) { x -= _rhs.x, y -= _rhs.y, z -= _rhs.z; return *this; }
		Vec3T& operator *= (const Vec3T& _rhs) { x *= _rhs.x, y *= _rhs.y, z *= _rhs.z; return *this; }
		Vec3T& operator /= (const Vec3T& _rhs) { x /= _rhs.x, y /= _rhs.y, z /= _rhs.z; return *this; }
		Vec3T& operator *= (T _rhs) { x *= _rhs, y *= _rhs, z *= _rhs; return *this; }
		Vec3T& operator /= (T _rhs) { x /= _rhs, y /= _rhs, z /= _rhs; return *this; }
		friend Vec3T operator / (T _lhs, const Vec3T& _rhs) { return Vec3T(_lhs / _rhs.x, _lhs / _rhs.y, _lhs / _rhs.z); }
		friend Vec3T operator * (T _lhs, const Vec3T& _rhs) { return Vec3T(_lhs * _rhs.x, _lhs * _rhs.y, _lhs * _rhs.z); }

		bool operator == (const Vec3T& _rhs) const { return x == _rhs.x && y == _rhs.y && z == _rhs.z; }
		bool operator != (const Vec3T& _rhs) const { return x != _rhs.x || y != _rhs.y || z != _rhs.z; }

		Vec3T& Set(T _xyz) { x = _xyz, y = _xyz, z = _xyz; return *this; }
		Vec3T& Set(T _x, T _y, T _z) { x = _x, y = _y, z = _z; return *this; }
		Vec3T& Set(const Vec3T& _other) { return *this = _other; }

		// for Vec3T<float>
		Vec3T<float>& SetMin(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Min(_a.x, _b.x), y = Min(_a.y, _b.y), z = Min(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMin(const Vec3T<float>& _other) { return SetMin(*this, _other); }
		Vec3T<float>& SetMax(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Max(_a.x, _b.x), y = Max(_a.y, _b.y), z = Max(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMax(const Vec3T<float>& _other) { return SetMax(*this, _other); }
		Vec3T<float>& Normalize(void) { float _l = LengthSq(); if (_l > EPSILON2) *this *= RSqrt(_l); return *this; }
		Vec3T<float>& NormalizeFast(void) { return *this *= LengthInv(); }
		float LengthSq(void) const { return x * x + y * y + z * z; }
		float Length(void) const { return Sqrt(x * x + y * y + z * z); }
		float LengthInv(void) const { return Sqrt(x * x + y * y + z * z); }
		float Dot(const Vec3T<float>& _rhs) const { return x * _rhs.x + y * _rhs.y + z * _rhs.z; }
		float AbsDot(const Vec3T<float>& _rhs) const { return Abs(x * _rhs.x) + Abs(y * _rhs.y) + Abs(z * _rhs.z); }
		Vec3T<float> Cross(const Vec3T<float>& _rhs) const { return Vec3T<float>(y * _rhs.z - z * _rhs.y, z * _rhs.x - x * _rhs.z, x * _rhs.y - y * _rhs.x); }
		float Distance(const Vec3T<float>& _v) const { return (*this - _v).Length(); }
		float DistanceSq(const Vec3T<float>& _v) const { return (*this - _v).LengthSq(); }
		Vec3T<float> MidPoint(const Vec3T<float>& _v) const { return (*this + _v) * 0.5f; }
		Vec3T<float> Perpendicular(void) const { Vec3T<float> _perp = Cross(UNIT_X); if (_perp.LengthSq() <= EPSILON2) _perp = Cross(UNIT_Y); return _perp.Normalize(); }
		float Angle(const Vec3T<float>& _v) const { float _lp = LengthSq() * _v.LengthSq(); if (_lp > EPSILON2) _lp = RSqrt(_lp); return ACos(Min< float >(Max< float >(-1, Dot(_v) * _lp), 1)); }
		Vec3T<float> Reflect(const Vec3T<float>& _n) const { return Vec3T<float>(*this - (2 * Dot(_n) * _n)); }

		union
		{
			struct { T x, y, z; };
			T v[3];
		};

		static const Vec3T ZERO;
		static const Vec3T ONE;
		static const Vec3T UNIT_X;
		static const Vec3T UNIT_Y;
		static const Vec3T UNIT_Z;
	};

	template <typename T> const Vec3T<T> Vec3T<T>::ZERO(0);
	template <typename T> const Vec3T<T> Vec3T<T>::ONE(1);
	template <typename T> const Vec3T<T> Vec3T<T>::UNIT_X(1, 0, 0);
	template <typename T> const Vec3T<T> Vec3T<T>::UNIT_Y(0, 1, 0);
	template <typename T> const Vec3T<T> Vec3T<T>::UNIT_Z(0, 0, 1);

	typedef Vec3T<float> Vec3;
	typedef Vec3T<int> Vec3i;
	typedef Vec3T<uint> Vec3ui;

	//----------------------------------------------------------------------------//
	// Vec4T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec4T
	{
		Vec4T(void) { }
		Vec4T(T _xyzw) : x(_xyzw), y(_xyzw), z(_xyzw), w(_xyzw) { }
		Vec4T(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }
		Vec4T Copy(void) const { return *this; }
		template <typename X> X Cast(void) const { return X(x, y, z, w); }

		const T operator [] (uint _index) const { return v[_index]; }
		T& operator [] (uint _index) { return v[_index]; }
		const T* operator * (void) const { return v; }
		T* operator * (void) { return v; }

		Vec4T operator - (void) const { return Vec4T(-x, -y, -z, -w); }

		Vec4T operator + (const Vec4T& _rhs) const { return Vec4T(x + _rhs.x, y + _rhs.y, z + _rhs.z, w + _rhs.w); }
		Vec4T operator - (const Vec4T& _rhs) const { return Vec4T(x - _rhs.x, y - _rhs.y, z - _rhs.z, w - _rhs.w); }
		Vec4T operator * (T _rhs) const { return Vec4T(x * _rhs, y * _rhs, z * _rhs, w * _rhs); }
		Vec4T operator / (T _rhs) const { return Vec4T(x / _rhs, y / _rhs, z / _rhs, w / _rhs); }
		Vec4T& operator += (const Vec4T& _rhs) { x += _rhs.x, y += _rhs.y, z += _rhs.z, w += _rhs.w; return *this; }
		Vec4T& operator -= (const Vec4T& _rhs) { x -= _rhs.x, y -= _rhs.y, z -= _rhs.z, w -= _rhs.w; return *this; }
		Vec4T& operator *= (T _rhs) { x *= _rhs, y *= _rhs, z *= _rhs, w *= _rhs; return *this; }
		Vec4T& operator /= (T _rhs) { x /= _rhs, y /= _rhs, z /= _rhs, w /= _rhs; return *this; }

		Vec4T& Set(T _xyzw) { x = _xyzw, y = _xyzw, z = _xyzw, w = _xyzw; return *this; }
		Vec4T& Set(T _x, T _y, T _z, T _w) { x = _x, y = _y, z = _z, w = _w; return *this; }
		Vec4T& Set(const Vec4T& _other) { return *this = _other; }

		union
		{
			struct { T x, y, z, w; };
			T v[4];
		};

		static const Vec4T ZERO;
		static const Vec4T ONE;
		static const Vec4T IDENTITY;
	};

	template <typename T> const Vec4T<T> Vec4T<T>::ZERO(0);
	template <typename T> const Vec4T<T> Vec4T<T>::ONE(1);
	template <typename T> const Vec4T<T> Vec4T<T>::IDENTITY(0, 0, 0, 1);

	typedef Vec4T<float> Vec4;
	typedef Vec4T<int> Vec4i;
	typedef Vec4T<uint8> Vec4ub;

	//----------------------------------------------------------------------------//
	// Quat
	//----------------------------------------------------------------------------//

	struct Quat
	{
		Quat(void) { }
		Quat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
		Quat Copy(void) const { return *this; }
		template <typename X> X Cast(void) const { return X(x, y, z, w); }

		const float operator [] (uint _index) const { return v[_index]; }
		float& operator [] (uint _index) { return v[_index]; }
		const float* operator * (void) const { return v; }
		float* operator * (void) { return v; }

		Quat operator - (void) const { return Quat(-x, -y, -z, -w); }

		Quat operator + (const Quat& _rhs) const { return Quat(x + _rhs.x, y + _rhs.y, z + _rhs.z, w + _rhs.w); }
		Quat operator - (const Quat& _rhs) const { return Quat(x - _rhs.x, y - _rhs.y, z - _rhs.z, w - _rhs.w); }
		Quat operator * (const Quat& _rhs) const { return Copy().Multiply(_rhs); }
		Quat operator * (float _rhs) const { return Quat(x * _rhs, y * _rhs, z * _rhs, w * _rhs); }
		Quat operator / (float _rhs) const { return Quat(x / _rhs, y / _rhs, z / _rhs, w / _rhs); }
		Quat& operator += (const Quat& _rhs) { x += _rhs.x, y += _rhs.y, z += _rhs.z, w += _rhs.w; return *this; }
		Quat& operator -= (const Quat& _rhs) { x -= _rhs.x, y -= _rhs.y, z -= _rhs.z, w -= _rhs.w; return *this; }
		Quat& operator *= (const Quat& _rhs) { return Multiply(_rhs); }
		Quat& operator *= (float _rhs) { x += _rhs, y += _rhs, z += _rhs, w += _rhs; return *this; }
		Quat& operator /= (float _rhs) { x += _rhs, y += _rhs, z += _rhs, w += _rhs; return *this; }
		friend Quat operator * (float _lhs, const Quat& _rhs) { return Quat(_lhs * _rhs.x, _lhs * _rhs.y, _lhs * _rhs.z, _lhs * _rhs.w); }
		friend Vec3 operator * (const Vec3& _lhs, const Quat& _rhs) { const Vec3& _q = *(Vec3*)_rhs.v; Vec3 _uv(_q.Cross(_lhs)); return _lhs + _uv * 2 * _rhs.w + _q.Cross(_uv) * 2; }

		Quat& Set(float _x, float _y, float _z, float _w) { x = _x, y = _y, z = _z, w = _w; return *this; }
		Quat& Set(const Quat& _other) { return *this = _other; }

		Quat& Multiply(const Quat& _rhs)
		{
			return Set(w * _rhs.x + x * _rhs.w + y * _rhs.z - z * _rhs.y,
				w * _rhs.y + y * _rhs.w + z * _rhs.x - x * _rhs.z,
				w * _rhs.z + z * _rhs.w + x * _rhs.y - y * _rhs.x,
				w * _rhs.w - x * _rhs.x - y * _rhs.y - z * _rhs.z);
		}
		float Dot(const Quat& _q) const { return x * _q.x + y * _q.y + z * _q.z + w * _q.w; }
		//float Norm(void) const { return x * x + y * y + z * z + w * w; }
		Quat& Normalize(void) { float _l = x * x + y * y + z * z + w * w; if (_l > EPSILON2) *this *= RSqrt(_l); return *this; }
		Quat& Inverse(void) { float _d = x * x + y * y + z * z + w * w; _d > 0 ? x *= -_d, y *= -_d, z *= -_d, w *= _d : x = 0, y = 0, z = 0, w = 1; return *this; }
		Quat& UnitInverse(void) { x = -x, y = -y, z = -z; return *this; }
		Quat Nlerp(const Quat& _q, float _t, bool _shortestPath = false) const
		{
			const Quat& _p = *this;
			float _c = _p.Dot(_q);
			Quat _result;
			if (_c < 0 && _shortestPath) _result = _p + _t * ((-_q) - _p);
			else _result = _p + _t * (_q - _p);
			return _result.Normalize();
		}
		Quat Slerp(const Quat& _q, float _t, bool _shortestPath = false) const
		{
			const Quat& _p = *this;
			float _c = _p.Dot(_q);
			Quat _tmp;
			if (_c < 0 && _shortestPath) _c = -_c, _tmp = -_q;
			else _tmp = _q;
			if (Abs(_c) < 1 - EPSILON)
			{
				float _s = Sqrt(1 - _c * _c);
				float _angle = ATan2(_s, _c);
				float _invs = 1 / _s;
				float _coeff0 = Sin((1 - _t) * _angle) * _invs;
				float _coeff1 = Sin(_t * _angle) * _invs;
				return _coeff0 * _p + _coeff1 * _tmp;
			}
			return Quat((1 - _t) * _p + _t * _tmp).Normalize();
		}

		void ToMatrixRows(float* _r0, float* _r1, float* _r2) const
		{
			float _x2 = x + x, _y2 = y + y, _z2 = z + z;
			float _wx = _x2 * w, _wy = _y2 * w, _wz = _z2 * w, _xx = _x2 * x, _xy = _y2 * x, _xz = _z2 * x, _yy = _y2 * y, _yz = _z2 * y, _zz = _z2 * z;
			_r0[0] = 1 - (_yy + _zz), _r0[1] = _xy + _wz, _r0[2] = _xz - _wy;
			_r1[0] = _xy - _wz, _r1[1] = 1.0f - (_xx + _zz), _r1[2] = _yz + _wx;
			_r2[0] = _xz + _wy, _r2[1] = _yz - _wx, _r2[2] = 1 - (_xx + _yy);
		}
		Quat& FromMatrixRows(const float* _r0, const float* _r1, const float* _r2)
		{
			float _invr, _root = _r0[0] + _r1[1] + _r2[2];
			if (_root > 0)
			{
				_root = Sqrt(_root + 1); _invr = 0.5f / _root;
				return Set((_r1[2] - _r2[1]) * _invr, (_r2[0] - _r0[2]) * _invr, (_r0[1] - _r1[0]) * _invr, _root * 0.5f);
			}
			if (_r0[0] > _r1[1] && _r0[0] > _r2[2])
			{
				_root = Sqrt(_r0[0] - _r1[1] - _r2[2] + 1); _invr = 0.5f / _root;
				return Set(_root * 0.5f, (_r0[1] + _r1[0]) * _invr, (_r0[2] + _r2[0]) * _invr, (_r1[2] - _r2[1]) * _invr);
			}
			else if (_r1[1] > _r0[0] && _r1[1] > _r2[2])
			{
				_root = Sqrt(_r1[1] - _r2[2] - _r0[0] + 1); _invr = 0.5f / _root;
				return Set((_r1[0] + _r0[1]) * _invr, _root * 0.5f, (_r1[2] + _r2[1]) * _invr, (_r2[0] - _r0[2]) * _invr);
			}
			_root = Sqrt(_r2[2] - _r0[0] - _r1[1] + 1); _invr = 0.5f / _root;
			return Set((_r2[0] + _r0[2]) * _invr, (_r2[1] + _r1[2]) * _invr, _root * 0.5f, (_r0[1] - _r1[0]) * _invr);
		}

		Quat& FromAxisAngle(const Vec3& _axis, float _angle)
		{
			float _s, _c;
			SinCos(_angle*0.5f, _s, _c);
			return Set(_axis.x * _s, _axis.y * _s, _axis.z * _s, _c);
		}

		union
		{
			struct { float x, y, z, w; };
			float v[4];
		};

		static const Quat ZERO;
		static const Quat IDENTITY;
	};

	inline Quat Mix(const Quat& _a, const Quat& _b, float _t) { return _a.Slerp(_b, _t); }

	//----------------------------------------------------------------------------//
	// Mat34
	//----------------------------------------------------------------------------//

	struct Mat34
	{
		Mat34(void) { }
		Mat34(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23) : m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23) { }
		explicit Mat34(const float* _m34) { memcpy(v, _m34, 12 * sizeof(float)); }
		Mat34 Copy(void) const { return *this; }

		const float* operator [] (int _row) const { return m[_row]; }
		float* operator [] (int _row) { return m[_row]; }
		const float* operator * (void) const { return v; }
		float* operator * (void) { return v; }
		float& operator () (uint _row, uint _col) { return m[_row][_col]; }
		const float operator () (uint _row, uint _col) const { return m[_row][_col]; }
		const Vec3& Row(uint _row) const { return *(Vec3*)(m[_row]); }
		Vec3& Row(uint _row) { return *(Vec3*)(m[_row]); }

		Mat34 operator + (const Mat34& _rhs) const { return Copy().Add(_rhs); }
		Mat34 operator * (const Mat34& _rhs) const { return Copy().Multiply(_rhs); }
		Mat34 operator * (float _rhs) const { return Copy().Multiply(_rhs); }
		friend Vec3 operator * (const Vec3& _lhs, const Mat34& _rhs) { return _rhs.Transform(_lhs); } // Vec3 = Vec3 * Mat34
		Mat34& operator += (const Mat34& _rhs) { return Add(_rhs); }
		Mat34& operator *= (const Mat34& _rhs) { return Multiply(_rhs); }
		Mat34& operator *= (float _rhs) { return Multiply(_rhs); }

		Mat34& Add(const Mat34& _rhs) { for (int i = 0; i < 12; ++i) v[i] += v[i]; return *this; }
		Mat34& Multiply(float _rhs) { for (int i = 0; i < 12; ++i) v[i] *= _rhs; return *this; }
		Mat34& Multiply(const Mat34& _rhs)
		{
			float _tmp[12];
			for (int l = 0; l < 12; l += 4)
			{
				for (int r = 0; r < 3; ++r)
				{
					_tmp[l + r] = v[l] * _rhs.v[r] + v[l + 1] * _rhs.v[r + 4] + v[l + 2] * _rhs.v[r + 8];
				}
				_tmp[l + 3] = v[l] * _rhs.v[3] + v[l + 1] * _rhs.v[7] + v[l + 2] * _rhs.v[11] + v[l + 3];
			}
			memcpy(v, _tmp, sizeof(float) * 12);
			return *this;
		}
		float Determinant(void) const { return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 - m02 * m11 * m20 - m00 * m12 * m21 - m01 * m10 * m22; }
		Mat34& Inverse(void)
		{
			float _m00 = m00, _m01 = m01, _m02 = m02, _m03 = m03, _m10 = m10, _m11 = m11, _m12 = m12, _m13 = m13, _m20 = m20, _m21 = m21, _m22 = m22, _m23 = m23;
			float _v0 = (_m23 * _m10 - _m20 * _m13), _v1 = (_m23 * _m11 - _m21 * _m13), _v2 = (_m23 * _m12 - _m22 * _m13);
			float _t00 = +(_m22 * _m11 - _m21 * _m12), _t10 = -(_m22 * _m10 - _m20 * _m12), _t20 = +(_m21 * _m10 - _m20 * _m11);
			float _invdet = 1 / (_t00 * _m00 + _t10 * _m01 + _t20 * _m02);
			m00 = _t00 * _invdet;
			m10 = _t10 * _invdet;
			m20 = _t20 * _invdet;
			m01 = -(_m22 * _m01 - _m21 * _m02) * _invdet;
			m11 = +(_m22 * _m00 - _m20 * _m02) * _invdet;
			m21 = -(_m21 * _m00 - _m20 * _m01) * _invdet;
			m02 = +(_m12 * _m01 - _m11 * _m02) * _invdet;
			m12 = -(_m12 * _m00 - _m10 * _m02) * _invdet;
			m22 = +(_m11 * _m00 - _m10 * _m01) * _invdet;
			m03 = -(_v2 * _m01 - _v1 * _m02 + (_m22 * _m11 - _m21 * _m12) * _m03) * _invdet;
			m13 = +(_v2 * _m00 - _v0 * _m02 + (_m22 * _m10 - _m20 * _m12) * _m03) * _invdet;
			m23 = -(_v1 * _m00 - _v0 * _m01 + (_m21 * _m10 - _m20 * _m11) * _m03) * _invdet;
			return *this;
		}

		Vec3 Translate(const Vec3& _v) const { return Vec3(_v.x + m03, _v.y + m13, _v.z + m23); }
		Vec3 Transform(const Vec3& _v) const
		{
			return Vec3(m00 * _v.x + m01 * _v.y + m02 * _v.z + m03, m10 * _v.x + m11 * _v.y + m12 * _v.z + m13, m20 * _v.x + m21 * _v.y + m22 * _v.z + m23);
		}
		Vec3 TransformVectorAbs(const Vec3& _v) const
		{
			return Vec3(Abs(m00) * _v.x + Abs(m01) * _v.y + Abs(m02) * _v.z, Abs(m10) * _v.x + Abs(m11) * _v.y + Abs(m12) * _v.z, Abs(m20) * _v.x + Abs(m21) * _v.y + Abs(m22) * _v.z);
		}
		Vec3 TransformVector(const Vec3& _v) const
		{
			return Vec3(m00 * _v.x + m01 * _v.y + m02 * _v.z, m10 * _v.x + m11 * _v.y + m12 * _v.z, m20 * _v.x + m21 * _v.y + m22 * _v.z);
		}

		Mat34& SetTranslation(const Vec3& _translation) { m03 = _translation.x, m13 = _translation.y, m23 = _translation.z; return *this; }
		Vec3 GetTranslation(void) const { return Vec3(m03, m12, m23); }
		Mat34& CreateTranslation(const Vec3& _translation) { return (*this = IDENTITY).SetTranslation(_translation); }

		Mat34& SetRotation(const Quat& _rotation)
		{
			Vec3 _scale = GetScale();
			_rotation.ToMatrixRows(m[0], m[1], m[2]);
			Row(0) *= _scale, Row(1) *= _scale, Row(2) *= _scale;
			return *this;
		}
		Quat GetRotation(void) const
		{
			Vec3 _m0(m00, m10, m20);
			Vec3 _m1(m01, m11, m21);
			Vec3 _m2(m02, m12, m22);
			Vec3 _q0 = _m0.Copy().Normalize();
			Vec3 _q1 = (_m1 - _q0 * _q0.Dot(_m1)).Normalize();
			Vec3 _q2 = ((_m2 - _q0 * _q0.Dot(_m2)) - _q1 * _q1.Dot(_m2)).Normalize();
			float _det = _q0[0] * _q1[1] * _q2[2] + _q0[1] * _q1[2] * _q2[0] + _q0[2] * _q1[0] * _q2[1] - _q0[2] * _q1[1] * _q2[0] - _q0[1] * _q1[0] * _q2[2] - _q0[0] * _q1[2] * _q2[1];
			if (_det < 0) _q0 = -_q0, _q1 = -_q1, _q2 = -_q2;
			return Quat().FromMatrixRows(*_q0, *_q1, *_q2);
		}
		Mat34& CreateRotation(const Quat& _rotation)
		{
			SetTranslation(Vec3::ZERO);
			_rotation.ToMatrixRows(m[0], m[1], m[2]);
			return *this;
		}

		bool HasScale(void) const
		{
			if (!(Abs((m00 * m00 + m10 * m10 + m20 * m20) - 1) < EPSILON)) return true;
			if (!(Abs((m01 * m01 + m11 * m11 + m21 * m21) - 1) < EPSILON)) return true;
			if (!(Abs((m02 * m02 + m12 * m12 + m22 * m22) - 1) < EPSILON)) return true;
			return false;
		}
		bool HasNegativeScale(void) const { return Determinant() < 0; }
		Mat34& SetScale(const Vec3& _scale)
		{
			Quat _rotation = GetRotation();
			_rotation.ToMatrixRows(m[0], m[1], m[2]);
			Row(0) *= _scale, Row(1) *= _scale, Row(2) *= _scale;
			return *this;
		}
		Vec3 GetScale(void) const
		{
			return Vec3(Sqrt(m00 * m00 + m10 * m10 + m20 * m20), Sqrt(m01 * m01 + m11 * m11 + m21 * m21), Sqrt(m02 * m02 + m12 * m12 + m22 * m22));
		}
		Mat34& CreateScale(const Vec3& _scale) { *this = ZERO; m00 = _scale.x, m11 = _scale.y, m22 = _scale.z; return *this; }

		Mat34& CreateTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::ONE)
		{
			// Ordering: Scale, Rotate, Translate
			float _r0[3], _r1[3], _r2[3];
			_rotation.ToMatrixRows(_r0, _r1, _r2);
			m00 = _scale.x * _r0[0], m01 = _scale.y * _r0[1], m02 = _scale.z * _r0[2], m03 = _translation.x;
			m10 = _scale.x * _r1[0], m11 = _scale.y * _r1[1], m12 = _scale.z * _r1[2], m13 = _translation.y;
			m20 = _scale.x * _r2[0], m21 = _scale.y * _r2[1], m22 = _scale.z * _r2[2], m23 = _translation.z;
			return *this;
		}
		Mat34& CreateInverseTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::ONE)
		{
			Vec3 _inv_scale(1 / _scale);
			Quat _inv_rotation = _rotation.Copy().Inverse();
			Vec3 _inv_translation((-_translation * _inv_rotation) * _inv_scale);
			return CreateTransform(_inv_translation, _inv_rotation, _inv_scale);
		}

		union
		{
			struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23; };
			float m[3][4]; // [row][col]
			float v[12];
		};

		static const Mat34 ZERO;
		static const Mat34 IDENTITY;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct Mat44
	{
		Mat44(void) { }
		Mat44(const Mat34& _m34) : m30(0), m31(0), m32(0), m33(1) { memcpy(v, *_m34, 12 * sizeof(float)); }
		explicit Mat44(const float* _m44) { memcpy(v, _m44, 16 * sizeof(float)); }
		Mat44(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33) : m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23), m30(_30), m31(_31), m32(_32), m33(_33) { }
		Mat44 Copy(void) const { return *this; }

		operator const Mat34& (void) const { return *(Mat34*)(v); /*ASSERT(IsAffine())*/ }
		const float* operator [] (int _row) const { return m[_row]; }
		float* operator [] (int _row) { return m[_row]; }
		const float* operator * (void) const { return v; }
		float* operator * (void) { return v; }
		float& operator () (uint _row, uint _col) { return m[_row][_col]; }
		const float operator () (uint _row, uint _col) const { return m[_row][_col]; }
		const Vec3& Row(uint _row) const { return *(Vec3*)(m[_row]); }
		Vec3& Row(uint _row) { return *(Vec3*)(m[_row]); }
		const Vec4& Row4(uint _row) const { return *(Vec4*)(m[_row]); }
		Vec4& Row4(uint _row) { return *(Vec4*)(m[_row]); }

		Mat44 operator + (const Mat44& _rhs) const { return Copy().Add(_rhs); }
		Mat44 operator * (const Mat44& _rhs) const { return Copy().Multiply(_rhs); }
		Mat44 operator * (const Mat34& _rhs) const { return Copy().Multiply(_rhs); }
		Mat44 operator * (float _rhs) const { return Copy().Multiply(_rhs); }
		friend Vec3 operator * (const Vec3& _lhs, const Mat44& _rhs) { return _rhs.Transform(_lhs); } // Vec3 = Vec3 * Mat44
		Mat44& operator += (const Mat44& _rhs) { return Add(_rhs); }
		Mat44& operator *= (const Mat44& _rhs) { return Multiply(_rhs); }
		Mat44& operator *= (const Mat34& _rhs) { return Multiply(_rhs); }
		Mat44& operator *= (float _rhs) { return Multiply(_rhs); }

		Mat44& Add(const Mat44& _rhs)
		{
			for (int i = 0; i < 16; ++i) v[i] += _rhs.v[i];
			return *this;
		}
		Mat44& Multiply(float _rhs)
		{
			for (int i = 0; i < 16; ++i) v[i] += _rhs;
			return *this;
		}
		Mat44& Multiply(const Mat44& _rhs)
		{
			float _tmp[16];
			for (int l = 0; l < 16; l += 4)
			{
				for (int r = 0; r < 4; ++r)
				{
					_tmp[l + r] = v[l] * _rhs.v[r] + v[l + 1] * _rhs.v[r + 4] + v[l + 2] * _rhs.v[r + 8] + v[l + 3] * _rhs.v[r + 12];
				}
			}
			memcpy(v, _tmp, sizeof(float) * 16);
			return *this;
		}
		Mat44& Multiply(const Mat34& _rhs)
		{
			float _tmp[16];
			for (int l = 0; l < 16; l += 4)
			{
				for (int r = 0; r < 3; ++r)
				{
					_tmp[l + r] = v[l] * _rhs.v[r] + v[l + 1] * _rhs.v[r + 4] + v[l + 2] * _rhs.v[r + 8];
				}
				_tmp[l + 3] = v[l] * _rhs.v[3] + v[l + 1] * _rhs.v[7] + v[l + 2] * _rhs.v[11] + (l < 12 ? v[l + 3] : 1);
			}
			memcpy(v, _tmp, sizeof(float) * 16);
			return *this;
		}
		Mat44& Inverse(void)
		{
			float _m00 = m00, _m01 = m01, _m02 = m02, _m03 = m03, _m10 = m10, _m11 = m11, _m12 = m12, _m13 = m13, _m20 = m20, _m21 = m21, _m22 = m22, _m23 = m23, _m30 = m30, _m31 = m31, _m32 = m32, _m33 = m33;
			float _v0 = (_m20 * _m31 - _m21 * _m30), _v1 = (_m20 * _m32 - _m22 * _m30), _v2 = (_m20 * _m33 - _m23 * _m30), _v3 = (_m21 * _m32 - _m22 * _m31), _v4 = (_m21 * _m33 - _m23 * _m31), _v5 = (_m22 * _m33 - _m23 * _m32);
			float _t00 = +(_v5 * _m11 - _v4 * _m12 + _v3 * _m13), _t10 = -(_v5 * _m10 - _v2 * _m12 + _v1 * _m13), _t20 = +(_v4 * _m10 - _v2 * _m11 + _v0 * _m13), _t30 = -(_v3 * _m10 - _v1 * _m11 + _v0 * _m12);
			float _invdet = 1 / (_t00 * _m00 + _t10 * _m01 + _t20 * _m02 + _t30 * _m03);
			m00 = _t00 * _invdet;
			m10 = _t10 * _invdet;
			m20 = _t20 * _invdet;
			m30 = _t30 * _invdet;
			m01 = -(_v5 * _m01 - _v4 * _m02 + _v3 * _m03) * _invdet;
			m11 = +(_v5 * _m00 - _v2 * _m02 + _v1 * _m03) * _invdet;
			m21 = -(_v4 * _m00 - _v2 * _m01 + _v0 * _m03) * _invdet;
			m31 = +(_v3 * _m00 - _v1 * _m01 + _v0 * _m02) * _invdet;
			_v0 = (_m10 * _m31 - _m11 * _m30);
			_v1 = (_m10 * _m32 - _m12 * _m30);
			_v2 = (_m10 * _m33 - _m13 * _m30);
			_v3 = (_m11 * _m32 - _m12 * _m31);
			_v4 = (_m11 * _m33 - _m13 * _m31);
			_v5 = (_m12 * _m33 - _m13 * _m32);
			m02 = +(_v5 * _m01 - _v4 * _m02 + _v3 * _m03) * _invdet;
			m12 = -(_v5 * _m00 - _v2 * _m02 + _v1 * _m03) * _invdet;
			m22 = +(_v4 * _m00 - _v2 * _m01 + _v0 * _m03) * _invdet;
			m32 = -(_v3 * _m00 - _v1 * _m01 + _v0 * _m02) * _invdet;
			_v0 = (_m21 * _m10 - _m20 * _m11);
			_v1 = (_m22 * _m10 - _m20 * _m12);
			_v2 = (_m23 * _m10 - _m20 * _m13);
			_v3 = (_m22 * _m11 - _m21 * _m12);
			_v4 = (_m23 * _m11 - _m21 * _m13);
			_v5 = (_m23 * _m12 - _m22 * _m13);
			m03 = -(_v5 * _m01 - _v4 * _m02 + _v3 * _m03) * _invdet;
			m13 = +(_v5 * _m00 - _v2 * _m02 + _v1 * _m03) * _invdet;
			m23 = -(_v4 * _m00 - _v2 * _m01 + _v0 * _m03) * _invdet;
			m33 = +(_v3 * _m00 - _v1 * _m01 + _v0 * _m02) * _invdet;
			return *this;
		}

		Vec3 Transform(const Vec3& _v) const
		{
			float _iw = 1 / (m30 * _v.x + m31 * _v.y + m32 * _v.z + m33);
			return Vec3((m00 * _v.x + m01 * _v.y + m02 * _v.z + m03) * _iw,
				(m10 * _v.x + m11 * _v.y + m12 * _v.z + m13) * _iw,
				(m20 * _v.x + m21 * _v.y + m22 * _v.z + m23) * _iw);
		}

		Mat44& CreatePerspective(float _fov, float _aspect, float _near, float _far, bool _reversed = false)
		{
			float _h = 1 / tanf(_fov * .5f);
			float _w = _h / _aspect;
			float _d = _far - _near;
			float _q = -(_far + _near) / _d;
			float _qn = -2 * (_far * _near) / _d;
			if (_reversed) Swap(_q, _qn);
			*this = ZERO;
			m00 = _w, m11 = _h, m22 = _q, m32 = -1, m23 = _qn;
			return *this;
		}

		Mat44& CreateOrtho2D(float _w, float _h, bool _leftTop = true)
		{
			*this = ZERO;
			v[0] = 2 / _w, v[3] = -1, v[10] = -1, v[15] = 1;
			if (_leftTop) v[5] = -2 / _h, v[7] = 1;
			else v[5] = 2 / _h, v[7] = -1;
			return *this;
		}


		union
		{
			struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
			float m[4][4]; // [row][col]
			float v[16];
		};

		static const Mat44 ZERO;
		static const Mat44 IDENTITY;
	};


	//----------------------------------------------------------------------------//
	// Ray
	//----------------------------------------------------------------------------//

	///\brief Ëó÷
	struct Ray
	{
		Ray(void) : origin(Vec3::ZERO), dir(Vec3::UNIT_Z) { }
		Ray(const Vec3& _origin, const Vec3& _dir) : origin(_origin), dir(_dir) { }
		Vec3 Point(float _t) const { return origin + dir * _t; }
		Vec3 operator * (float _t) const { return Point(_t); }
		friend Vec3 operator * (float _t, const Ray& _ray) { return _ray.Point(_t); }

		Vec3 origin, dir;
	};

	//----------------------------------------------------------------------------//
	// Plane
	//----------------------------------------------------------------------------//

	struct Plane
	{
		Plane(void) : normal(Vec3::ZERO), dist(0) { }
		Plane(const Vec3& _n, float _d) : normal(_n), dist(_d) { }
		Plane Copy(void) const { return *this; }
		Plane& Set(float _nx, float _ny, float _nz, float _d) { normal.Set(_nx, _ny, _nz); dist = _d; return *this; }
		Plane& Set(const Vec3& _normal, float _distance) { normal = _normal; dist = _distance; return *this; }
		Plane& Normalize(void)
		{
			float _length = normal.LengthSq();
			if (_length > EPSILON2) _length = 1 / _length, normal *= _length, dist *= _length;
			return *this;
		}
		float Distance(const Vec3& _point) const { return normal.Dot(_point) + dist; }
		float Distance(const Vec3& _center, float _radius) const
		{
			float _d = normal.Dot(_center) + dist;
			float _md = normal.AbsDot(_radius);
			return (_d < -_md ? _d + _md : (_d > _md ? _d - _md : 0));
		}
		static Plane FromPoint(const Vec3& _normal, const Vec3& _point) { return Plane(_normal, -_normal.Dot(_point)); }

		Vec3 normal;
		float dist;

		static const Plane ZERO;
	};

	//----------------------------------------------------------------------------//
	// Sphere
	//----------------------------------------------------------------------------//

	struct Sphere
	{
		Sphere(void) : center(Vec3::ZERO), radius(0) { }
		Sphere(const Vec3& _center, float _radius) : center(_center), radius(_radius) { }

		bool Contains(const Vec3& _pt) const { return center.DistanceSq(_pt) <= Sqr(radius); }
		bool Intersects(const Sphere& _bv) const { return center.DistanceSq(_bv.center) <= Sqr(radius + _bv.radius); }

		Vec3 center;
		float radius;
	};

	//----------------------------------------------------------------------------//
	// AlignedBox
	//----------------------------------------------------------------------------//

	enum BoxCorner : uint
	{
		BC_LEFT_BOTTOM_FAR = 0,// min
		BC_RIGHT_BOTTOM_FAR,
		BC_RIGHT_BOTTOM_NEAR,
		BC_LEFT_BOTTOM_NEAR,
		BC_RIGHT_TOP_NEAR, // max
		BC_LEFT_TOP_NEAR,
		BC_LEFT_TOP_FAR,
		BC_RIGHT_TOP_FAR,
	};

	struct AlignedBox
	{
		AlignedBox(void) : mn(999999.9f), mx(-999999.9f) { }
		AlignedBox(const Vec3& _min, const Vec3& _max) : mn(_min), mx(_max) { }
		AlignedBox Copy(void) const { return *this; }
		AlignedBox& Set(const AlignedBox& _b) { return *this = _b; }
		AlignedBox& Set(const Vec3& _min, const Vec3& _max) { mn.Set(_min), mx.Set(_max); return *this; }
		AlignedBox& SetMinMax(const Vec3& _a, const Vec3& _b) { mn.SetMin(_a, _b), mx.SetMax(_a, _b); return *this; }
		AlignedBox& SetCenterExtends(const Vec3& _center, const Vec3& _extends) { return Set(_center - _extends, _center + _extends); }
		AlignedBox& SetZero(void) { mn = Vec3::ZERO, mx = Vec3::ZERO; return *this; }
		AlignedBox& Reset(const Vec3& _pt) { return Set(_pt, _pt); }
		AlignedBox& Reset(void) { mn = 999999.9f, mx = -999999.9f; return *this; }
		AlignedBox& AddPoint(const Vec3& _pt) { mn.SetMin(_pt); mx.SetMax(_pt); return *this; }
		AlignedBox& AddVertices(const void* _data, uint _count, uint _stride = 0, uint _offset = 0)
		{
			if (_data && _count > 0)
			{
				union { const uint8* p; const Vec3* v; } _vertices = { ((const uint8_t*)_data) + _offset };
				for (uint i = 0, _step = _stride ? _stride : sizeof(Vec3); i < _count; ++i)
				{
					mn.SetMin(*_vertices.v);
					mx.SetMax(*_vertices.v);
					_vertices.p += _step;
				}
			}
			return *this;
		}


		bool IsZero(void) const { return mn == mx && mn == Vec3::ZERO; }
		bool IsFinite(void) const { return mn.x <= mx.x && mn.y <= mx.y && mn.z <= mx.z; }
		Vec3 Size(void) const { return mx - mn; }
		Vec3 Extends(void) const { return (mx - mn) * 0.5f; }
		Vec3 Center(void) const { return (mx + mn) * 0.5f; }
		ge::Sphere Sphere(void) const { return ge::Sphere(Center(), Radius()); }
		float Diagonal(void) const { return (mx - mn).Length(); }
		float DiagonalSq(void) const { return (mx - mn).LengthSq(); }
		float Radius(void) const { return Diagonal() * 0.5f; }
		float Width(void) const { return mx.x - mn.x; }
		float Height(void) const { return mx.y - mn.y; }
		float Depth(void) const { return mx.z - mn.z; }
		float Volume(void) const { return (mx - mn).LengthSq(); }
		void GetAllCorners(const void* _data, uint _stride = 0, uint _offset = 0) const
		{
			if (_data)
			{
				union { uint8* p; Vec3* v; } _vertices = { ((uint8_t*)_data) + _offset };
				uint _step = _stride ? _stride : sizeof(Vec3);
				*_vertices.v = mn; _vertices.p += _step;
				*_vertices.v = Vec3(mx.x, mn.y, mn.z); _vertices.p += _step;
				*_vertices.v = Vec3(mx.x, mn.y, mx.z); _vertices.p += _step;
				*_vertices.v = Vec3(mn.x, mn.y, mx.z); _vertices.p += _step;
				*_vertices.v = mx; _vertices.p += _step;
				*_vertices.v = Vec3(mn.x, mx.y, mx.z); _vertices.p += _step;
				*_vertices.v = Vec3(mn.x, mx.y, mn.z); _vertices.p += _step;
				*_vertices.v = Vec3(mx.x, mx.y, mn.z);
			}
		}


		AlignedBox operator + (const Vec3& _point) const { return Copy().AddPoint(_point); }
		AlignedBox operator + (const AlignedBox& _box) const { return Copy().AddPoint(_box.mn).AddPoint(_box.mx); }
		AlignedBox& operator += (const Vec3& _point) { return AddPoint(_point); }
		AlignedBox& operator += (const AlignedBox& _box) { return AddPoint(_box.mn).AddPoint(_box.mx); }
		AlignedBox operator * (const Mat34& _rhs) const { return AlignedBox().SetCenterExtends(Center() * _rhs, _rhs.TransformVectorAbs(Extends())); }
		AlignedBox operator * (const Mat44& _rhs) const
		{
			AlignedBox _r;
			Vec3 _c;
			_c = mn;   _r.Reset(_c * _rhs); // min, min, min
			_c.z = mx.z; _r.AddPoint(_c * _rhs); // min, min, max
			_c.y = mx.y; _r.AddPoint(_c * _rhs); // min, max, max
			_c.z = mn.z; _r.AddPoint(_c * _rhs); // min, max, min
			_c.x = mx.x; _r.AddPoint(_c * _rhs); // max, max, min
			_c.z = mx.z; _r.AddPoint(_c * _rhs); // max, max, max
			_c.y = mn.y; _r.AddPoint(_c * _rhs); // max, min, max
			_c.z = mn.z; _r.AddPoint(_c * _rhs); // max, min, min
			return _r;
		}

		bool Contains(const Vec3& _point) const
		{
			return !(_point.x < mn.x || _point.x > mx.x || _point.y < mn.y || _point.y > mx.y || _point.z < mn.z || _point.z > mx.z);
		}
		bool Contains(const AlignedBox& _box) const
		{
			return (mx.x >= _box.mx.x && mx.y >= _box.mx.y && mx.z >= _box.mx.z && mn.x <= _box.mn.x && mn.y <= _box.mn.y && mn.z <= _box.mn.z);
		}
		bool Intersects(const AlignedBox& _box, bool* _contains) const
		{
			if (mx.x < _box.mn.x || mx.y < _box.mn.y || mx.z < _box.mn.z || mn.x > _box.mx.x || mn.y > _box.mx.y || mn.z > _box.mx.z) return false;
			if (_contains) *_contains = (mx.x >= _box.mx.x && mx.y >= _box.mx.y && mx.z >= _box.mx.z && mn.x <= _box.mn.x && mn.y <= _box.mn.y && mn.z <= _box.mn.z);
			return true;
		}
		bool Intersects(const AlignedBox& _box) const
		{
			return !(IsZero() || _box.IsZero() || mx.x < _box.mn.x || mx.y < _box.mn.y || mx.z < _box.mn.z || mn.x > _box.mx.x || mn.y > _box.mx.y || mn.z > _box.mx.z);
		}
		bool Intersects(const Ray& _ray, float* _distance = 0, Vec3* _point = 0) const
		{
			if (IsZero()) return false;
			static const uint _axes[3][3] = { { 0, 1, 2 }, { 1, 0, 2 }, { 2, 0, 1 } };
			float _d = 0;
			bool _hit = false;
			if (_ray.origin.x >= mn.x && _ray.origin.y >= mn.y && _ray.origin.z >= mn.z &&
				_ray.origin.x <= mx.x && _ray.origin.y <= mx.y && _ray.origin.z <= mx.z && !_distance && !_point)
			{
				_hit = true;
			}
			else if (IsFinite())
			{
				float _t, _low_t = 0;
				Vec3 _hit_point, _min = mn, _max = mx, _origin = _ray.origin, _dir = _ray.dir;
				for (uint i = 0, x, y, z; i < 3 && (!_hit || _distance || _point); ++i)
				{
					x = _axes[i][0], y = _axes[i][1], z = _axes[i][2];
					if (_origin[x] <= _min[x] && _dir[x] > 0)
					{
						_t = (_min[x] - _origin[x]) / _dir[x];
						if (_t >= 0)
						{
							_hit_point = _origin + _dir * _t;
							if ((!_hit || _t < _low_t) && _hit_point[y] >= _min[y] && _hit_point[y] <= _max[y] && _hit_point[z] >= _min[z] && _hit_point[z] <= _max[z]) _hit = true, _low_t = _t;
						}
					}
					if (_origin[x] >= _max[x] && _dir[x] < 0)
					{
						_t = (_max[x] - _origin[x]) / _dir[x];
						if (_t >= 0)
						{
							_hit_point = _origin + _dir * _t;
							if ((!_hit || _t < _low_t) && _hit_point[y] >= _min[y] && _hit_point[y] <= _max[y] && _hit_point[z] >= _min[z] && _hit_point[z] <= _max[z]) _hit = true, _low_t = _t;
						}
					}
				}
				_d = _low_t;
			}
			if (_distance) *_distance = _d;
			if (_point) *_point = _ray.Point(_d);
			return _hit;
		}

		//static AlignedBox FromCenterExtends(const Vec3& _center, const Vec3& _extends) { return AlignedBox().SetCenterExtends(_center, _extends); }

		static AlignedBox FromViewProjMatrix(const Mat44& _m)
		{
			AlignedBox _r;
			_r.Reset(Vec3(-1, +1, +1) * _m);
			_r.AddPoint(Vec3(+1, +1, +1) * _m);
			_r.AddPoint(Vec3(-1, -1, +1) * _m);
			_r.AddPoint(Vec3(+1, -1, +1) * _m);
			_r.AddPoint(Vec3(-1, +1, -1) * _m);
			_r.AddPoint(Vec3(+1, +1, -1) * _m);
			_r.AddPoint(Vec3(-1, -1, -1) * _m);
			_r.AddPoint(Vec3(+1, -1, -1) * _m);
			return _r;
		}

		Vec3 mn, mx;

		static const AlignedBox ZERO;
		static const AlignedBox INF;
		static const uint16 LINES[24];
		static const uint16 QUADS[24];
		static const uint16 TRIANGLES[36];
	};

	//----------------------------------------------------------------------------//
	// Frustum
	//----------------------------------------------------------------------------//

	enum FrustumPlane : uint
	{
		FP_LEFT = 0,
		FP_RIGHT,
		FP_BOTTOM,
		FP_TOP,
		FP_NEAR,
		FP_FAR,
	};

	struct Frustum
	{
		Frustum(void) { }

		Frustum Copy(void) { return *this; }
		Frustum& Set(const Frustum& _frustum) { return *this = _frustum; }
		Frustum& SetCameraMatrices(const Mat34& _view, const Mat44& _proj)
		{
			Mat44 _m = _proj * _view; // _view_proj
			planes[FP_RIGHT].Set(_m(3, 0) - _m(0, 0), _m(3, 1) - _m(0, 1), _m(3, 2) - _m(0, 2), _m(3, 3) - _m(0, 3)).Normalize();
			planes[FP_LEFT].Set(_m(3, 0) + _m(0, 0), _m(3, 1) + _m(0, 1), _m(3, 2) + _m(0, 2), _m(3, 3) + _m(0, 3)).Normalize();
			planes[FP_TOP].Set(_m(3, 0) - _m(1, 0), _m(3, 1) - _m(1, 1), _m(3, 2) - _m(1, 2), _m(3, 3) - _m(1, 3)).Normalize();
			planes[FP_BOTTOM].Set(_m(3, 0) + _m(1, 0), _m(3, 1) + _m(1, 1), _m(3, 2) + _m(1, 2), _m(3, 3) + _m(1, 3)).Normalize();
			planes[FP_NEAR].Set(_m(3, 0) + _m(2, 0), _m(3, 1) + _m(2, 1), _m(3, 2) + _m(2, 2), _m(3, 3) + _m(2, 3)).Normalize();
			planes[FP_FAR].Set(_m(3, 0) - _m(2, 0), _m(3, 1) - _m(2, 1), _m(3, 2) - _m(2, 2), _m(3, 3) - _m(2, 3)).Normalize();
			origin = Vec3::ZERO * _view.Copy().Inverse();
			_m.Inverse(); // _inv_view_proj
			corners[BC_LEFT_BOTTOM_FAR] = Vec3(-1, -1, -1) * _m;
			corners[BC_RIGHT_BOTTOM_FAR] = Vec3(+1, -1, -1) * _m;
			corners[BC_RIGHT_BOTTOM_NEAR] = Vec3(+1, -1, +1) * _m;
			corners[BC_LEFT_BOTTOM_NEAR] = Vec3(-1, -1, +1) * _m;
			corners[BC_RIGHT_TOP_NEAR] = Vec3(+1, +1, +1) * _m;
			corners[BC_LEFT_TOP_NEAR] = Vec3(-1, +1, +1) * _m;
			corners[BC_LEFT_TOP_FAR] = Vec3(-1, +1, -1) * _m;
			corners[BC_RIGHT_TOP_FAR] = Vec3(+1, +1, -1) * _m;
			box.Reset().AddVertices(corners, 8);
			return *this;
		}

		bool Intersects(const Vec3& _point) const
		{
			return planes[FP_NEAR].Distance(_point) >= 0 &&
				planes[FP_FAR].Distance(_point) >= 0 &&
				planes[FP_LEFT].Distance(_point) >= 0 &&
				planes[FP_RIGHT].Distance(_point) >= 0 &&
				planes[FP_TOP].Distance(_point) >= 0 &&
				planes[FP_BOTTOM].Distance(_point) >= 0;
		}
		bool Intersects(const Vec3& _center, float _radius) const
		{
			return planes[FP_NEAR].Distance(_center, _radius) >= 0 &&
				planes[FP_FAR].Distance(_center, _radius) >= 0 &&
				planes[FP_LEFT].Distance(_center, _radius) >= 0 &&
				planes[FP_RIGHT].Distance(_center, _radius) >= 0 &&
				planes[FP_TOP].Distance(_center, _radius) >= 0 &&
				planes[FP_BOTTOM].Distance(_center, _radius) >= 0;
		}
		bool Intersects(const AlignedBox& _box, bool* _contains = 0) const
		{
			if (box.Intersects(_box) && Intersects(_box.Center(), _box.Radius()))
			{
				if (_contains)
				{
					*_contains = true;
					Vec3 _corners[8];
					_box.GetAllCorners(_corners);
					for (uint i = 0; i < 8; ++i) if (!Intersects(_corners[i]))
					{
						*_contains = false;
						break;
					}
				}
				return true;
			}
			return false;
		}
		bool Intersects(const Frustum& _frustum, bool* _contains = 0) const
		{
			if (box.Intersects(_frustum.box))
			{
				for (uint i = 0; i < 6; ++i)
				{
					int _out = 0;
					for (uint j = 0; j < 8; ++j) if (planes[i].Distance(_frustum.corners[j]) < 0) ++_out;
					if (_out == 8) return false;
				}
				if (_contains)
				{
					*_contains = Intersects(_frustum.corners[0]) &&
						Intersects(_frustum.corners[1]) &&
						Intersects(_frustum.corners[2]) &&
						Intersects(_frustum.corners[3]) &&
						Intersects(_frustum.corners[4]) &&
						Intersects(_frustum.corners[5]) &&
						Intersects(_frustum.corners[6]) &&
						Intersects(_frustum.corners[7]);
				}
				return true;
			}
			return false;
		}

		float Distance(const Vec3& _point) const { return origin.Distance(_point); }
		float Distance(const Vec3& _center, float _radius) const { float _d = origin.Distance(_center); return _d < _radius ? 0 : _d - _radius; }
		float Distance(const AlignedBox& _box) const { return Distance(_box.Center(), _box.Radius()); }

		static Frustum& FromCameraMatrices(const Mat34& _view, const Mat44& _proj) { return Frustum().SetCameraMatrices(_view, _proj); }

		static const uint16 LINES[24];
		static const uint16 QUADS[24];

		Plane planes[6];
		Vec3 corners[8];
		Vec3 origin;
		AlignedBox box;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
