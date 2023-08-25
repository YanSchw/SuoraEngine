#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;
out vec2 PassUV;

$VERT_INPUTS

void main(void)
{
	gl_Position = vec4(a_Position, 1.0);
	PassUV = a_TexCoord;
}

#type fragment
#version 330 core

in vec2 PassUV;
layout(location = 0) out vec3 out_BaseColor;
layout(location = 1) out float out_Metallic;
layout(location = 2) out float out_Roughness;
layout(location = 3) out vec3 out_WorldPos;
layout(location = 4) out vec3 out_WorldNormal;
layout(location = 5) out vec3 out_Emissive;
layout(location = 6) out int out_MeshID;
layout(location = 7) out int out_Cluster;

uniform int u_DecalCount;
uniform mat4 u_DecalViewMatrix[128];

uniform sampler2D u_OG_WorldPos;

$FRAG_INPUTS

void main(void)
{
	int i = u_DecalCount - 1;
	vec3 ogWorldPos = texture(u_OG_WorldPos, PassUV).xyz;

	while (i >= 0)
	{
		vec4 fragPos = u_DecalViewMatrix[i] * vec4(ogWorldPos, 1.0);
		vec3 screenCoords = fragPos.xyz / fragPos.w;
		screenCoords = (screenCoords + 1.0) / 2.0;
		vec2 UV = screenCoords.xy;

		if (fragPos.w < 0.0 || UV.x < 0.0 || UV.x > 1.0 || UV.y < 0.0 || UV.y > 1.0) 
		{
			i--;
			continue;
		}

		vec3 baseColor = $FRAG_INPUT("Base Color", vec3, { vec3(1.0) });
		float opacity = $FRAG_INPUT("Opacity", float, { 1.0 });

		if (opacity <= 0.5) 
		{
			discard;
			return;
		}

		float metallic = $FRAG_INPUT("Metallic", float, { 0.0 });
		float roughness = $FRAG_INPUT("Roughness", float, { 0.5 });
		vec3 emissive = $FRAG_INPUT("Emissive Color", vec3, { vec3(0.0) });

		out_BaseColor = baseColor;
		out_Metallic = metallic;
		out_Roughness = roughness;
		out_Emissive = emissive;

		return;
	}

	discard;
	return;
	
}