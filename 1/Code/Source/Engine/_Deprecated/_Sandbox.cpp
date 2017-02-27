#include "Sandbox.hpp"
#include "Internal.hpp"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/BroadphaseCollision/btDbvt.h>

namespace ge
{
	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// SPATIAL
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	//----------------------------------------------------------------------------//
	// DbvTree utils
	//----------------------------------------------------------------------------//

	namespace
	{
		template <class T> struct CompoundFrustum
		{
			CompoundFrustum(const Frustum& _camera, const T& _light) : camera(_camera), light(_light) { }
			const Frustum& camera;
			const T& light;
		};

		bool _Intersects(btDbvtAabbMm& _node, const AlignedBox& _bv, bool& _contains)
		{
			AlignedBox _aabb(*(const Vec3*)(&_node.Mins()), *(const Vec3*)(&_node.Maxs()));
			return _bv.Intersects(_aabb, &_contains);
		}

		bool _Intersects(btDbvtAabbMm& _node, const Sphere& _bv, bool& _contains)
		{
			AlignedBox _aabb(*(const Vec3*)(&_node.Mins()), *(const Vec3*)(&_node.Maxs()));
			if (_bv.Intersects(_aabb.Sphere()))
			{
				Vec3 _corners[8];
				uint _out = 8;
				_aabb.GetAllCorners(_corners);
				for (uint i = 0; i < 8 && _bv.Contains(_corners[i]); ++i) --_out;
				_contains = _out == 0;
				return true;
			}
			return false;
		}

		bool _Intersects(btDbvtAabbMm& _node, const Frustum& _bv, bool& _contains)
		{
			AlignedBox _aabb(*(const Vec3*)(&_node.Mins()), *(const Vec3*)(&_node.Maxs()));
			return _bv.Intersects(_aabb, &_contains);
		}

		bool _Intersects(btDbvtAabbMm& _node, const CompoundFrustum<Frustum>& _bv, bool& _contains)
		{
			AlignedBox _aabb(*(const Vec3*)(&_node.Mins()), *(const Vec3*)(&_node.Maxs()));
			bool _contains2;
			bool _intersects = _bv.camera.Intersects(_aabb, &_contains) && _bv.light.Intersects(_aabb, &_contains2);
			_contains &= _contains2;
			return _intersects;
		}

		template <uint SIZE, typename BV, typename CB> void _EnumObjects(btDbvt* _dbvt, CB _callback, void* _ud, const BV& _bv)
		{
			assert(_callback != 0);
			assert(btDbvt::maxdepth(_dbvt->m_root) < SIZE);
			btDbvtNode* _storage[SIZE];
			btDbvtNode** _stack = _storage;
			uint _depth = 0;
			_stack[_depth++] = _dbvt->m_root;
			do
			{
				btDbvtNode* _node = _stack[--_depth];
				bool _contains;
				if (_Intersects(_node->volume, _bv, _contains))
				{
					if (_contains)
					{
						if (_node->isleaf())
						{
							_callback(_node->data, true, _ud);
						}
						else
						{
							uint _depth2 = _depth;
							_stack[_depth2++] = _node;
							do
							{
								_node = _stack[--_depth];
								if (_node->isinternal())
								{
									_stack[_depth2++] = _node->childs[0];
									_stack[_depth2++] = _node->childs[1];
								}
								else
								{
									_callback(_node->data, true, _ud);
								}

							} while (_depth2 > _depth);

						}
					}
					else
					{
						if (_node->isinternal())
						{
							_stack[_depth++] = _node->childs[0];
							_stack[_depth++] = _node->childs[1];
						}
						else
						{
							_callback(_node->data, false, _ud);
						}
					}
				}

			} while (_depth > 0);
		}

	}

	//----------------------------------------------------------------------------//
	// DbvTree
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	DbvTree::DbvTree(void) : 
		m_dbvt(new btDbvt),
		m_updates(0)
	{
	}
	//----------------------------------------------------------------------------//
	DbvTree::~DbvTree(void)
	{
		delete m_dbvt;
	}
	//----------------------------------------------------------------------------//
	DbvTreeNode* DbvTree::Create(void* _object, const AlignedBox& _bv)
	{
		if (_bv.IsFinite())
		{
			++m_updates;
			return m_dbvt->insert(btDbvtVolume::FromMM(_bv.mn.Cast<btVector3>(), _bv.mx.Cast<btVector3>()), _object);
		}
		return 0;
	}
	//----------------------------------------------------------------------------//
	void DbvTree::Delete(DbvTreeNode* _node)
	{
		if (_node)
		{
			++m_updates;
			m_dbvt->remove(_node);
		}
	}
	//----------------------------------------------------------------------------//
	void DbvTree::Update(DbvTreeNode** _node, void* _object, const AlignedBox& _bv)
	{
		ASSERT(_node != nullptr);
		bool _finite = _bv.IsFinite();
		if (*_node)
		{
			if (_finite)
			{
				++m_updates;
				m_dbvt->update(*_node, btDbvtVolume::FromMM(_bv.mn.Cast<btVector3>(), _bv.mx.Cast<btVector3>()));
			}
			else
			{
				++m_updates;
				m_dbvt->remove(*_node);
				*_node = nullptr;
			}
		}
		else if (_finite)
		{
			++m_updates;
			*_node = m_dbvt->insert(btDbvtVolume::FromMM(_bv.mn.Cast<btVector3>(), _bv.mx.Cast<btVector3>()), _object);
		}
	}
	//----------------------------------------------------------------------------//
	void DbvTree::Optimize(void)
	{
		// ???????
		if (m_updates > 0)
		{
			m_dbvt->optimizeIncremental(m_updates / 100);
			m_updates >>= 1;
		}
	}
	//----------------------------------------------------------------------------//
	void DbvTree::EnumObjects(EnumCallback _callback, void* _ud, const AlignedBox& _bv)
	{
		ASSERT(_callback != nullptr);
		// todo: нужно тестировать на больш»х сценах
		_EnumObjects<256>(m_dbvt, _callback, _ud, _bv);
	}
	//----------------------------------------------------------------------------//
	void DbvTree::EnumObjects(EnumCallback _callback, void* _ud, const Frustum& _bv)
	{
		ASSERT(_callback != nullptr);
		_EnumObjects<256>(m_dbvt, _callback, _ud, _bv);
	}
	//----------------------------------------------------------------------------//
	void DbvTree::EnumObjects(EnumCallback _callback, void* _ud, const Sphere& _bv)
	{
		ASSERT(_callback != nullptr);
		_EnumObjects<256>(m_dbvt, _callback, _ud, _bv);
	}
	//----------------------------------------------------------------------------//
	void DbvTree::EnumObjects(EnumCallback _callback, void* _ud, const Frustum& _bv, const Frustum& _bv2)
	{
		ASSERT(_callback != nullptr);
		_EnumObjects<256>(m_dbvt, _callback, _ud, CompoundFrustum<Frustum>(_bv, _bv2));
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////
}
