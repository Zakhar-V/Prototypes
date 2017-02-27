#pragma once

#include "GraphicsCore.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//
	

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	enum ShaderFlags_
	{
		
		SF_Skinned,
		SF_LowQuality,
		SF_HighQuality,
		SF_
	};

	enum MaterialFlags
	{
		MF_
	};

	enum ShaderQuality
	{
		SQ_Low = 0,
		SQ_Normal,
		SQ_High,
	};

	enum ShaderInputType
	{
		SIT_StaticMesh,
		SIT_SkinnedMesh,
		SIT_

	};


	class Pass
	{
	public:

	protected:
	};

	class Technique
	{

	};

	class Effect
	{
		//m_vs_skinnedMesh;
		//m_vs_staticMesh;
		//m_vs_terrain;
		//m_vs_terrain;
		//ShaderStagePtr 
	};

	class Material
	{
	public:

	protected:

		Technique* m_technique;
		//Technique* m_technique;

	};

	//----------------------------------------------------------------------------//
	// ShaderManager
	//----------------------------------------------------------------------------//

	/*class ShaderManager : public Singleton<ShaderManager>
	{
	public:

	protected:
	};*/

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	class RenderSystem final : public Singleton<RenderSystem>
	{
	public:
		RenderSystem(void);
		~RenderSystem(void);

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
