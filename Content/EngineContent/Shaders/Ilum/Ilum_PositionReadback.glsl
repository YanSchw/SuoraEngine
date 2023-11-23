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

uniform sampler2D u_BaseColor;
uniform sampler2D u_Metallic;
uniform sampler2D u_Roughness;
uniform sampler2D u_WorldPosition;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_Emissive;
uniform sampler2D u_SurfelPositions;
uniform vec2 u_Hotspot[160];
uniform int u_Seed;
uniform bool u_ScreenPass;
uniform mat4 u_ViewProjection;

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
	if (texture(u_WorldPosition, UV).xyz == vec3(0.0)) return;
	//if (rand(vec2(u_Seed) + UV) < 0.95) return;

	vec3 worldPos = texture(u_WorldPosition, UV).xyz;
	
	int Index1 = (int(rand(vec2(u_Seed) - UV * UV) * 5458793.0) + int(rand(vec2(u_Seed) * UV + UV) * 8954371.0)) % (u_ScreenPass ? 128 : 32) + (u_ScreenPass ? 0 : 128);
	int Index2 = (int(rand(vec2(u_Seed) + UV * UV) * 2085341.0) + int(rand(vec2(u_Seed) * UV - UV) * 9163745.0)) % 4096;
	
	if (distance(UV, u_Hotspot[Index1]) > 0.028) return;

	int x = Index2 % 64; int y = Index2 / 64;
	vec4 screenPos = u_ViewProjection * vec4(texture(u_SurfelPositions, vec2(float(x)/64.0, float(y)/64.0)).xyz, 1.0);
	vec2 screenPos2D = vec2(screenPos.x, screenPos.y) / screenPos.w;
	if (screenPos2D.x < -1.0 || screenPos2D.x > 1.0 || screenPos2D.y < -1.0 || screenPos2D.y > 1.0)
	{
	ssbo[Index1].SurfelIndex = -1;
	discard;
	return;
	}

	ssbo[Index1].SurfelIndex = Index2;
	ssbo[Index1].BaseColor = texture(u_BaseColor, UV).xyz;
	ssbo[Index1].Metallic = texture(u_Metallic, UV).r;
	ssbo[Index1].Roughness = texture(u_Roughness, UV).r;
	ssbo[Index1].WorldPosition = worldPos;
	ssbo[Index1].WorldNormal = texture(u_WorldNormal, UV).xyz;
	ssbo[Index1].Emissive = texture(u_Emissive, UV).xyz;

}