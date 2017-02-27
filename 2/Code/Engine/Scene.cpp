#include "Scene.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Node
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Node::Node(Scene* _scene) :
		m_scene(_scene),
		m_parent(nullptr),
		//m_prev(nullptr),
		m_next(nullptr),
		m_child(nullptr),
		m_nodeFlags(0),
		m_position(Vec3::Zero),
		m_rotation(Quat::Identity),
		m_scale(Vec3::One),
		m_matrix(Mat34::Identity),
		m_components(nullptr)
	{
		assert(_scene != nullptr);
	}
	//----------------------------------------------------------------------------//
	Node::~Node(void)
	{
		
	}
	//----------------------------------------------------------------------------//
	void Node::SetParent(Node* _parent, bool _keepWorldPos)
	{
		if (_parent == this || _parent == m_parent)
			return;

		if (_parent && _parent->m_scene != m_scene)
			return;

		if (m_parent)
		{
			for (Node* i = m_parent->m_child, *p = nullptr; ; p = i, i = i->m_next)
			{
				assert(i != nullptr);
				if (i == this)
				{
					if (p)
						p->m_next = m_next;
					else
						m_parent->m_child = m_next;
					break;
				}
			}
			m_next = nullptr;
		}

		m_parent = _parent;

		if (m_parent)
		{
			m_next = m_parent->m_child;
			m_parent->m_child = this;

			if (_keepWorldPos)
			{
				// update local transform
			}
			else
			{
				// update world matrix
			}
		}
		else
		{
			m_position = m_matrix.GetTranslation();
			m_rotation = m_matrix.GetRotation();
			m_scale = m_matrix.GetScale();
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Scene
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
