#pragma once

#include "Common.hpp"
#include "String.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#ifdef _FAST_HALF_FLOAT
#	define FAST_HALF_FLOAT
#endif

	typedef float float32;
	typedef struct half float16;
	typedef double float64;

	static const float Epsilon = 1e-6f;
	static const float Epsilon2 = 1e-12f;
	static const float Pi = 3.1415926535897932384626433832795f;
	static const float Rad2Deg = 57.295779513082320876798154814105f;
	static const float Deg2Rad = 0.01745329251994329576923690768489f;

	//----------------------------------------------------------------------------//
	// Math funcs
	//----------------------------------------------------------------------------//

	template <typename T> const T& Min(const T& _a, const T& _b) { return _a < _b ? _a : _b; }
	template <typename T> const T& Min(const T& _a, const T& _b, const T& _c) { return _a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); }
	template <typename T> const T& Max(const T& _a, const T& _b) { return _a > _b ? _a : _b; }
	template <typename T> const T& Max(const T& _a, const T& _b, const T& _c) { return _a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); }
	template <typename T> const T Clamp(T _x, T _l, T _u) { return _x > _l ? (_x < _u ? _x : _u) : _l; }
	template <typename T> T Mix(const T& _a, const T& _b, float _t) { return _a + (_b - _a) * _t; }
	template <typename T> T Abs(T _x) { return abs(_x); }
	template <typename T> T Radians(T _val) { return _val * Deg2Rad; }
	template <typename T> T Degrees(T _val) { return _val * Rad2Deg; }
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

	//----------------------------------------------------------------------------//
	// Bitwise
	//----------------------------------------------------------------------------//

	inline uint32 FirstPow2(uint32 _val)
	{
		--_val |= _val >> 16;
		_val |= _val >> 8;
		_val |= _val >> 4;
		_val |= _val >> 2;
		_val |= _val >> 1;
		return ++_val;
	}
	inline bool IsPow2(uint32 _val) { return (_val & (_val - 1)) == 0; }
	inline uint8 FloatToByte(float _value) { return (uint8)(_value * 0xff); }
	inline float ByteToFloat(uint8 _value) { return (float)(_value * (1 / 255.0f)); }
	inline uint16 FloatToHalf(float _value)
	{
		union { float f; uint32 i; }_fb = { _value };
#	ifdef FAST_HALF_FLOAT
		return (uint16)((_fb.i >> 16) & 0x8000) | ((((_fb.i & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((_fb.i >> 13) & 0x03ff);
#	else
		uint32 _s = (_fb.i >> 16) & 0x00008000; // sign
		int32 _e = ((_fb.i >> 23) & 0x000000ff) - 0x00000070; // exponent
		uint32 _r = _fb.i & 0x007fffff; // mantissa
		if (_e < 1)
		{
			if (_e < -10)
				return 0;
			_r = (_r | 0x00800000) >> (14 - _e);
			return (uint16)(_s | _r);
		}
		else if (_e == 0x00000071)
		{
			if (_r == 0)
				return (uint16)(_s | 0x7c00); // Inf
			else
				return (uint16)(((_s | 0x7c00) | (_r >>= 13)) | (_r == 0)); // NaN
		}
		if (_e > 30)
			return (uint16)(_s | 0x7c00); // Overflow
		return (uint16)((_s | (_e << 10)) | (_r >> 13));
#	endif
	}
	inline float HalfToFloat(uint16 _value)
	{
		union { uint32 i; float f; }_fb;
#	ifdef FAST_HALF_FLOAT
		_fb.i = ((_value & 0x8000) << 16) | (((_value & 0x7c00) + 0x1C000) << 13) | ((_value & 0x03FF) << 13);
#	else
		register int32 _s = (_value >> 15) & 0x00000001; // sign
		register int32 _e = (_value >> 10) & 0x0000001f; // exponent
		register int32 _r = _value & 0x000003ff; // mantissa
		if (_e == 0)
		{
			if (_r == 0) // Plus or minus zero
			{
				_fb.i = _s << 31;
				return _fb.f;
			}
			else // Denormalized number -- renormalize it
			{
				for (; !(_r & 0x00000400); _r <<= 1, _e -= 1);
				_e += 1;
				_r &= ~0x00000400;
			}
		}
		else if (_e == 31)
		{
			if (_r == 0) // Inf
			{
				_fb.i = (_s << 31) | 0x7f800000;
				return _fb.f;
			}
			else // NaN
			{
				_fb.i = ((_s << 31) | 0x7f800000) | (_r << 13);
				return _fb.f;
			}
		}
		_e = (_e + 112) << 23;
		_r = _r << 13;
		_fb.i = ((_s << 31) | _e) | _r;
#	endif
		return _fb.f;
	}
	inline float FixedToFloat(uint32 _value, uint32 _bits, float _default = 0)
	{
		if (_bits > 31)
			_bits = 31;
		return _bits ? ((float)_value) / ((float)((1u << _bits) - 1u)) : _default;
	}
	inline uint32 FloatToFixed(float _value, uint32 _bits)
	{
		if (_bits > 31)
			_bits = 31;
		if (_value <= 0)
			return 0;
		if (_value >= 1)
			return (1u << _bits) - 1u;
		return (uint32)(_value * (float)(1u << _bits));
	}
	inline uint32 FixedToFixed(uint32 _value, uint32 _from, uint32 _to)
	{
		if (_from > 31)
			_from = 31;
		if (_to > 31)
			_to = 31;
		if (_from > _to)
			_value >>= _from - _to;
		else if (_from < _to && _value != 0)
		{
			uint32 _max = (1u << _from) - 1u;
			if (_value == _max) _value = (1u << _to) - 1u;
			else if (_max > 0) _value *= (1u << _to) / _max;
			else _value = 0;
		}
		return _value;
	}

	//----------------------------------------------------------------------------//
	// CheckSum
	//----------------------------------------------------------------------------//

	ENGINE_API uint32 Crc32(uint32 _crc, const void* _buf, uint _size);
	inline uint32 Crc32(const void* _buf, uint _size) { return Crc32(0, _buf, _size); }
	inline uint32 Crc32(const char* _str, int _length = -1, uint32 _crc = 0) { return _str ? Crc32(_crc, _str, _length < 0 ? (uint)strlen(_str) : _length) : _crc; }
	inline uint32 Crc32(const String& _str, uint32 _crc = 0) { return Crc32(_crc, _str, _str.Length()); }
	template <typename T> uint32 Crc32(const T& _obj, uint32 _crc = 0) { return Crc32(_crc, &_obj, sizeof(_obj)); }

	//----------------------------------------------------------------------------//
	// half
	//----------------------------------------------------------------------------//

	struct half
	{
		half(void) { }
		half(float _value) { Pack(_value); }
		half& operator = (float _value) { return Pack(_value); }
		operator const float(void) const { return Unpack(); }
		half operator ++ (int) { half _tmp(*this); Pack(Unpack() + 1); return _tmp; }
		half& operator ++ (void) { return Pack(Unpack() + 1); }
		half operator -- (int) { half _tmp(*this); Pack(Unpack() - 1); return _tmp; }
		half& operator -- (void) { return Pack(Unpack() - 1); }
		half& operator += (float _rhs) { return Pack(Unpack() + _rhs); }
		half& operator -= (float _rhs) { return Pack(Unpack() - _rhs); }
		half& operator *= (float _rhs) { return Pack(Unpack() * _rhs); }
		half& operator /= (float _rhs) { return Pack(Unpack() / _rhs); }
		
		half& Pack(float _value) { value = FloatToHalf(_value); return *this; }
		const float Unpack(void) const { return HalfToFloat(value); }

		uint16 value;
	};

	//----------------------------------------------------------------------------//
	// Vec2T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec2T
	{
		Vec2T(void) { }
		Vec2T(T _xy) : x(_xy), y(_xy) { }
		Vec2T(T _x, T _y) : x(_x), y(_y) { }
		Vec2T Copy(void) const { return *this; }
		template <typename X> Vec2T(const Vec2T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) { }
		template <typename X> explicit Vec2T(X _x, X _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }
		template <typename X> X Cast(void) const { return X(x, y); }

		const T operator [] (uint _index) const { return (&x)[_index]; }
		T& operator [] (uint _index) { return (&x)[_index]; }
		const T* operator * (void) const { return &x; }
		T* operator * (void) { return &x; }

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

		T x, y;

		static const Vec2T Zero;
		static const Vec2T One;
		static const Vec2T UnitX;
		static const Vec2T UnitY;
	};

	template <typename T> const Vec2T<T> Vec2T<T>::Zero(0);
	template <typename T> const Vec2T<T> Vec2T<T>::One(1);
	template <typename T> const Vec2T<T> Vec2T<T>::UnitX(1, 0);
	template <typename T> const Vec2T<T> Vec2T<T>::UnitY(0, 1);

	typedef Vec2T<float> Vec2;
	typedef Vec2T<half> Vec2h;
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
		template <typename X> Vec3T(const Vec3T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)) { }
		template <typename X> explicit Vec3T(X _x, X _y, X _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) { }
		Vec3T Copy(void) const { return *this; }
		template <typename X> X Cast(void) const { return X(x, y, z); }

		const T operator [] (uint _index) const { return (&x)[_index]; }
		T& operator [] (uint _index) { return (&x)[_index]; }
		const T* operator * (void) const { return &x; }
		T* operator * (void) { return &x; }

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

		// [for Vec3T<float>]

		Vec3T<float>& SetMin(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Min(_a.x, _b.x), y = Min(_a.y, _b.y), z = Min(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMin(const Vec3T<float>& _other) { return SetMin(*this, _other); }
		Vec3T<float>& SetMax(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Max(_a.x, _b.x), y = Max(_a.y, _b.y), z = Max(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMax(const Vec3T<float>& _other) { return SetMax(*this, _other); }
		Vec3T<float>& Normalize(void)
		{
			float _l = LengthSq();
			if (_l > Epsilon2)
				*this *= RSqrt(_l);
			return *this;
		}
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
		Vec3T<float> Reflect(const Vec3T<float>& _n) const { return Vec3T<float>(*this - (2 * Dot(_n) * _n)); }
		Vec3T<float> Perpendicular(void) const
		{
			Vec3T<float> _perp = Cross(UNIT_X);
			if (_perp.LengthSq() <= EPSILON2)
				_perp = Cross(UNIT_Y);
			return _perp.Normalize();
		}
		float Angle(const Vec3T<float>& _v) const
		{
			float _lp = LengthSq() * _v.LengthSq();
			if (_lp > EPSILON2)
				_lp = RSqrt(_lp);
			return ACos(Clamp<float>(Dot(_v) * _lp, -1, 1));
		}

		T x, y, z;

		static const Vec3T Zero;
		static const Vec3T One;
		static const Vec3T UnitX;
		static const Vec3T UnitY;
		static const Vec3T UnitZ;
	};

	template <typename T> const Vec3T<T> Vec3T<T>::Zero(0);
	template <typename T> const Vec3T<T> Vec3T<T>::One(1);
	template <typename T> const Vec3T<T> Vec3T<T>::UnitX(1, 0, 0);
	template <typename T> const Vec3T<T> Vec3T<T>::UnitY(0, 1, 0);
	template <typename T> const Vec3T<T> Vec3T<T>::UnitZ(0, 0, 1);

	typedef Vec3T<float> Vec3;
	typedef Vec3T<half> Vec3h;
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
		template <typename X> Vec4T(const Vec4T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)), w(static_cast<T>(_v.w)) { }
		template <typename X> explicit Vec4T(X _x, X _y, X _z, X _w) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)), w(static_cast<T>(_w)) { }
		Vec4T Copy(void) const { return *this; }
		template <typename X> X Cast(void) const { return X(x, y, z, w); }

		const T operator [] (uint _index) const { return (&x)[_index]; }
		T& operator [] (uint _index) { return (&x)[_index]; }
		const T* operator * (void) const { return &x; }
		T* operator * (void) { return &x; }

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

		T x, y, z, w;

		static const Vec4T Zero;
		static const Vec4T One;
		static const Vec4T Identity;
	};

	template <typename T> const Vec4T<T> Vec4T<T>::Zero(0);
	template <typename T> const Vec4T<T> Vec4T<T>::One(1);
	template <typename T> const Vec4T<T> Vec4T<T>::Identity(0, 0, 0, 1);

	typedef Vec4T<float> Vec4;
	typedef Vec4T<half> Vec4h;
	typedef Vec4T<int> Vec4i;
	typedef Vec4T<int8> Vec4b;
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
		Quat operator * (const Quat& _rhs) const 
		{ 
			return Quat(w * _rhs.x + x * _rhs.w + y * _rhs.z - z * _rhs.y,
				w * _rhs.y + y * _rhs.w + z * _rhs.x - x * _rhs.z,
				w * _rhs.z + z * _rhs.w + x * _rhs.y - y * _rhs.x,
				w * _rhs.w - x * _rhs.x - y * _rhs.y - z * _rhs.z);
		}
		Quat operator * (float _rhs) const { return Quat(x * _rhs, y * _rhs, z * _rhs, w * _rhs); }
		Quat operator / (float _rhs) const { return Quat(x / _rhs, y / _rhs, z / _rhs, w / _rhs); }
		Quat& operator += (const Quat& _rhs) { x += _rhs.x, y += _rhs.y, z += _rhs.z, w += _rhs.w; return *this; }
		Quat& operator -= (const Quat& _rhs) { x -= _rhs.x, y -= _rhs.y, z -= _rhs.z, w -= _rhs.w; return *this; }
		Quat& operator *= (const Quat& _rhs) { return *this = *this * _rhs; }
		Quat& operator *= (float _rhs) { x += _rhs, y += _rhs, z += _rhs, w += _rhs; return *this; }
		Quat& operator /= (float _rhs) { x += _rhs, y += _rhs, z += _rhs, w += _rhs; return *this; }
		friend Quat operator * (float _lhs, const Quat& _rhs) { return Quat(_lhs * _rhs.x, _lhs * _rhs.y, _lhs * _rhs.z, _lhs * _rhs.w); }
		friend Vec3 operator * (const Vec3& _lhs, const Quat& _rhs)
		{
			const Vec3& _q = *(Vec3*)_rhs.v;
			Vec3 _uv(_q.Cross(_lhs));
			return _lhs + _uv * 2 * _rhs.w + _q.Cross(_uv) * 2;
		}

		Quat& Set(float _x, float _y, float _z, float _w) { x = _x, y = _y, z = _z, w = _w; return *this; }
		Quat& Set(const Quat& _other) { return *this = _other; }

		float Dot(const Quat& _q) const
		{
			return x * _q.x + y * _q.y + z * _q.z + w * _q.w;
		}
		Quat& Normalize(void)
		{
			float _l = x * x + y * y + z * z + w * w;
			if (_l > Epsilon2)
				*this *= RSqrt(_l);
			return *this;
		}
		Quat& Inverse(void)
		{
			float _d = x * x + y * y + z * z + w * w;
			if (_d > 0)
				x *= -_d, y *= -_d, z *= -_d, w *= _d;
			else
				x = 0, y = 0, z = 0, w = 1;
			return *this;
		}
		Quat& UnitInverse(void)
		{
			x = -x, y = -y, z = -z;
			return *this;
		}
		Quat Nlerp(const Quat& _q, float _t, bool _shortestPath = false) const
		{
			const Quat& _p = *this;
			float _c = _p.Dot(_q);
			Quat _result;
			if (_c < 0 && _shortestPath)
				_result = _p + _t * ((-_q) - _p);
			else
				_result = _p + _t * (_q - _p);
			return _result.Normalize();
		}
		Quat Slerp(const Quat& _q, float _t, bool _shortestPath = false) const
		{
			const Quat& _p = *this;
			float _c = _p.Dot(_q);
			Quat _tmp;
			if (_c < 0 && _shortestPath)
				_c = -_c, _tmp = -_q;
			else
				_tmp = _q;
			if (Abs(_c) < 1 - Epsilon)
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
			_r1[0] = _xy - _wz, _r1[1] = 1 - (_xx + _zz), _r1[2] = _yz + _wx;
			_r2[0] = _xz + _wy, _r2[1] = _yz - _wx, _r2[2] = 1 - (_xx + _yy);
		}
		Quat& FromMatrixRows(const float* _r0, const float* _r1, const float* _r2)
		{
			float _invr, _root = _r0[0] + _r1[1] + _r2[2];
			if (_root > 0)
			{
				_root = Sqrt(_root + 1);
				_invr = 0.5f / _root;
				return Set((_r1[2] - _r2[1]) * _invr, (_r2[0] - _r0[2]) * _invr, (_r0[1] - _r1[0]) * _invr, _root * 0.5f);
			}
			if (_r0[0] > _r1[1] && _r0[0] > _r2[2])
			{
				_root = Sqrt(_r0[0] - _r1[1] - _r2[2] + 1);
				_invr = 0.5f / _root;
				return Set(_root * 0.5f, (_r0[1] + _r1[0]) * _invr, (_r0[2] + _r2[0]) * _invr, (_r1[2] - _r2[1]) * _invr);
			}
			else if (_r1[1] > _r0[0] && _r1[1] > _r2[2])
			{
				_root = Sqrt(_r1[1] - _r2[2] - _r0[0] + 1);
				_invr = 0.5f / _root;
				return Set((_r1[0] + _r0[1]) * _invr, _root * 0.5f, (_r1[2] + _r2[1]) * _invr, (_r2[0] - _r0[2]) * _invr);
			}
			_root = Sqrt(_r2[2] - _r0[0] - _r1[1] + 1);
			_invr = 0.5f / _root;
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

		ENGINE_API static const Quat Zero;
		ENGINE_API static const Quat Identity;
	};

	inline Quat Mix(const Quat& _a, const Quat& _b, float _t)
	{
		return _a.Slerp(_b, _t);
	}

	//----------------------------------------------------------------------------//
	// Mat34
	//----------------------------------------------------------------------------//

	struct Mat34
	{
		Mat34(void) { }
		Mat34(float _val) { *this = Zero; m[0][0] = _val; m[1][1] = _val; m[2][2] = _val; }
		Mat34(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23) :
			m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23) { }
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
		Mat34 operator * (float _rhs) const{ return Copy().Multiply(_rhs); }
		friend Vec3 operator * (const Vec3& _lhs, const Mat34& _rhs) { return _rhs.Transform(_lhs); } // Vec3 = Vec3 * Mat34
		Mat34& operator += (const Mat34& _rhs) { return Add(_rhs); }
		Mat34& operator *= (const Mat34& _rhs) { return Multiply(_rhs); }
		Mat34& operator *= (float _rhs) { return Multiply(_rhs); }

		Mat34& Add(const Mat34& _rhs)
		{
			for (int i = 0; i < 12; ++i) v[i] += v[i];
			return *this;
		}
		Mat34& Multiply(float _rhs)
		{
			for (int i = 0; i < 12; ++i)
				v[i] *= _rhs;
			return *this;
		}
		Mat34& Multiply(const Mat34& _rhs)
		{
			return *this = Mat34(m00 * _rhs.m00 + m01 * _rhs.m10 + m02 * _rhs.m20,
				m00 * _rhs.m01 + m01 * _rhs.m11 + m02 * _rhs.m21,
				m00 * _rhs.m02 + m01 * _rhs.m12 + m02 * _rhs.m22,
				m00 * _rhs.m03 + m01 * _rhs.m13 + m02 * _rhs.m23 + m03,
				m10 * _rhs.m00 + m11 * _rhs.m10 + m12 * _rhs.m20,
				m10 * _rhs.m01 + m11 * _rhs.m11 + m12 * _rhs.m21,
				m10 * _rhs.m02 + m11 * _rhs.m12 + m12 * _rhs.m22,
				m10 * _rhs.m03 + m11 * _rhs.m13 + m12 * _rhs.m23 + m13,
				m20 * _rhs.m00 + m21 * _rhs.m10 + m22 * _rhs.m20,
				m20 * _rhs.m01 + m21 * _rhs.m11 + m22 * _rhs.m21,
				m20 * _rhs.m02 + m21 * _rhs.m12 + m22 * _rhs.m22,
				m20 * _rhs.m03 + m21 * _rhs.m13 + m22 * _rhs.m23 + m23);
		}
		float Determinant(void) const
		{
			return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 - m02 * m11 * m20 - m00 * m12 * m21 - m01 * m10 * m22;
		}
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

		Vec3 Translate(const Vec3& _v) const
		{
			return Vec3(_v.x + m03, _v.y + m13, _v.z + m23);
		}
		Vec3 Transform(const Vec3& _v) const
		{
			return Vec3(
				m00 * _v.x + m01 * _v.y + m02 * _v.z + m03,
				m10 * _v.x + m11 * _v.y + m12 * _v.z + m13,
				m20 * _v.x + m21 * _v.y + m22 * _v.z + m23);
		}
		Vec3 TransformVectorAbs(const Vec3& _v) const
		{
			return Vec3(
				Abs(m00) * _v.x + Abs(m01) * _v.y + Abs(m02) * _v.z,
				Abs(m10) * _v.x + Abs(m11) * _v.y + Abs(m12) * _v.z,
				Abs(m20) * _v.x + Abs(m21) * _v.y + Abs(m22) * _v.z);
		}
		Vec3 TransformVector(const Vec3& _v) const
		{
			return Vec3(m00 * _v.x + m01 * _v.y + m02 * _v.z, m10 * _v.x + m11 * _v.y + m12 * _v.z, m20 * _v.x + m21 * _v.y + m22 * _v.z);
		}

		Mat34& SetTranslation(const Vec3& _translation)
		{
			m03 = _translation.x, m13 = _translation.y, m23 = _translation.z;
			return *this;
		}
		Vec3 GetTranslation(void) const
		{
			return Vec3(m03, m12, m23);
		}
		Mat34& CreateTranslation(const Vec3& _translation)
		{
			return (*this = Identity).SetTranslation(_translation);
		}

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
			if (_det < 0)
				_q0 = -_q0, _q1 = -_q1, _q2 = -_q2;
			return Quat().FromMatrixRows(*_q0, *_q1, *_q2);
		}
		Mat34& CreateRotation(const Quat& _rotation)
		{
			SetTranslation(Vec3::Zero);
			_rotation.ToMatrixRows(m[0], m[1], m[2]);
			return *this;
		}

		bool HasScale(void) const
		{
			if (!(Abs((m00 * m00 + m10 * m10 + m20 * m20) - 1) < Epsilon))
				return true;
			if (!(Abs((m01 * m01 + m11 * m11 + m21 * m21) - 1) < Epsilon))
				return true;
			if (!(Abs((m02 * m02 + m12 * m12 + m22 * m22) - 1) < Epsilon))
				return true;
			return false;
		}
		bool HasNegativeScale(void) const
		{
			return Determinant() < 0;
		}
		Mat34& SetScale(const Vec3& _scale)
		{
			Quat _rotation = GetRotation();
			_rotation.ToMatrixRows(m[0], m[1], m[2]);
			Row(0) *= _scale, Row(1) *= _scale, Row(2) *= _scale;
			return *this;
		}
		Vec3 GetScale(void) const
		{
			return Vec3(
				Sqrt(m00 * m00 + m10 * m10 + m20 * m20),
				Sqrt(m01 * m01 + m11 * m11 + m21 * m21),
				Sqrt(m02 * m02 + m12 * m12 + m22 * m22));
		}
		Mat34& CreateScale(const Vec3& _scale)
		{
			*this = Zero;
			m00 = _scale.x, m11 = _scale.y, m22 = _scale.z;
			return *this;
		}

		Mat34& CreateTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::One)
		{
			// Ordering: Scale, Rotate, Translate
			float _r0[3], _r1[3], _r2[3];
			_rotation.ToMatrixRows(_r0, _r1, _r2);
			m00 = _scale.x * _r0[0], m01 = _scale.y * _r0[1], m02 = _scale.z * _r0[2], m03 = _translation.x;
			m10 = _scale.x * _r1[0], m11 = _scale.y * _r1[1], m12 = _scale.z * _r1[2], m13 = _translation.y;
			m20 = _scale.x * _r2[0], m21 = _scale.y * _r2[1], m22 = _scale.z * _r2[2], m23 = _translation.z;
			return *this;
		}
		Mat34& CreateInverseTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::One)
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

		ENGINE_API static const Mat34 Zero;
		ENGINE_API static const Mat34 Identity;
	};

	//----------------------------------------------------------------------------//
	// Mat44
	//----------------------------------------------------------------------------//

	struct Mat44
	{
		Mat44(void) { }
		Mat44(float _val) { *this = Zero; m[0][0] = _val; m[1][1] = _val; m[2][2] = _val; m[3][3] = _val; }
		Mat44(const Mat34& _m34) : m30(0), m31(0), m32(0), m33(1) { memcpy(v, *_m34, 12 * sizeof(float)); }
		explicit Mat44(const float* _m44) { memcpy(v, _m44, 16 * sizeof(float)); }
		Mat44(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33) :
			m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23), m30(_30), m31(_31), m32(_32), m33(_33) { }
		Mat44 Copy(void) const { return *this; }

		operator const Mat34& (void) const { /*ASSERT(IsAffine());*/ return *(Mat34*)(v); }
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
			for (int i = 0; i < 16; ++i)
				v[i] += _rhs.v[i];
			return *this;
		}
		Mat44& Multiply(float _rhs)
		{
			for (int i = 0; i < 16; ++i)
				v[i] += _rhs;
			return *this;
		}
		Mat44& Multiply(const Mat44& _rhs)
		{
			return *this = Mat44(
				m00 * _rhs.m00 + m01 * _rhs.m10 + m02 * _rhs.m20 + m03 * _rhs.m30,
				m00 * _rhs.m01 + m01 * _rhs.m11 + m02 * _rhs.m21 + m03 * _rhs.m31,
				m00 * _rhs.m02 + m01 * _rhs.m12 + m02 * _rhs.m22 + m03 * _rhs.m32,
				m00 * _rhs.m03 + m01 * _rhs.m13 + m02 * _rhs.m23 + m03 * _rhs.m33,
				m10 * _rhs.m00 + m11 * _rhs.m10 + m12 * _rhs.m20 + m13 * _rhs.m30,
				m10 * _rhs.m01 + m11 * _rhs.m11 + m12 * _rhs.m21 + m13 * _rhs.m31,
				m10 * _rhs.m02 + m11 * _rhs.m12 + m12 * _rhs.m22 + m13 * _rhs.m32,
				m10 * _rhs.m03 + m11 * _rhs.m13 + m12 * _rhs.m23 + m13 * _rhs.m33,
				m20 * _rhs.m00 + m21 * _rhs.m10 + m22 * _rhs.m20 + m23 * _rhs.m30,
				m20 * _rhs.m01 + m21 * _rhs.m11 + m22 * _rhs.m21 + m23 * _rhs.m31,
				m20 * _rhs.m02 + m21 * _rhs.m12 + m22 * _rhs.m22 + m23 * _rhs.m32,
				m20 * _rhs.m03 + m21 * _rhs.m13 + m22 * _rhs.m23 + m23 * _rhs.m33,
				m30 * _rhs.m00 + m31 * _rhs.m10 + m32 * _rhs.m20 + m33 * _rhs.m30,
				m30 * _rhs.m01 + m31 * _rhs.m11 + m32 * _rhs.m21 + m33 * _rhs.m31,
				m30 * _rhs.m02 + m31 * _rhs.m12 + m32 * _rhs.m22 + m33 * _rhs.m32,
				m30 * _rhs.m03 + m31 * _rhs.m13 + m32 * _rhs.m23 + m33 * _rhs.m33);
		}
		Mat44& Multiply(const Mat34& _rhs)
		{
			return *this = Mat44(
				m00 * _rhs.m00 + m01 * _rhs.m10 + m02 * _rhs.m20,
				m00 * _rhs.m01 + m01 * _rhs.m11 + m02 * _rhs.m21,
				m00 * _rhs.m02 + m01 * _rhs.m12 + m02 * _rhs.m22,
				m00 * _rhs.m03 + m01 * _rhs.m13 + m02 * _rhs.m23 + m03,
				m10 * _rhs.m00 + m11 * _rhs.m10 + m12 * _rhs.m20,
				m10 * _rhs.m01 + m11 * _rhs.m11 + m12 * _rhs.m21,
				m10 * _rhs.m02 + m11 * _rhs.m12 + m12 * _rhs.m22,
				m10 * _rhs.m03 + m11 * _rhs.m13 + m12 * _rhs.m23 + m13,
				m20 * _rhs.m00 + m21 * _rhs.m10 + m22 * _rhs.m20,
				m20 * _rhs.m01 + m21 * _rhs.m11 + m22 * _rhs.m21,
				m20 * _rhs.m02 + m21 * _rhs.m12 + m22 * _rhs.m22,
				m20 * _rhs.m03 + m21 * _rhs.m13 + m22 * _rhs.m23 + m23,
				m30 * _rhs.m00 + m31 * _rhs.m10 + m32 * _rhs.m20,
				m30 * _rhs.m01 + m31 * _rhs.m11 + m32 * _rhs.m21,
				m30 * _rhs.m02 + m31 * _rhs.m12 + m32 * _rhs.m22,
				m30 * _rhs.m03 + m31 * _rhs.m13 + m32 * _rhs.m23 + m33);
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
			return Vec3( (m00 * _v.x + m01 * _v.y + m02 * _v.z + m03) * _iw,
				(m10 * _v.x + m11 * _v.y + m12 * _v.z + m13) * _iw,
				(m20 * _v.x + m21 * _v.y + m22 * _v.z + m23) * _iw);
		}

		Mat44& CreatePerspective(float _fov, float _aspect, float _near, float _far, bool _reversed = false)
		{
			if (_aspect != _aspect)
				_aspect = 1; // NaN
			if (_far == _near)
				_far = _near + Epsilon;
			float _h = 1 / Tan(_fov * 0.5f);
			float _w = _h / _aspect;
			float _d = (_far - _near);
			float _q = -(_far + _near) / _d;
			float _qn = -2 * (_far * _near) / _d;
			if (_reversed)
				Swap(_q, _qn);
			return *this = Mat44(_w, 0, 0, 0, 0, _h, 0, 0, 0, 0, _q, _qn, 0, 0, -1, 0);
		}
		Mat44& CreateOrtho2D(float _w, float _h, bool _leftTop = true)
		{
			*this = Zero;
			v[0] = 2 / _w, v[3] = -1, v[10] = -1, v[15] = 1;
			if (_leftTop)
				v[5] = -2 / _h, v[7] = 1;
			else
				v[5] = 2 / _h, v[7] = -1;
			return *this;
		}

		union
		{
			struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
			float m[4][4]; // [row][col]
			float v[16];
		};

		ENGINE_API static const Mat44 Zero;
		ENGINE_API static const Mat44 Identity;
	};


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
