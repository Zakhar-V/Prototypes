
#ifndef COMPILE_VS
#error "this is vertex shader"
#endif

#include "Shaders/VertexSemantic.glsl"

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

