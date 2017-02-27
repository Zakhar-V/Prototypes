#pragma once

#include "Base.hpp"
#include "Resource.hpp"
#include "GraphicsDefs.hpp"

#ifdef MULTIRENDER
#include "GLGraphics.hpp"
//#include "D3D11Graphics.hpp"
//#include "D3D12Graphics.hpp"
//#include "VulkanGraphics.hpp"
#else
#ifdef USE_GL
#include "GLGraphics.hpp"
#elif defined(USE_D3D11)
#include "D3D11Graphics.hpp"
#endif
#endif

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gRenderSystem Engine::RenderSystem::Get()

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#define gTexturePool Engine::TexturePool::Get()

	class Texture : public Resource
	{
	public:

		static ResourcePtr Factory(void) { return new Texture; }

	protected:
	};

	class TexturePool : public Singleton<TexturePool>
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#define gVertexCache ge::VertexCache::Get()

	struct VertexCacheEntry
	{

	};

	class VertexCache : public Singleton<VertexCache>
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Shader
	{

	};

	class ShaderManager : public Singleton<ShaderManager>
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Effect
	{

	};

	class EffectManager : public Singleton<EffectManager>
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Material
	{

	};

	class MaterialManager : public Singleton<MaterialManager>
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HeightMap
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	enum : uint
	{
		MAX_TEXCOORDS = 8,
	};

	struct VertexTangentData
	{
		Vec4b normal;
		Vec4b tangent;
	};

	struct VertexSkinData
	{
		Vec4ub indices;
		Vec4ub weights;
	};

	class Mesh : public Mutex, public RCObject
	{
	public:

		template <class T> struct Data
		{
			SArray<T> data;
			bool cached = false;
			bool use = false;
		};
		struct TexCoordData
		{
			//VertexElementType type;
			bool cached = false;
			uint esize;
			SArray<uint8> data;
		};

		void SetNumVertices(uint _newSize)
		{
			if (m_numVertices != _newSize)
			{
				m_positionData.data.Resize(_newSize);
				m_positionData.cached = false;
				if (m_tangentData.use)
				{
					m_tangentData.data.Resize(_newSize);
					m_tangentData.cached = false;
				}
				if (m_tangentData.use)
				{
					m_tangentData.data.Resize(_newSize);
					m_tangentData.cached = false;
				}
				for (uint i = 0; i < m_numTexCoords; ++i)
				{
					TexCoordData& _tc = m_texCoords[i];
					_tc.data.Resize(_tc.esize * _newSize);
					_tc.cached = false;
				}
			}
		}

		void MarkDirty(void) { m_dirty = true; }
		bool IsDirty(void) { return m_dirty; }



	protected:
		uint m_numVertices;
		bool m_dirty;
		bool m_changed;
		bool m_resized;

		Data<Vec3> m_positionData;
		Data<VertexTangentData> m_tangentData;
		Data<VertexSkinData> m_skinData;
		TexCoordData m_texCoords[MAX_TEXCOORDS];
		uint m_numTexCoords;
	};

	class MeshManager : public Singleton<MeshManager>
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class BoneDesc
	{

	};

	class SkeletonDesc
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class RenderSystem : public Singleton<RenderSystem>
	{
	public:

		struct StartupParams
		{
			RendererType renderer; // r_RenderSystem = { 'GL' | 'GLES' | 'D3D11' | 'D3D12' | 'Vulkan' | }
			ShaderModel shaderModel = SM_Unknown; // r_ShaderModel = { 'SM4' | 'SM5' }
			RenderContextProfile profile = RCP_Core; // r_Profile = { 'Core' | 'Compatible' | 'Forward' }
			bool debugContext = false; // r_DebugContext

			void Serialize(Config& _cfg, bool _loading);
			void SetDefaults(void);
			void Validate(void);
		};
		static StartupParams& GetStartupParams(void) { return s_startupParams; }
		static StartupParams& GetCurrentStartupParams(void);

	protected:
		friend class System;

		RenderSystem(void);
		~RenderSystem(void);

		bool _Init(void);

		static int s_moduleRefCount;
		static StartupParams s_startupParams;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
