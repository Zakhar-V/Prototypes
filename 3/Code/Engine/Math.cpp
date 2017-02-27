#include "Math.hpp"

namespace Engine
{
	TODO_EX("MathLib", "Добавить функции сериализации");
	TODO_EX("MathLib", "Реализовать Dbvt");

	//----------------------------------------------------------------------------//
	// Quat
	//----------------------------------------------------------------------------//

	const Quat Quat::Zero(0, 0, 0, 0);
	const Quat Quat::Identity(0, 0, 0, 1);

	//----------------------------------------------------------------------------//
	Quat& Quat::Multiply(const Quat& _rhs)
	{
		return Set(w * _rhs.x + x * _rhs.w + y * _rhs.z - z * _rhs.y,
			w * _rhs.y + y * _rhs.w + z * _rhs.x - x * _rhs.z,
			w * _rhs.z + z * _rhs.w + x * _rhs.y - y * _rhs.x,
			w * _rhs.w - x * _rhs.x - y * _rhs.y - z * _rhs.z);
	}
	//----------------------------------------------------------------------------//
	Quat& Quat::Normalize(void)
	{
		float _l = x * x + y * y + z * z + w * w;
		if (_l > EPSILON2)
			*this *= RSqrt(_l);
		return *this;
	}
	//----------------------------------------------------------------------------//
	Quat& Quat::Inverse(void)
	{
		float _d = x * x + y * y + z * z + w * w;
		if (_d > 0)
			x *= -_d, y *= -_d, z *= -_d, w *= _d;
		else
			x = 0, y = 0, z = 0, w = 1;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Quat Quat::Nlerp(const Quat& _q, float _t, bool _shortestPath) const
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
	//----------------------------------------------------------------------------//
	Quat Quat::Slerp(const Quat& _q, float _t, bool _shortestPath) const
	{
		const Quat& _p = *this;
		float _c = _p.Dot(_q);
		Quat _tmp;
		if (_c < 0 && _shortestPath)
			_c = -_c, _tmp = -_q;
		else
			_tmp = _q;
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
	//----------------------------------------------------------------------------//
	void Quat::ToMatrixRows(float* _r0, float* _r1, float* _r2) const
	{
		float _x2 = x + x, _y2 = y + y, _z2 = z + z;
		float _wx = _x2 * w, _wy = _y2 * w, _wz = _z2 * w, _xx = _x2 * x, _xy = _y2 * x, _xz = _z2 * x, _yy = _y2 * y, _yz = _z2 * y, _zz = _z2 * z;
		_r0[0] = 1 - (_yy + _zz), _r0[1] = _xy + _wz, _r0[2] = _xz - _wy;
		_r1[0] = _xy - _wz, _r1[1] = 1 - (_xx + _zz), _r1[2] = _yz + _wx;
		_r2[0] = _xz + _wy, _r2[1] = _yz - _wx, _r2[2] = 1 - (_xx + _yy);
	}
	//----------------------------------------------------------------------------//
	Quat& Quat::FromMatrixRows(const float* _r0, const float* _r1, const float* _r2)
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
	//----------------------------------------------------------------------------//
	Quat& Quat::FromAxisAngle(const Vec3& _axis, float _angle)
	{
		float _s, _c;
		SinCos(_angle*0.5f, _s, _c);
		return Set(_axis.x * _s, _axis.y * _s, _axis.z * _s, _c);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Mat34
	//----------------------------------------------------------------------------//

	const Mat34 Mat34::Zero(0);
	const Mat34 Mat34::Identity(1);

	//----------------------------------------------------------------------------//
	Mat34& Mat34::Multiply(const Mat34& _rhs)
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
	//----------------------------------------------------------------------------//
	Mat34& Mat34::Inverse(void)
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
	//----------------------------------------------------------------------------//
	Mat34& Mat34::SetRotation(const Quat& _rotation)
	{
		Vec3 _scale = GetScale();
		_rotation.ToMatrixRows(m[0], m[1], m[2]);
		Row(0) *= _scale, Row(1) *= _scale, Row(2) *= _scale;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Quat Mat34::GetRotation(void) const
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
	//----------------------------------------------------------------------------//
	Mat34& Mat34::CreateRotation(const Quat& _rotation)
	{
		SetTranslation(Vec3::Zero);
		_rotation.ToMatrixRows(m[0], m[1], m[2]);
		return *this;
	}
	//----------------------------------------------------------------------------//
	bool Mat34::HasScale(void) const
	{
		if (!(Abs((m00 * m00 + m10 * m10 + m20 * m20) - 1) < EPSILON))
			return true;
		if (!(Abs((m01 * m01 + m11 * m11 + m21 * m21) - 1) < EPSILON))
			return true;
		if (!(Abs((m02 * m02 + m12 * m12 + m22 * m22) - 1) < EPSILON))
			return true;
		return false;
	}
	//----------------------------------------------------------------------------//
	Mat34& Mat34::SetScale(const Vec3& _scale)
	{
		Quat _rotation = GetRotation();
		_rotation.ToMatrixRows(m[0], m[1], m[2]);
		Row(0) *= _scale, Row(1) *= _scale, Row(2) *= _scale;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Vec3 Mat34::GetScale(void) const
	{
		return Vec3(
			Sqrt(m00 * m00 + m10 * m10 + m20 * m20),
			Sqrt(m01 * m01 + m11 * m11 + m21 * m21),
			Sqrt(m02 * m02 + m12 * m12 + m22 * m22));
	}
	//----------------------------------------------------------------------------//
	Mat34& Mat34::CreateScale(const Vec3& _scale)
	{
		*this = Zero;
		m00 = _scale.x, m11 = _scale.y, m22 = _scale.z;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Mat34& Mat34::CreateTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale)
	{
		// Ordering: Scale, Rotate, Translate
		float _r0[3], _r1[3], _r2[3];
		_rotation.ToMatrixRows(_r0, _r1, _r2);
		m00 = _scale.x * _r0[0], m01 = _scale.y * _r0[1], m02 = _scale.z * _r0[2], m03 = _translation.x;
		m10 = _scale.x * _r1[0], m11 = _scale.y * _r1[1], m12 = _scale.z * _r1[2], m13 = _translation.y;
		m20 = _scale.x * _r2[0], m21 = _scale.y * _r2[1], m22 = _scale.z * _r2[2], m23 = _translation.z;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Mat34& Mat34::CreateInverseTransform(const Vec3& _translation, const Quat& _rotation, const Vec3& _scale)
	{
		Vec3 _inv_scale(1 / _scale);
		Quat _inv_rotation = _rotation.Copy().Inverse();
		Vec3 _inv_translation((-_translation * _inv_rotation) * _inv_scale);
		return CreateTransform(_inv_translation, _inv_rotation, _inv_scale);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Mat44
	//----------------------------------------------------------------------------//

	const Mat44 Mat44::Zero(0);
	const Mat44 Mat44::Identity(1);

	//----------------------------------------------------------------------------//
	Mat44& Mat44::Multiply(const Mat44& _rhs)
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
	//----------------------------------------------------------------------------//
	Mat44& Mat44::Multiply(const Mat34& _rhs)
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
	//----------------------------------------------------------------------------//
	Mat44& Mat44::Inverse(void)
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
	//----------------------------------------------------------------------------//
	Vec3 Mat44::Transform(const Vec3& _v) const
	{
		float _iw = 1 / (m30 * _v.x + m31 * _v.y + m32 * _v.z + m33);
		return Vec3(
			(m00 * _v.x + m01 * _v.y + m02 * _v.z + m03) * _iw,
			(m10 * _v.x + m11 * _v.y + m12 * _v.z + m13) * _iw,
			(m20 * _v.x + m21 * _v.y + m22 * _v.z + m23) * _iw);
	}
	//----------------------------------------------------------------------------//
	Mat44& Mat44::CreatePerspective(float _fov, float _aspect, float _near, float _far, bool _reversed)
	{
		if (_aspect != _aspect)
			_aspect = 1; // NaN
		if (_far == _near)
			_far = _near + EPSILON;
		float _h = 1 / Tan(_fov * 0.5f);
		float _w = _h / _aspect;
		float _d = (_far - _near);
		float _q = -(_far + _near) / _d;
		float _qn = -2 * (_far * _near) / _d;
		if (_reversed)
		{
			float _tmp = _q;
			_q = _qn;
			_qn = _tmp;
		}
		return *this = Mat44(_w, 0, 0, 0, 0, _h, 0, 0, 0, 0, _q, _qn, 0, 0, -1, 0);
	}
	//----------------------------------------------------------------------------//
	Mat44& Mat44::CreateOrtho2D(float _w, float _h, bool _leftTop)
	{
		*this = Zero;
		v[0] = 2 / _w, v[3] = -1, v[10] = -1, v[15] = 1;
		if (_leftTop)
			v[5] = -2 / _h, v[7] = 1;
		else
			v[5] = 2 / _h, v[7] = -1;
		return *this;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// AlignedBox
	//----------------------------------------------------------------------------//

	const AlignedBox AlignedBox::Zero(Vec3::Zero, Vec3::Zero);
	const AlignedBox AlignedBox::Inf(999999.9f, -999999.9f);
	const uint16_t AlignedBox::Lines[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 3, 5, 2, 4, 1, 7, 0, 6, 4, 5, 5, 6, 6, 7, 7, 4 };
	const uint16_t AlignedBox::Quads[24] = { 5, 3, 2, 4, 4, 2, 1, 7, 7, 1, 0, 6, 6, 0, 3, 5, 6, 5, 4, 7, 2, 3, 0, 1 };
	const uint16_t AlignedBox::Triangles[36] = { 5, 3, 2, 5, 2, 4, 4, 2, 1, 4, 1, 7, 7, 1, 0, 7, 0, 6, 6, 0, 3, 6, 3, 5, 6, 5, 4, 6, 4, 7, 3, 0, 1, 3, 1, 2 };

	//----------------------------------------------------------------------------//
	AlignedBox& AlignedBox::FromViewProjMatrix(const Mat44& _m)
	{
		Reset(Vec3(-1, +1, +1) * _m);
		AddPoint(Vec3(+1, +1, +1) * _m);
		AddPoint(Vec3(-1, -1, +1) * _m);
		AddPoint(Vec3(+1, -1, +1) * _m);
		AddPoint(Vec3(-1, +1, -1) * _m);
		AddPoint(Vec3(+1, +1, -1) * _m);
		AddPoint(Vec3(-1, -1, -1) * _m);
		AddPoint(Vec3(+1, -1, -1) * _m);
		return *this;
	}
	//----------------------------------------------------------------------------//
	AlignedBox& AlignedBox::AddVertices(const void* _data, unsigned int _count, unsigned int _stride, unsigned int _offset)
	{
		if (_data && _count > 0)
		{
			union { const uint8_t* p; const Vec3* v; } _vertices = { ((const uint8_t*)_data) + _offset };
			for (unsigned int i = 0, _step = _stride ? _stride : sizeof(Vec3); i < _count; ++i)
			{
				mn.SetMin(*_vertices.v);
				mx.SetMax(*_vertices.v);
				_vertices.p += _step;
			}
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	void AlignedBox::GetAllCorners(const void* _data, unsigned int _stride, unsigned int _offset) const
	{
		if (_data)
		{
			union { uint8_t* p; Vec3* v; } _vertices = { ((uint8_t*)_data) + _offset };
			unsigned int _step = _stride ? _stride : sizeof(Vec3);
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
	//----------------------------------------------------------------------------//
	AlignedBox AlignedBox::operator * (const Mat44& _rhs) const
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
	//----------------------------------------------------------------------------//
	bool AlignedBox::Contains(const Vec3& _point) const
	{
		return !(_point.x < mn.x || _point.x > mx.x || _point.y < mn.y || _point.y > mx.y || _point.z < mn.z || _point.z > mx.z);
	}
	//----------------------------------------------------------------------------//
	bool AlignedBox::Contains(const AlignedBox& _box) const
	{
		return (mx.x >= _box.mx.x && mx.y >= _box.mx.y && mx.z >= _box.mx.z && mn.x <= _box.mn.x && mn.y <= _box.mn.y && mn.z <= _box.mn.z);
	}
	//----------------------------------------------------------------------------//
	bool AlignedBox::Intersects(const AlignedBox& _box, bool* _contains) const
	{
		if (mx.x < _box.mn.x || mx.y < _box.mn.y || mx.z < _box.mn.z || mn.x > _box.mx.x || mn.y > _box.mx.y || mn.z > _box.mx.z)
			return false;
		if (_contains)
			*_contains = (mx.x >= _box.mx.x && mx.y >= _box.mx.y && mx.z >= _box.mx.z && mn.x <= _box.mn.x && mn.y <= _box.mn.y && mn.z <= _box.mn.z);
		return true;
	}
	//----------------------------------------------------------------------------//
	bool AlignedBox::Intersects(const AlignedBox& _box) const
	{
		return !(IsZero() || _box.IsZero() || mx.x < _box.mn.x || mx.y < _box.mn.y || mx.z < _box.mn.z || mn.x > _box.mx.x || mn.y > _box.mx.y || mn.z > _box.mx.z);
	}
	//----------------------------------------------------------------------------//
	bool AlignedBox::Intersects(const Ray& _ray, float* _distance, Vec3* _point) const
	{
		static const unsigned int _axes[3][3] = { { 0, 1, 2 },{ 1, 0, 2 },{ 2, 0, 1 } };

		if (IsZero())
			return false;

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
			for (unsigned int i = 0, x, y, z; i < 3 && (!_hit || _distance || _point); ++i)
			{
				x = _axes[i][0], y = _axes[i][1], z = _axes[i][2];
				if (_origin[x] <= _min[x] && _dir[x] > 0)
				{
					_t = (_min[x] - _origin[x]) / _dir[x];
					if (_t >= 0)
					{
						_hit_point = _origin + _dir * _t;
						if ((!_hit || _t < _low_t) && _hit_point[y] >= _min[y] && _hit_point[y] <= _max[y] && _hit_point[z] >= _min[z] && _hit_point[z] <= _max[z])
							_hit = true, _low_t = _t;
					}
				}
				if (_origin[x] >= _max[x] && _dir[x] < 0)
				{
					_t = (_max[x] - _origin[x]) / _dir[x];
					if (_t >= 0)
					{
						_hit_point = _origin + _dir * _t;
						if ((!_hit || _t < _low_t) && _hit_point[y] >= _min[y] && _hit_point[y] <= _max[y] && _hit_point[z] >= _min[z] && _hit_point[z] <= _max[z])
							_hit = true, _low_t = _t;
					}
				}
			}
			_d = _low_t;
		}
		if (_distance)
			*_distance = _d;
		if (_point)
			*_point = _ray.Point(_d);
		return _hit;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Frustum
	//----------------------------------------------------------------------------//

	const uint16_t Frustum::Lines[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 3, 5, 2, 4, 1, 7, 0, 6, 4, 5, 5, 6, 6, 7, 7, 4 };
	const uint16_t Frustum::Quads[24] = { 4, 2, 3, 5, 7, 1, 2, 4, 7, 4, 5, 6, 5, 3, 0, 6, 6, 0, 1, 7, 2, 1, 0, 3 };

	//----------------------------------------------------------------------------//
	Frustum& Frustum::FromCameraMatrices(const Mat34& _view, const Mat44& _proj)
	{
		Mat44 _m = _proj * _view; // _view_proj
		planes[FP_Right].Set(_m(3, 0) - _m(0, 0), _m(3, 1) - _m(0, 1), _m(3, 2) - _m(0, 2), _m(3, 3) - _m(0, 3)).Normalize();
		planes[FP_Left].Set(_m(3, 0) + _m(0, 0), _m(3, 1) + _m(0, 1), _m(3, 2) + _m(0, 2), _m(3, 3) + _m(0, 3)).Normalize();
		planes[FP_Top].Set(_m(3, 0) - _m(1, 0), _m(3, 1) - _m(1, 1), _m(3, 2) - _m(1, 2), _m(3, 3) - _m(1, 3)).Normalize();
		planes[FP_Bottom].Set(_m(3, 0) + _m(1, 0), _m(3, 1) + _m(1, 1), _m(3, 2) + _m(1, 2), _m(3, 3) + _m(1, 3)).Normalize();
		planes[FP_Near].Set(_m(3, 0) + _m(2, 0), _m(3, 1) + _m(2, 1), _m(3, 2) + _m(2, 2), _m(3, 3) + _m(2, 3)).Normalize();
		planes[FP_Far].Set(_m(3, 0) - _m(2, 0), _m(3, 1) - _m(2, 1), _m(3, 2) - _m(2, 2), _m(3, 3) - _m(2, 3)).Normalize();
		origin = Vec3::Zero * _view.Copy().Inverse();
		_m.Inverse(); // _inv_view_proj
		corners[BC_LeftBottomFar] = Vec3(-1, -1, -1) * _m;
		corners[BC_RightBottomFar] = Vec3(+1, -1, -1) * _m;
		corners[BC_RightBottomNear] = Vec3(+1, -1, +1) * _m;
		corners[BC_LeftBottomNear] = Vec3(-1, -1, +1) * _m;
		corners[BC_RightTopNear] = Vec3(+1, +1, +1) * _m;
		corners[BC_LeftTopNear] = Vec3(-1, +1, +1) * _m;
		corners[BC_LeftTopFar] = Vec3(-1, +1, -1) * _m;
		corners[BC_RightTopFar] = Vec3(+1, +1, -1) * _m;
		box.Reset().AddVertices(corners, 8);
		return *this;
	}
	//----------------------------------------------------------------------------//
	bool Frustum::Intersects(const Vec3& _point) const
	{
		return planes[FP_Near].Distance(_point) >= 0 &&
			planes[FP_Far].Distance(_point) >= 0 &&
			planes[FP_Left].Distance(_point) >= 0 &&
			planes[FP_Right].Distance(_point) >= 0 &&
			planes[FP_Top].Distance(_point) >= 0 &&
			planes[FP_Bottom].Distance(_point) >= 0;
	}
	//----------------------------------------------------------------------------//
	bool Frustum::Intersects(const Vec3& _center, float _radius) const
	{
		return planes[FP_Near].Distance(_center, _radius) >= 0 &&
			planes[FP_Far].Distance(_center, _radius) >= 0 &&
			planes[FP_Left].Distance(_center, _radius) >= 0 &&
			planes[FP_Right].Distance(_center, _radius) >= 0 &&
			planes[FP_Top].Distance(_center, _radius) >= 0 &&
			planes[FP_Bottom].Distance(_center, _radius) >= 0;
	}
	//----------------------------------------------------------------------------//
	bool Frustum::Intersects(const AlignedBox& _box, bool* _contains) const
	{
		if (box.Intersects(_box) && Intersects(_box.Center(), _box.Radius()))
		{
			if (_contains)
			{
				*_contains = true;
				Vec3 _corners[8];
				_box.GetAllCorners(_corners);
				for (unsigned int i = 0; i < 8; ++i) if (!Intersects(_corners[i]))
				{
					*_contains = false;
					break;
				}
			}
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool Frustum::Intersects(const Frustum& _frustum, bool* _contains) const
	{
		if (box.Intersects(_frustum.box))
		{
			for (unsigned int i = 0; i < 6; ++i)
			{
				int _out = 0;
				for (unsigned int j = 0; j < 8; ++j)
					if (planes[i].Distance(_frustum.corners[j]) < 0)
						++_out;
				if (_out == 8)
					return false;
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
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
