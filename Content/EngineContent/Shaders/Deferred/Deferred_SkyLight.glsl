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

in vec2 UV;
out vec4 out_Color;

uniform sampler2D u_WorldPos;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_Emissive;
uniform vec3 u_ViewPos;

void main(void)
{
	vec3 light = vec3(0.00);

	vec3 worldPos = texture(u_WorldPos, UV).xyz;
	vec3 worldNormal = texture(u_WorldNormal, UV).xyz;
	vec3 emissive = texture(u_Emissive, UV).xyz;
    vec3 viewDir = normalize(u_ViewPos - worldPos);

    if (emissive != vec3(0.0))
    {
        light = vec3(0.0);
    }

	out_Color = vec4(light, 1.0);
	
}