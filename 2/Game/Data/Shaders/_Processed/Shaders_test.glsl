#line 1 0 // Shaders/test.glsl

#ifndef COMPILE_VS
#error "this is vertex shader"
#endif

#line 1 1 // Shaders/VertexSemantic.glsl
// VERTEX SEMANTIC

#define VS_POSITION 0 
#define VS_NORMAL 1 
#define VS_TANGENT 2
#define VS_COLOR 3
#define VS_COLOR2 4
#define VS_WEIGHTS 5
#define VS_INDICES 6
#define VS_TEXCOORD 7
#define VS_TEXCOORD2 8
#define VS_TEXCOORD3 9
#define VS_TEXCOORD4 10
#define VS_LIGHTMAP 11
#define VS_AUX0 12
#define VS_AUX1 13
#define VS_AUX2 14
#define VS_AUX3 15
  

/*layout(location = VS_POSITION) in vec4 vPos;
layout(location = VS_NORMAL) in vec3 vNormal;
layout(location = VS_TANGENT) in vec4 vTangent;
layout(location = VS_COLOR) in vec4 vColor;
layout(location = VS_COLOR2) in vec4 vColor1;
layout(location = VS_WEIGHTS) in vec4 vWeights;
layout(location = VS_INDICES) in ivec4 vIndices;
layout(location = VS_TEXCOORD) in vec4 vTexCoord;
layout(location = VS_TEXCOORD2) in vec4 vTexCoord2;
layout(location = VS_TEXCOORD3) in vec4 vTexCoord3;
layout(location = VS_TEXCOORD4) in vec4 vTexCoord4;
layout(location = VS_LIGHTMAP) in vec4 vLightMap;*/

#line 6 0 // Shaders/test.glsl


layout (std140) uniform CameraParams
{
	mat4 viewProj;
	
} uCamera;

layout(location = VS_POSITION) in vec4 vPos;
layout(location = VS_NORMAL) in vec3 vNormal;
layout(location = VS_TANGENT) in vec4 vTangent;

#ifdef INSTANCE_
layout(location = VS_AUX0) in vec4 vInstanceMatrix0;
layout(location = VS_AUX1) in vec4 vInstanceMatrix1;
layout(location = VS_AUX2) in vec4 vInstanceMatrix2;

mat4 GetWorldMatrix()
{
	return mat4(
		vInstanceMatrix0,
		vInstanceMatrix0,
		vInstanceMatrix0,
		vec4(0, 0, 0, 1));
}
#else

layout (std140) uniform InstanceMatrices
{
	mat3x4 uWorldMatrices[256];
};

mat4 GetWorldMatrix()
{
	mat3x4 m = uWorldMatrices[gl_InstanceID];
	return mat4(m[0], m[1], m[2], vec4(0, 0, 0, 1));
	
}

#endif  

void main()
{
	gl_Position = uCamera.viewProj * GetWorldMatrix() * vPos;
}

