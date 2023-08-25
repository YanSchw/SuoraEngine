#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 3) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;
out vec3 vPos;
flat out int vInstanceID;

uniform mat4 u_ViewProjection;
uniform sampler2D u_WorldPosition;
uniform sampler2D u_WorldNormal;

void main(void)
{
	vec2 surfelUV = vec2(float(gl_InstanceID % 64) / 64.0, float(int(gl_InstanceID / 64)) / 64.0);
	
	vec3 worldPosition = texture(u_WorldPosition, surfelUV).xyz;
	vec3 worldNormal = texture(u_WorldNormal, surfelUV).xyz;
	vec3 surfelDiskPos = normalize(a_Position);


	// Source: https://stackoverflow.com/questions/9605556/how-to-project-a-point-onto-a-plane-in-3d
	float dist = dot(surfelDiskPos, worldNormal);
	vec3 projectedPos = surfelDiskPos - dist * worldNormal;

	gl_Position = u_ViewProjection * vec4(projectedPos * 0.25 + worldPosition, 1.0);
	
	if (texture(u_WorldPosition, surfelUV).xyz == vec3(0.0))
	{
		gl_Position = vec4(0.0);
	}

	vPos = a_Position;
	UV = a_TexCoord;
	vInstanceID = gl_InstanceID;
}

#type fragment
#version 330 core

flat in int vInstanceID;
in vec3 vPos;
in vec2 UV;
out vec4 out_Color;

uniform sampler2D u_BaseColor;

void main(void)
{
	vec2 surfelUV = vec2(float(vInstanceID % 64) / 64.0, float(int(vInstanceID / 64)) / 64.0);
		
	out_Color = vec4(texture(u_BaseColor, surfelUV).xyz, 1.0);
}