#pragma once

#include "Base.hpp"
#include "Math.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	typedef Ptr<class Actor> ActorPtr;
	class Entity;
	class Scene;
	class PhysicsActor;

	//----------------------------------------------------------------------------//
	// Animator
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// Actor
	//----------------------------------------------------------------------------//

	class Actor	: public RCBase
	{
	public:

	protected:


	};

	//----------------------------------------------------------------------------//
	// Scene
	//----------------------------------------------------------------------------//

	class Scene : public NonCopyable
	{
	public:

		Scene(void);
		~Scene(void);

	protected:
		friend class Actor;

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
