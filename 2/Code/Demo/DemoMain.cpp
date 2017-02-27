#include <Engine.hpp>
#include <GL/glLoad.h>
#include <SDL2/include/SDL.h>
#include <Windows.h>

using namespace Engine;

namespace Engine
{

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	/*class ShaderSource : public Resource
	{
	public:

	protected:
	};

	class Shader : public RefCounted
	{
		static Shader* _Create(ShaderSource* _src, uint _defs);

		HashMap<uint, ShaderInstance*> m_instances;
	};

	class ShaderInstance : public RefCounted
	{

	};

	class ShaderProgram	: public RefCounted
	{
	public:

	protected:

		ShaderProgram(uint _uid, Shader* _vs, Shader* _fs, Shader* _gs, uint _outputAttribs)
		{

		}

		struct Stage
		{
			Shader* shader;
			ShaderInstance* instance;
			uint defs;
		};

		Stage m_stages[3];
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class HwShader
	{
	public:

		void Compile(const char* _src);
		bool Wait(void);

	protected:
	};

	class HwProgram
	{
		HwShader* m_shaders[3];
	};*/

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class Mesh : public Resource
	{

	};


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

}

#define PRINT_SIZEOF(T) printf("sizeof(%s) = %d\n", #T, sizeof(T));

class TestResource : public Resource
{
public:
	CLASS(TestResource);

	static ResourcePtr Factory(void)
	{
		return new TestResource;
	}
protected:

	/*virtual bool _Load(File& _f)
	{ 
		LOG_EVENT("Load TestResource '%s'", m_name.c_str());
		return true;
	}
	virtual void _Unload(void)
	{
		LOG_EVENT("Unload TestResource '%s'", m_name.c_str());
	}*/
};


int main(void)
{
	try
	{
		uint _num;



		PRINT_SIZEOF(Node);
		//PRINT_SIZEOF(Entity);
		printf("1KK nodes = %d mb\n", (sizeof(Node) * 1000000) / (1024 * 1024));
		System::CreateEngine();

		gFileSystem->SetRoot("../../");
		gFileSystem->SetWriteDir("Game");
		gFileSystem->AddPath("Engine");
		gFileSystem->AddPath("Game");
		gFileSystem->AddPath("Game/Data");


		gResourceCache->EnableTracking();
		gResourceCache->Register<TestResource>();
		gResourceCache->SetDeferredLoading(false);
		ShaderSourcePtr _r = gResourceCache->LoadResource<ShaderSource>("Shaders/test.glsl");
		gResourceCache->LoadQueuedResources();
		
		ShaderPtr _s = _r->CreateInstance(ST_Vertex);
		_r = nullptr;
		//_s = nullptr;
		//gResourceCache->RemoveUnusedResources();

		float _ft = 0;
		gDevice->SetVisible();
		while (gDevice->IsOpened())
		{
			gSystem->BeginFrame();
			gSystem->EndFrame();
			_ft += gSystem->FrameTime();
			if (_ft > 1000.0f)
			{
				_ft = 0;
				gRenderContext->ReloadShaders();
			}
		}

		System::DestroyEngine();
	}
	catch (std::exception _e)
	{
		LOG_ERROR("Fatal error: %s", _e.what());
	}

	//RenderContext _rc;
	/*
	Shader* _ss1 = _rc.LoadShader("test.glsl");
	if (_ss1)
	{
		_ss1->_ProcessSource();
		LOG_INFO("Shader log: \n%s\n", _ss1->GetLog().c_str());
	}
	*/ 
	/*for (;;)
	{
		Shader::_ReloadAll();
		SDL_Delay(1);
	}*/

	system("pause");
	return 0;
}
