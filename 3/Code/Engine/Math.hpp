#pragma once

#include "Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define _FAST_HALF_FLOAT //!< do define for fast half-float conversion

	typedef float float32;
	typedef struct half float16;
	typedef double float64;

	static const float EPSILON = 1e-6f;
	static const float EPSILON2 = 1e-12f;
	static const float PI = 3.1415926535897932384626433832795f;
	static const float DEGREES = 57.295779513082320876798154814105f;
	static const float RADIANS = 0.01745329251994329576923690768489f;

	//----------------------------------------------------------------------------//
	// Math
	//----------------------------------------------------------------------------//

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

	inline uint32_t FirstPow2(uint32_t _val)
	{
		--_val |= _val >> 16;
		_val |= _val >> 8;
		_val |= _val >> 4;
		_val |= _val >> 2;
		_val |= _val >> 1;
		return ++_val;
	}
	inline bool IsPow2(uint32_t _val) { return (_val & (_val - 1)) == 0; }
	inline uint8_t FloatToByte(float _value) { return (uint8_t)(_value * 0xff); }
	inline float ByteToFloat(uint8_t _value) { return (float)(_value * (1.0f / 255.0f)); }
	inline uint16_t FloatToHalf(float _value)
	{
		union { float f; uint32_t i; }_fb = { _value };
#	ifdef _FAST_HALF_FLOAT
		return (uint16_t)((_fb.i >> 16) & 0x8000) | ((((_fb.i & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((_fb.i >> 13) & 0x03ff);
#	else
		uint32_t _s = (_fb.i >> 16) & 0x00008000; // sign
		int32_t _e = ((_fb.i >> 23) & 0x000000ff) - 0x00000070; // exponent
		uint32_t _r = _fb.i & 0x007fffff; // mantissa
		if (_e < 1)
		{
			if (_e < -10)
				return 0;
			_r = (_r | 0x00800000) >> (14 - _e);
			return (uint16_t)(_s | _r);
		}
		else if (_e == 0x00000071)
		{
			if (_r == 0)
				return (uint16_t)(_s | 0x7c00); // Inf
			else
				return (uint16_t)(((_s | 0x7c00) | (_r >>= 13)) | (_r == 0)); // NAN
		}
		if (_e > 30)
			return (uint16_t)(_s | 0x7c00); // Overflow
		return (uint16_t)((_s | (_e << 10)) | (_r >> 13));
#	endif
	}
	inline float HalfToFloat(uint16_t _value)
	{
		union { uint32_t i; float f; }_fb;
#	ifdef _FAST_HALF_FLOAT
		_fb.i = ((_value & 0x8000) << 16) | (((_value & 0x7c00) + 0x1C000) << 13) | ((_value & 0x03FF) << 13);
#	else
		register int32_t _s = (_value >> 15) & 0x00000001; // sign
		register int32_t _e = (_value >> 10) & 0x0000001f; // exponent
		register int32_t _r = _value & 0x000003ff; // mantissa
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
	inline float FixedToFloat(uint32_t _value, uint32_t _bits, float _default = 0.0f)
	{
		if (_bits > 31)
			_bits = 31;
		return _bits ? ((float)_value) / ((float)((1u << _bits) - 1u)) : _default;
	}
	inline uint32_t FloatToFixed(float _value, uint32_t _bits)
	{
		if (_bits > 31)
			_bits = 31;
		if (_value <= 0)
			return 0;
		if (_value >= 1)
			return (1u << _bits) - 1u;
		return static_cast<uint32_t>(_value * (float)(1u << _bits));
	}
	inline uint32_t FixedToFixed(uint32_t _value, uint32_t _from, uint32_t _to)
	{
		if (_from > 31)
			_from = 31;
		if (_to > 31)
			_to = 31;
		if (_from > _to)
			_value >>= _from - _to;
		else if (_from < _to && _value != 0)
		{
			uint32_t _max = (1u << _from) - 1u;
			if (_value == _max)
				_value = (1u << _to) - 1u;
			else if (_max > 0)
				_value *= (1u << _to) / _max;
			else _value = 0;
		}
		return _value;
	}

	//----------------------------------------------------------------------------//
	// half
	//----------------------------------------------------------------------------//

	struct half
	{
		half(void) { }
		half(float _value) { Pack(_value); }
		half& operator = (float _value) { value = FloatToHalf(_value); return *this; }
		operator const float(void) const { return HalfToFloat(value); }
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

		uint16_t value;
	};

	//----------------------------------------------------------------------------//
	// Vec2T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec2T
	{
		Vec2T(void) { }
		Vec2T(T _s) : x(_s), y(_s) { }
		Vec2T(T _x, T _y) : x(_x), y(_y) { }

		Vec2T Copy(void) const { return *this; }
		template <typename X> Vec2T(const Vec2T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) { }
		template <typename X> explicit Vec2T(X _x, X _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }
		template <typename X> X Cast(void) const { return X(x, y); }

		const T operator [] (unsigned  int _index) const { return (&x)[_index]; }
		T& operator [] (unsigned  int _index) { return (&x)[_index]; }
		const T* operator * (void) const { return &x; }
		T* operator * (void) { return &x; }

		Vec2T operator - (void) const { return Vec2T(-x, -y); }
		Vec2T operator + (const Vec2T& _rhs) const { return Vec2T(x + _rhs.x, y + _rhs.y); }
		Vec2T operator - (const Vec2T& _rhs) const { return Vec2T(x - _rhs.x, y - _rhs.y); }
		Vec2T operator * (const Vec2T& _rhs) const { return Vec2T(x * _rhs.x, y * _rhs.y); }
		Vec2T operator / (const Vec2T& _rhs) const { return Vec2T(x / _rhs.x, y / _rhs.y); }
		Vec2T operator * (T _rhs) const { return Vec2T(x * _rhs, y * _rhs); }
		Vec2T operator / (T _rhs) const { return Vec2T(x / _rhs, y / _rhs); }
		Vec2T& operator += (const Vec2T& _rhs) { x += _rhs.x, y += _rhs.y;	return *this; }
		Vec2T& operator -= (const Vec2T& _rhs) { x -= _rhs.x, y -= _rhs.y;	return *this; }
		Vec2T& operator *= (const Vec2T& _rhs) { x *= _rhs.x, y *= _rhs.y;	return *this; }
		Vec2T& operator /= (const Vec2T& _rhs) { x /= _rhs.x, y /= _rhs.y;	return *this; }
		Vec2T& operator *= (T _rhs) { x *= _rhs, y *= _rhs;	return *this; }
		Vec2T& operator /= (T _rhs) { x /= _rhs, y /= _rhs;	return *this; }
		friend Vec2T operator / (T _lhs, const Vec2T& _rhs) { return Vec2T(_lhs / _rhs.x, _lhs / _rhs.y); }
		friend Vec2T operator * (T _lhs, const Vec2T& _rhs) { return Vec2T(_lhs * _rhs.x, _lhs * _rhs.y); }

		bool operator == (const Vec2T& _rhs) const { return x == _rhs.x && y == _rhs.y; }
		bool operator != (const Vec2T& _rhs) const { return x != _rhs.x || y != _rhs.y; }

		Vec2T& Set(T _s) { x = _s, y = _s; return *this; }
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
	typedef Vec2T<unsigned int> Vec2ui;

	//----------------------------------------------------------------------------//
	// Vec3T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec3T
	{
		Vec3T(void) { }
		Vec3T(T _s) : x(_s), y(_s), z(_s) { }
		Vec3T(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }

		Vec3T Copy(void) const { return *this; }
		template <typename X> Vec3T(const Vec3T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)) { }
		template <typename X> explicit Vec3T(X _x, X _y, X _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) { }
		template <typename X> X Cast(void) const { return X(x, y, z); }

		const T operator [] (unsigned  int _index) const { return (&x)[_index]; }
		T& operator [] (unsigned  int _index) { return (&x)[_index]; }
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

		Vec3T& Set(T _s) { x = _s, y = _s, z = _s; return *this; }
		Vec3T& Set(T _x, T _y, T _z) { x = _x, y = _y, z = _z; return *this; }
		Vec3T& Set(const Vec3T& _other) { return *this = _other; }

		// for Vec3T<float>

		Vec3T<float>& SetMin(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Min(_a.x, _b.x), y = Min(_a.y, _b.y), z = Min(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMin(const Vec3T<float>& _other) { return SetMin(*this, _other); }
		Vec3T<float>& SetMax(const Vec3T<float>& _a, const Vec3T<float>& _b) { x = Max(_a.x, _b.x), y = Max(_a.y, _b.y), z = Max(_a.z, _b.z); return *this; }
		Vec3T<float>& SetMax(const Vec3T<float>& _other) { return SetMax(*this, _other); }
		Vec3T<float>& Normalize(void)
		{
			float _l = LengthSq();
			if (_l > EPSILON2)
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
			Vec3T<float> _perp = Cross(UnitX);
			if (_perp.LengthSq() <= EPSILON2)
				_perp = Cross(UnitY);
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
	typedef Vec3T<unsigned int> Vec3ui;

	//----------------------------------------------------------------------------//
	// Vec4T
	//----------------------------------------------------------------------------//

	template <typename T> struct Vec4T
	{
		Vec4T(void) { }
		Vec4T(T _s) : x(_s), y(_s), z(_s), w(_s) { }
		Vec4T(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) { }

		Vec4T Copy(void) const { return *this; }
		template <typename X> Vec4T(const Vec4T<X>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)), w(static_cast<T>(_v.w)) { }
		template <typename X> explicit Vec4T(X _x, X _y, X _z, X _w) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)), w(static_cast<T>(_w)) { }
		template <typename X> X Cast(void) const { return X(x, y, z, w); }

		const T operator [] (unsigned  int _index) const { return (&x)[_index]; }
		T& operator [] (unsigned  int _index) { return (&x)[_index]; }
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
		Vec4T& Set(T _s) { x = _s, y = _s, z = _s, w = _s; return *this; }
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
	typedef Vec4T<int8_t> Vec4b;
	typedef Vec4T<uint8_t> Vec4ub;
	typedef Vec4T<uint16_t> Vec4us;

	//----------------------------------------------------------------------------//
	// Quat
	//----------------------------------------------------------------------------//

	struct Quat
	{
		Quat(void) { }
		Quat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
		Quat Copy(void) const { return *this; }

		template <typename X> X Cast(void) const { return X(x, y, z, w); }

		const float operator [] (unsigned int _index) const { return v[_index]; }
		float& operator [] (unsigned int _index) { return v[_index]; }
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
		friend Vec3 operator * (const Vec3& _lhs, const Quat& _rhs)
		{
			const Vec3& _q = *(Vec3*)_rhs.v;
			Vec3 _uv(_q.Cross(_lhs));
			return _lhs + _uv * 2 * _rhs.w + _q.Cross(_uv) * 2;
		}

		Quat& Set(float _x, float _y, float _z, float _w) { x = _x, y = _y, z = _z, w = _w; return *this; }
		Quat& Set(const Quat& _other) { return *this = _other; }

		Quat& Multiply(const Quat& _rhs);
		float Dot(const Quat& _q) const { return x * _q.x + y * _q.y + z * _q.z + w * _q.w; }
		Quat& Normalize(void);
		Quat& Inverse(void);
		Quat& UnitInverse(void) { x = -x, y = -y, z = -z; return *this; }

		Quat Nlerp(const Quat& _q, float _t, bool _shortestPath = false) const;
		Quat Slerp(const Quat& _q, float _t, bool _shortestPath = false) const;

		void ToMatrixRows(float* _r0, float* _r1, float* _r2) const;
		Quat& FromMatrixRows(const float* _r0, const float* _r1, const float* _r2);

		Quat& FromAxisAngle(const Vec3& _axis, float _angle);

		union
		{
			struct { float x, y, z, w; };
			float v[4];
		};

		static const Quat Zero;
		static const Quat Identity;
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
		Mat34(float _value) { *this = Zero; m00 = _value, m11 = _value, m22 = _value; }
		Mat34(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23) :
			m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23) { }

		Mat34 Copy(void) const { return *this; }

		const float* operator [] (int _row) const { return m[_row]; }
		float* operator [] (int _row) { return m[_row]; }
		const float* operator * (void) const { return v; }
		float* operator * (void) { return v; }
		float& operator () (unsigned int _row, unsigned int _col) { return m[_row][_col]; }
		const float operator () (unsigned int _row, unsigned int _col) const { return m[_row][_col]; }
		const Vec3& Row(unsigned int _row) const { return *(Vec3*)(m[_row]); }
		Vec3& Row(unsigned int _row) { return *(Vec3*)(m[_row]); }

		Mat34 operator + (const Mat34& _rhs) const { return Copy().Add(_rhs); }
		Mat34 operator * (const Mat34& _rhs) const { return Copy().Multiply(_rhs); }
		Mat34 operator * (float _rhs) const { return Copy().Multiply(_rhs); }
		friend Vec3 operator * (const Vec3& _lhs, const Mat34& _rhs) { return _rhs.Transform(_lhs); } // Vec3 = Vec3 * Mat34
		Mat34& operator += (const Mat34& _rhs) { return Add(_rhs); }
		Mat34& operator *= (const Mat34& _rhs) { return Multiply(_rhs); }
		Mat34& operator *= (float _rhs) { return Multiply(_rhs); }

		Mat34& Set(float _value) { *this = Zero; m00 = _value, m11 = _value, m22 = _value; return *this; }
		Mat34& Set(const Mat34& _m) { return *this = _m; }
		Mat34& FromPtr(const void* _ptr) { memcpy(v, _ptr, 12 * sizeof(float));  return *this; }

		Mat34& Add(const Mat34& _rhs) { for (int i = 0; i < 12; ++i) v[i] += v[i]; return *this; }
		Mat34& Multiply(float _rhs) { for (int i = 0; i < 12; ++i) v[i] *= _rhs; return *this; }
		Mat34& Multiply(const Mat34& _rhs);
		float Determinant(void) const { return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 - m02 * m11 * m20 - m00 * m12 * m21 - m01 * m10 * m22; }
		Mat34& Inverse(void);

		Vec3 Translate(const Vec3& _v) const { return Vec3(_v.x + m03, _v.y + m13, _v.z + m23); }
		Vec3 Transform(const Vec3& _v) const { return Vec3(m00 * _v.x + m01 * _v.y + m02 * _v.z + m03, m10 * _v.x + m11 * _v.y + m12 * _v.z + m13, m20 * _v.x + m21 * _v.y + m22 * _v.z + m23); }
		Vec3 TransformVectorAbs(const Vec3& _v) const { return Vec3(Abs(m00) * _v.x + Abs(m01) * _v.y + Abs(m02) * _v.z, Abs(m10) * _v.x + Abs(m11) * _v.y + Abs(m12) * _v.z, Abs(m20) * _v.x + Abs(m21) * _v.y + Abs(m22) * _v.z); }
		Vec3 TransformVector(const Vec3& _v) const { return Vec3(m00 * _v.x + m01 * _v.y + m02 * _v.z, m10 * _v.x + m11 * _v.y + m12 * _v.z, m20 * _v.x + m21 * _v.y + m22 * _v.z); }

		Mat34& SetTranslation(const Vec3& _translation) { m03 = _translation.x, m13 = _translation.y, m23 = _translation.z; return *this; }
		Vec3 GetTranslation(void) const { return Vec3(m03, m12, m23); }
		Mat34& CreateTranslation(const Vec3& _translation) { return (*this = Identity).SetTranslation(_translation); }

		Mat34& SetRotation(const Quat& _rotation);
		Quat GetRotation(void) const;
		Mat34& CreateRotation(const Quat& _rotation);

		bool HasScale(void) const;
		bool HasNegativeScale(void) const { return Determinant() < 0; }
		Mat34& SetScale(const Vec3& _scale);
		Vec3 GetScale(void) const;
		Mat34& CreateScale(const Vec3& _scale);

		Mat34& CreateTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::One); // Ordering: Scale, Rotate, Translate
		Mat34& CreateInverseTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale = Vec3::One);

		union
		{
			struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23; };
			float m[3][4]; // [row][col]
			float v[12];
		};

		static const Mat34 Zero;
		static const Mat34 Identity;
	};

	//----------------------------------------------------------------------------//
	// Mat44
	//----------------------------------------------------------------------------//

	struct Mat44
	{
		Mat44(void) { }
		Mat44(float _value) { *this = Zero; m00 = _value, m11 = _value, m22 = _value, m33 = _value; }
		Mat44(const Mat34& _m34) : m30(0), m31(0), m32(0), m33(1) { memcpy(v, *_m34, 12 * sizeof(float)); }
		Mat44(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33) :
			m00(_00), m01(_01), m02(_02), m03(_03), m10(_10), m11(_11), m12(_12), m13(_13), m20(_20), m21(_21), m22(_22), m23(_23), m30(_30), m31(_31), m32(_32), m33(_33) { }

		Mat44 Copy(void) const { return *this; }

		operator const Mat34& (void) const { /*assert(IsAffine());*/ return *(Mat34*)(v); }
		const float* operator [] (int _row) const { return m[_row]; }
		float* operator [] (int _row) { return m[_row]; }
		const float* operator * (void) const { return v; }
		float* operator * (void) { return v; }
		float& operator () (unsigned int _row, unsigned int _col) { return m[_row][_col]; }
		const float operator () (unsigned int _row, unsigned int _col) const { return m[_row][_col]; }
		const Vec3& Row(unsigned int _row) const { return *(Vec3*)(m[_row]); }
		Vec3& Row(unsigned int _row) { return *(Vec3*)(m[_row]); }
		const Vec4& Row4(unsigned int _row) const { return *(Vec4*)(m[_row]); }
		Vec4& Row4(unsigned int _row) { return *(Vec4*)(m[_row]); }

		Mat44 operator + (const Mat44& _rhs) const { return Copy().Add(_rhs); }
		Mat44 operator * (const Mat44& _rhs) const { return Copy().Multiply(_rhs); }
		Mat44 operator * (const Mat34& _rhs) const { return Copy().Multiply(_rhs); }
		Mat44 operator * (float _rhs) const { return Copy().Multiply(_rhs); }
		friend Vec3 operator * (const Vec3& _lhs, const Mat44& _rhs) { return _rhs.Transform(_lhs); }
		Mat44& operator += (const Mat44& _rhs) { return Add(_rhs); }
		Mat44& operator *= (const Mat44& _rhs) { return Multiply(_rhs); }
		Mat44& operator *= (const Mat34& _rhs) { return Multiply(_rhs); }
		Mat44& operator *= (float _rhs) { return Multiply(_rhs); }

		Mat44& Set(float _value) { *this = Zero; m00 = _value, m11 = _value, m22 = _value, m33 = _value; return *this; }
		Mat44& Set(const Mat34& _m) { memcpy(v, *_m, 12 * sizeof(float)); Row4(3) = Vec4::Zero; return *this; }
		Mat44& Set(const Mat44& _m) { return *this = _m; }
		Mat44& Insert(const Mat34& _m) { memcpy(v, *_m, 12 * sizeof(float)); return *this; }
		Mat44& FromPtr(const void* _ptr) { memcpy(v, _ptr, 16 * sizeof(float)); return *this; }

		Mat44& Add(const Mat44& _rhs) { for (int i = 0; i < 16; ++i) v[i] += _rhs.v[i]; return *this; }
		Mat44& Multiply(float _rhs) { for (int i = 0; i < 16; ++i) v[i] += _rhs; return *this; }
		Mat44& Multiply(const Mat44& _rhs);
		Mat44& Multiply(const Mat34& _rhs);
		Mat44& Inverse(void);
		Vec3 Transform(const Vec3& _v) const;

		Mat44& CreatePerspective(float _fov, float _aspect, float _near, float _far, bool _reversed = false);
		Mat44& CreateOrtho2D(float _w, float _h, bool _leftTop = true);

		union
		{
			struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
			float m[4][4]; // [row][col]
			float v[16];
		};

		static const Mat44 Zero;
		static const Mat44 Identity;
	};

	//----------------------------------------------------------------------------//
	// TRect
	//----------------------------------------------------------------------------//

	template <typename T> struct TRect
	{
		typedef Vec2T<T> Point;

		TRect(void) : pos(0), size(0) { }
		TRect(T _x, T _y, T _w, T _h) : pos(_x, _y), size(_w, _h) { }
		TRect(const Point& _pos, const Point& _size) : pos(_pos), size(_size) { }

		TRect& FromPoints(const Point& _mn, const Point& _mx) { pos = _mn; size = _mx - _mn; return *this; }
		TRect& SetSize(T _w, T _h) { size.Set(_w, _h); return *this; }
		TRect& SetPos(T _x, T _y) { pos.Set(_x, _y); return *this; }
		T Width(void) const { return size.x; }
		T Height(void) const { return size.y; }
		const Point& Pos(void) const { return pos; }
		const Point& Size(void) const { return size; }
		const Point& Mn(void) const { return pos; }
		Point Mx(void) const { return pos + size; }

		Point pos, size;
	};

	typedef TRect<int> Recti;
	typedef TRect<float> Rect;

	//----------------------------------------------------------------------------//
	// Ray
	//----------------------------------------------------------------------------//

	struct Ray
	{
		///
		Ray(void) :
			origin(Vec3::Zero), dir(Vec3::UnitZ)
		{
		}
		///
		Ray(const Vec3& _origin, const Vec3& _dir) :
			origin(_origin), dir(_dir)
		{
		}
		///
		Vec3 Point(float _t) const
		{
			return origin + dir * _t;
		}
		///
		Vec3 operator * (float _t) const
		{
			return Point(_t);
		}
		///
		friend Vec3 operator * (float _t, const Ray& _ray)
		{
			return _ray.Point(_t);
		}

		Vec3 origin, dir;
	};

	//----------------------------------------------------------------------------//
	// Plane
	//----------------------------------------------------------------------------//

	struct Plane
	{
		Plane(void) : normal(Vec3::Zero), dist(0) { }
		Plane(const Vec3& _n, float _d) : normal(_n), dist(_d) { }

		Plane Copy(void) const { return *this; }

		Plane& Set(float _nx, float _ny, float _nz, float _d) { normal.Set(_nx, _ny, _nz); dist = _d; return *this; }
		Plane& Set(const Vec3& _normal, float _distance) { normal = _normal, dist = _distance; return *this; }

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

		Plane& FromPoint(const Vec3& _normal, const Vec3& _point) { return Set(_normal, -_normal.Dot(_point)); }

		Vec3 normal;
		float dist;
	};

	//----------------------------------------------------------------------------//
	// Sphere
	//----------------------------------------------------------------------------//

	///
	struct Sphere
	{
		Sphere(void) : center(Vec3::Zero), radius(0) { }
		Sphere(const Vec3& _center, float _radius) : center(_center), radius(_radius) { }

		bool Contains(const Vec3& _pt) const { return center.DistanceSq(_pt) <= Sqr(radius); }
		bool Intersects(const Sphere& _bv) const { return center.DistanceSq(_bv.center) <= Sqr(radius + _bv.radius); }

		Vec3 center;
		float radius;
	};

	//----------------------------------------------------------------------------//
	// AlignedBox
	//----------------------------------------------------------------------------//

	enum BoxCorner : unsigned int
	{
		BC_LeftBottomFar = 0,// min
		BC_RightBottomFar,
		BC_RightBottomNear,
		BC_LeftBottomNear,
		BC_RightTopNear, // max
		BC_LeftTopNear,
		BC_LeftTopFar,
		BC_RightTopFar,
	};

	struct AlignedBox
	{
		AlignedBox(void) : mn(999999.9f), mx(-999999.9f) { }
		AlignedBox(const Vec3& _min, const Vec3& _max) : mn(_min), mx(_max) { }

		AlignedBox Copy(void) const { return *this; }

		AlignedBox& Set(const AlignedBox& _b) { return *this = _b; }
		AlignedBox& Set(const Vec3& _min, const Vec3& _max) { mn.Set(_min), mx.Set(_max); return *this; }
		AlignedBox& SetMinMax(const Vec3& _a, const Vec3& _b) { mn.SetMin(_a, _b), mx.SetMax(_a, _b); return *this; }
		AlignedBox& SetZero(void) { mn = Vec3::Zero, mx = Vec3::Zero; return *this; }
		AlignedBox& FromCenterExtends(const Vec3& _center, const Vec3& _extends) { return Set(_center - _extends, _center + _extends); }
		AlignedBox& FromViewProjMatrix(const Mat44& _m);

		AlignedBox& Reset(const Vec3& _pt) { return Set(_pt, _pt); }
		AlignedBox& Reset(void) { mn = 999999.9f, mx = -999999.9f; return *this; }
		AlignedBox& AddPoint(const Vec3& _pt) { mn.SetMin(_pt), mx.SetMax(_pt); return *this; }
		AlignedBox& AddVertices(const void* _data, unsigned int _count, unsigned int _stride = 0, unsigned int _offset = 0);

		bool IsZero(void) const { return mn == mx && mn == Vec3::Zero; }
		bool IsFinite(void) const { return mn.x <= mx.x && mn.y <= mx.y && mn.z <= mx.z; }

		Vec3 Size(void) const { return mx - mn; }
		Vec3 Extends(void) const { return (mx - mn) * 0.5f; }
		Vec3 Center(void) const { return (mx + mn) * 0.5f; }
		operator Sphere (void) const { return Sphere(Center(), Radius()); }
		float Diagonal(void) const { return (mx - mn).Length(); }
		float DiagonalSq(void) const { return (mx - mn).LengthSq(); }
		float Radius(void) const { return Diagonal() * 0.5f; }
		float Width(void) const { return mx.x - mn.x; }
		float Height(void) const { return mx.y - mn.y; }
		float Depth(void) const { return mx.z - mn.z; }
		float Volume(void) const { return (mx - mn).LengthSq(); }
		void GetAllCorners(const void* _data, unsigned int _stride = 0, unsigned int _offset = 0) const;

		AlignedBox operator + (const Vec3& _point) const { return Copy().AddPoint(_point); }
		AlignedBox operator + (const AlignedBox& _box) const { return Copy().AddPoint(_box.mn).AddPoint(_box.mx); }
		AlignedBox& operator += (const Vec3& _point) { return AddPoint(_point); }
		AlignedBox& operator += (const AlignedBox& _box) { return AddPoint(_box.mn).AddPoint(_box.mx); }
		AlignedBox operator * (const Mat34& _rhs) const { return AlignedBox().FromCenterExtends(Center() * _rhs, _rhs.TransformVectorAbs(Extends())); }
		AlignedBox operator * (const Mat44& _rhs) const;

		bool Contains(const Vec3& _point) const;
		bool Contains(const AlignedBox& _box) const;
		bool Intersects(const AlignedBox& _box, bool* _contains) const;
		bool Intersects(const AlignedBox& _box) const;
		bool Intersects(const Ray& _ray, float* _distance = nullptr, Vec3* _point = nullptr) const;

		Vec3 mn, mx;

		static const AlignedBox Zero;
		static const AlignedBox Inf;
		static const uint16_t Lines[24];
		static const uint16_t Quads[24];
		static const uint16_t Triangles[36];
	};

	//----------------------------------------------------------------------------//
	// Frustum
	//----------------------------------------------------------------------------//

	enum FrustumPlane : unsigned int
	{
		FP_Left = 0,
		FP_Right,
		FP_Bottom,
		FP_Top,
		FP_Near,
		FP_Far,
	};

	struct Frustum
	{
		Frustum(void) { }

		Frustum& FromCameraMatrices(const Mat34& _view, const Mat44& _proj);

		bool Intersects(const Vec3& _point) const;
		bool Intersects(const Vec3& _center, float _radius) const;
		bool Intersects(const AlignedBox& _box, bool* _contains = nullptr) const;
		bool Intersects(const Frustum& _frustum, bool* _contains = nullptr) const;

		float Distance(const Vec3& _point) const { return origin.Distance(_point); }
		float Distance(const Vec3& _center, float _radius) const { float _d = origin.Distance(_center); return _d < _radius ? 0 : _d - _radius; }
		float Distance(const AlignedBox& _box) const { return Distance(_box.Center(), _box.Radius()); }

		static const uint16_t Lines[24];
		static const uint16_t Quads[24];

		Plane planes[6];
		Vec3 corners[8];
		Vec3 origin;
		AlignedBox box;
	};

	//----------------------------------------------------------------------------//
	// Dbvt
	//----------------------------------------------------------------------------//

	///\warning It is very simple implementation (linear search O(n))
	class Dbvt
	{
	public:

		struct Node	// 36 bytes (x86), 48 bytes (x64)
		{
			AlignedBox box = AlignedBox::Inf;
			Node* parent = nullptr;
			union
			{
				Node* childs[2] = { nullptr };
				void* object;
				uint index;
			};

			bool IsNode(void) const { return childs[1] != nullptr; }
			bool IsLeaf(void) const { return childs[1] == nullptr; }
		};

		struct Callback
		{
			virtual void AddResult(void* _object) = 0;
		};

		Node* Add(void* _object, const AlignedBox& _bv)
		{
			ASSERT(_bv.IsFinite());
			Node* _nn = new Node;
			_nn->object = _object;
			_nn->box = _bv;
			m_nodes.PushBack(_nn);
			return _nn;
		}
		void Remove(Node* _node)
		{
			ASSERT(_node != nullptr);
			m_nodes.Remove(_node);
			delete _node;
		}
		void Update(Node* _node)
		{
			ASSERT(_node != nullptr);
			ASSERT(_node->box.IsFinite());
		}
		void EnumObjects(const Frustum& _bv, Callback& _callback)
		{
			for (uint i = 0; i < m_nodes.Size(); ++i)
			{
				Node* _n = m_nodes[i];
				if (_bv.Intersects(_n->box))
				{
					_callback.AddResult(_n->object);
				}
			}
		}
		void EnumObjects(const Frustum& _bv, const Frustum& _bv2, Callback& _callback)
		{
			for (uint i = 0; i < m_nodes.Size(); ++i)
			{
				Node* _n = m_nodes[i];
				if (_bv.Intersects(_n->box) && _bv2.Intersects(_n->box))
				{
					_callback.AddResult(_n->object);
				}
			}
		}
		void EnumObjects(const AlignedBox& _bv, Callback& _callback)
		{
			for (uint i = 0; i < m_nodes.Size(); ++i)
			{
				Node* _n = m_nodes[i];
				if (_bv.Intersects(_n->box))
				{
					_callback.AddResult(_n->object);
				}
			}
		}
		void EnumObjects(const Sphere& _bv, Callback& _callback)
		{
			for (uint i = 0; i < m_nodes.Size(); ++i)
			{
				Node* _n = m_nodes[i];
				if (_bv.Intersects(_n->box))
				{
					_callback.AddResult(_n->object);
				}
			}
		}

	//protected:

		//void _Insert(Node*);

		//Node* m_root;
		SArray<Node*, uint> m_nodes; // temp
	};

	typedef Dbvt::Node DbvtNode;

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
