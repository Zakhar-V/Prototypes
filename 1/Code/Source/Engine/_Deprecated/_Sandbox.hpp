#pragma once

#include "Graphics.hpp"

typedef struct btDbvtNode DbvTreeNodeImpl;
typedef struct btDbvt DbvTreeImpl;

namespace ge
{
	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// SPATIAL
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	//----------------------------------------------------------------------------//
	// DbvTree
	//----------------------------------------------------------------------------//

	typedef DbvTreeNodeImpl DbvTreeNode;

	class DbvTree : public NonCopyable
	{
	public:
		typedef bool(*EnumCallback)(void* _object, bool _contains, void* _ud);
		typedef bool(*RayCallback)(void* _object, const Ray& _ray, float _dist, void* _ud);

		DbvTree(void);
		~DbvTree(void);

		DbvTreeNode* Create(void* _object, const AlignedBox& _bv);
		void Delete(DbvTreeNode* _node);
		void Update(DbvTreeNode** _node, void* _object, const AlignedBox& _bv);
		void Optimize(void);
		void EnumObjects(EnumCallback _callback, void* _ud, const AlignedBox& _bv);
		void EnumObjects(EnumCallback _callback, void* _ud, const Sphere& _bv);
		void EnumObjects(EnumCallback _callback, void* _ud, const Frustum& _bv);
		void EnumObjects(EnumCallback _callback, void* _ud, const Frustum& _bv, const Frustum& _bv2);
		//void RayCast(RayCallback& _callback, const Ray& _ray);

	protected:
		DbvTreeImpl* m_dbvt;
		uint m_updates;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// RESOURCES
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////




	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// ENTITY
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class Entity;
	class EntityClass;


	class Entity
	{

	};


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	class ActorCollision;
	class ActorDynamics;
	class ActorController;


	typedef btDbvtNode DbvTreeNode;


	class Actor
	{
	public:
		Actor(void);
		virtual ~Actor(void);



	protected:
		


		Actor* m_parentActor;
		
		Actor* m_prevActor;
		Actor* m_nextActor;
		Actor* m_childActor;
		Vec3 m_position;
		Quat m_rotation;
		Vec3 m_scale;
		Mat34 m_matrix;
		ActorCollision* m_collision;
		ActorDynamics* m_dynamics;
		AlignedBox m_bounds;
		DbvTreeNode* m_dbvTreeNode;

		void* m_controllers;
		
		union
		{
			struct
			{

			};
			uint8 m_actorFlags;
		};

		union
		{
			struct
			{

			};
			uint8 m_actorState;
		};
	};


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// PHYSICS
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class CollisionShape
	{

	};

	class ActorCollision
	{

	};

	class ActorDynamics
	{

	};

	class RigidBody : public ActorDynamics
	{

	};

	class SoftBody : public ActorDynamics
	{
	public:

	};

	class PhysicsJoint
	{

	};

	class PhysicsWorld
	{

	};

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// graphics
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class Camera;
	class Mesh;
	class ParticleSystem;
	class Skeleton;
	class Bone;

	enum LightType : uint8
	{
		LT_POINT,
		LT_SPOT,
		LT_DIRECTIONAL,
	};
		 

	class LightSource : public Actor
	{

	};

	class MeshActor : public Actor
	{

	};

	class Skeleton : public Actor
	{

	};

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////
}
