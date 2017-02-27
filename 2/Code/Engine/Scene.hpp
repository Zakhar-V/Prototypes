#pragma once

#include "Core.hpp"

namespace Engine
{

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Scene;
	class Component;
	class Node;
	class Entity;

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	/*enum ComponentType
	{
		//CT_Render,
		//CT_Physics,
		CT_Logic,
		CT_Script,
		CT_AI,
		CT_Skeleton,
		CT_CharacterController,
		CT_MaxTypes,
	};*/

	class Component
	{
	public:

	protected:
		Node* m_node;
	};

	class NodeComponent
	{
		Node* m_node;
		NodeComponent* m_next;
	};

	//----------------------------------------------------------------------------//
	// Node
	//----------------------------------------------------------------------------//

	class Node : public RefCounted
	{
	public:
		Node(Scene* _scene);
		~Node(void);

		void SetParent(Node* _parent, bool _keepWorldPos = true);
		void RemoveFromScene(void);

	protected:

		void _Link(Node*& _head);
		void _Unlink(Node*& _head);

		void _OnParentRemoved(void);

		Scene* m_scene;
		Node* m_parent;
		//Node* m_prev;
		Node* m_next;
		Node* m_child;
		uint m_nodeFlags;
		Vec3 m_position;
		Quat m_rotation;
		Vec3 m_scale;
		Mat34 m_matrix;

		NodeComponent* m_components;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class LightNode : public Node
	{
	public:

	protected:

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class StaticMesh : public Node
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Bone : public Node
	{

	};

	class SkeletalMesh : public Node
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Scene
	{
	public:

	protected:

		Array<Node*> m_nodesToUpdate;
		Array<Node*> m_nodesToAdd;
		Array<Node*> m_nodesToRemove;
	};
}
