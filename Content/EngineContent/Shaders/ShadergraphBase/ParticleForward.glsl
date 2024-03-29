#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;
out vec4 frag_Color;

struct Particle
{
	vec4 Position; // 16
	vec4 Scale; // 16
	vec4 Velocity;  // 16
	float LifeTime; // 4

	// Padding 
	uint padding_1;
	uint padding_2;
	uint padding_3;
};

layout(std430, binding = 0) readonly buffer ssbo
{
	Particle Particles[];
};

uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;

//$VERT_FUNCTIONS
$VERT_INPUTS

void main(void)
{
    vec3 worldPositionOffset = $VERT_INPUT("World Position Offset", vec3, { vec3(0.0) });
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position * Particles[gl_InstanceID].Scale.xyz + Particles[gl_InstanceID].Position.xyz + worldPositionOffset, 1.0);
	
	frag_Color = a_Color;
	UV = a_TexCoord;
}

#type fragment
#version 330 core

in vec4 frag_Color;
in vec2 UV;
out vec4 out_Color;

//$FRAG_FUNCTIONS
$FRAG_INPUTS

void main(void)
{
	out_Color = $FRAG_INPUT("Base Color", vec4, { vec4(1.0) });
	if (out_Color.a == 0.0) discard;
}