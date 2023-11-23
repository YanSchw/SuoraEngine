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
#version 330 core

uniform sampler2D u_WorldPos;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_Iluminance;

// Surfels
uniform int u_SurfelCount;
uniform vec3 u_SurfelPos[501];
uniform vec3 u_SurfelNormal[501];
uniform int u_SurfelSlot[501];

in vec2 UV;
out vec4 out_Color;

void main(void)
{
	vec3 worldPos = texture(u_WorldPos, UV).xyz;
	vec3 worldNormal = texture(u_WorldNormal, UV).xyz;

	int surfel = 0;
	float surfelDist = distance(worldPos, u_SurfelPos[0]);

	for (int i = 1; i < u_SurfelCount; i++)
	{
		break;
		float dist = distance(worldPos, u_SurfelPos[i]);
		if (dist < surfelDist)
		{
			surfel = i;
			surfelDist = dist;
		}
	}

	int slot = u_SurfelSlot[surfel];
	int slotsPerRow = textureSize(u_Iluminance, 0).x;
	vec2 slotUV = vec2( (slot % slotsPerRow) / slotsPerRow ,
	                    (slot / slotsPerRow) / slotsPerRow );

	out_Color = texture(u_Iluminance, slotUV);
}