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
out vec4 out_Color;

uniform sampler2D u_Texture;
uniform vec3 u_ViewPos;
uniform int u_Seed;

layout(std430, binding = 0) buffer ssbo
{
	vec3 Pos;
	vec3 other[63];
};

vec3 Lerp(vec3 a, vec3 b, float v)
{
	return a + ((b - a) * v);
}

float rand(vec2 co)
{
    return (fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;
}

void main(void)
{
	//if (rand(vec2(u_Seed)) < 0.1) return;

	vec3 pos = texture(u_Texture, UV).xyz;

	float dist = 1000.0;
	for (int i = 0; i < 63; i++)
	{
		float d = distance(pos, other[i]);
		if (d < dist) dist = d;
	}

	if (pos.x != 0.0 || pos.y != 0.0 || pos.z != 0.0)
	{
		if (dist >= 4.0 || rand(pos.xz) <= dist * 0.2)
		{
			Pos = pos;
		}
	}
}