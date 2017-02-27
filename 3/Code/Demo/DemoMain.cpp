#include <Logger.hpp>
#include <Window.hpp>
#include <Graphics.hpp>
#include <Math.hpp>
#include <Scene.hpp>
#include <Render.hpp>
#include <System.hpp>
#include <File.hpp>
#include <Resource.hpp>
#include <timer.hpp>
#include <typeinfo>
#include <locale.h>
#include <Windows.h>

using namespace Engine;

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//

namespace Sandbox
{

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Entity;
	class Actor;

	enum ComponentEventType
	{
		CET_,
	};

	enum EnitiyEventType
	{
		EET_,
	};

	class Component : public RCObject
	{
	public:

	protected:
		Entity* m_entity;
		uint16 m_cFlags;
		uint16 m_cStateFlags;
	};

	class Actor : public Component
	{
	public:

	protected:
		Scene* m_scene;

		Actor* m_parent;
		Actor* m_prev;
		Actor* m_next;
		Actor* m_child;

		Vec3 m_position;
		Quat m_rotation;
		Vec3 m_scale;
		Mat34 m_matrix;

	};


	class Entity : public RCObject
	{
	public:


	protected:

		Scene* m_scene;
		Actor* m_actor;

		Entity* m_parent;
		Entity* m_prev;
		Entity* m_next;
		Entity* m_child;
	};


	class Transform : public Component
	{

	};

	class Bone : public Actor
	{
	public:

	protected:
	};

	class Skeleton : public Component
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class LightActor : public Actor
	{

	};

	class LightComponent : public Component
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class RenderPass
	{

	};

	class RenderBatch
	{

	};

	class RenderQueue
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//

namespace ge
{

}

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//

class TestResource : public Resource
{
public:

	CLASSNAME(TestResource);

	static ResourcePtr Factory(void) { return new TestResource; }

	bool _Load(DataStream& _src) override
	{ 
		String _str = _src.ReadString();
		printf("%s\n", *_str);
		return true;
	}


protected:
};


#define PRINT_SIZEOF(T) printf("sizeof(" #T ") = %d\n", sizeof(T))

class FileDirectoryCached
{
public:

protected:

	String m_name;
	String m_path;
};

class FileSearcher : public RCBase
{
public:

	///\param[in] _path specify the local path in root directory
	void AddDir(const String& _path, const char* _mask, bool _recursive)
	{

	}
	String SearchFile(const String& _name)
	{

	}
	DataStream& OpenFile(const String& _name)
	{

	}

	void Update()
	{

	}

protected:
};


//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//



/*struct GLCommand
{

	void Link(GLCommand*& _first, GLCommand*& _last)
	{
		next = _first;
		_first = this;
		if (next)
			next->prev = this;
		else
			_last = this;
	}
	void Unlink(GLCommand*& _first, GLCommand*& _last)
	{
		if (next)
			next->prev = next;
		else
			_last = prev;

		if (prev)
			prev->next = prev;
		else
			_first = next;

		prev = nullptr;
		next = nullptr;
	}

	GLCommand* prev = 0;
	GLCommand* next = 0;
};

struct GLCommandList
{
	void Push(GLCommand* _first, GLCommand* _last)
	{

	}
	GLCommand* first;
	GLCommand* last;
}; */

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//

/*struct TestCmd : public GLCommand
{
	GL_COMMAND_ALLOCATOR(TestCmd);
};
  */



int main(void)
{
	setlocale(LC_ALL, "Ru-ru");
	setlocale(LC_NUMERIC, "En-us");
	gLogger->SetWriteInfo(false);

	/*printf("%d\n", GLCommandPool<TestCmd>::Allocator::ElementSize);

	TestCmd* _t1 = new TestCmd;
	TestCmd* _t2 = new TestCmd;
	delete _t1;
	delete _t2;
	_t1 = new TestCmd;
	_t2 = new TestCmd;
			  */
	//system("pause");
	//return 0;

	PRINT_SIZEOF(Sandbox::Actor);


	PRINT_SIZEOF(Engine::Resource);
	PRINT_SIZEOF(Engine::Actor);

	//system("pause");
	//return 0;

	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	gFileSystem->AddSearchDir("../../Data/Core");

	ResourceCache::GetStartupParams().numThreads = 2;

	RenderSystem::GetStartupParams().shaderModel = SM4;
	RenderSystem::GetStartupParams().profile = RCP_Core;
	RenderSystem::GetStartupParams().debugContext = false;

	if (System::Create())
	{
		gResourceCache->RegisterType("TestResource", &TestResource::Factory, "Test", "txt");
		gResourceCache->LoadResource("TestResource", "test", true);
		gResourceCache->LoadResource("TestResource", "test.txt", true);
		gResourceCache->Load<TestResource>("Test/test.txt", true);

		gWindow->SetVisible();
		while (gWindow->IsOpened())
		{
			gWindow->PollEvents();
			//Thread::Pause(1);
			gRenderContext->BeginFrame();
			gRenderContext->EndFrame();

			//double _st = Timer::Us();
			//gRenderSystem->_SwapBuffers();
			//double _et = Timer::Us();
			//printf("time = %.3f us\n", _et - _st);
		}

		System::Destroy();
	}

	system("pause");
	return 0;
}
