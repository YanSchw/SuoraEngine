#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;

void main(void)
{
	gl_Position = vec4(a_Position, 1.0);
	UV = a_TexCoord;
}

#type fragment
#version 430 core

in vec2 UV;
/*
uniform sampler2D u_BaseColor;
uniform sampler2D u_Metallic;
uniform sampler2D u_Roughness;
uniform sampler2D u_WorldPosition;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_Emissive;*/
uniform int u_Seed;

layout(location = 0) out vec3 out_BaseColor;
layout(location = 1) out float out_Metallic;
layout(location = 2) out float out_Roughness;
layout(location = 3) out vec3 out_WorldPos;
layout(location = 4) out vec3 out_WorldNormal;
layout(location = 5) out vec3 out_Emissive;
layout(location = 6) out int out_MeshID;
layout(location = 7) out int out_Cluster;

struct PosReadbackBufferStruct
{
	int SurfelIndex;
	vec3 BaseColor;
	float Metallic;
	float Roughness;
	vec3 WorldPosition;
	vec3 WorldNormal;
	vec3 Emissive;
};
layout(std430, binding = 0) buffer ssbo__
{
	PosReadbackBufferStruct ssbo[160];
};

float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void)
{
	int Index = int(UV.x * 64.0) + int(UV.y * 63.0) * 64;
	for (int i = 0; i < 160; i++)
	{
		if (ssbo[i].SurfelIndex == Index)
		{
			out_BaseColor		= ssbo[i].BaseColor;
			out_Metallic		= ssbo[i].Metallic;
			out_Roughness		= ssbo[i].Roughness;
			out_WorldPos		= ssbo[i].WorldPosition;
			out_WorldNormal		= ssbo[i].WorldNormal;
			out_Emissive		= ssbo[i].Emissive;
			return;
		}
	}
	discard;
}