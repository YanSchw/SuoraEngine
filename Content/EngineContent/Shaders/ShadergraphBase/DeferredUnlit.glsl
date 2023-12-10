#type vertex
#version 330 core

$DEFERRED

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Normal;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;
out vec2 UV;
out vec4 frag_Color;
out vec3 worldPos;
out vec3 worldNormal;
out mat3 TBN;

uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;
uniform mat4 u_NormalMatrix;

//$VERT_FUNCTIONS
$VERT_INPUTS

void main(void)
{
    vec3 worldPositionOffset = $VERT_INPUT("World Position Offset", vec3, { vec3(0.0) });
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position + worldPositionOffset, 1.0);
	
	frag_Color = a_Color;
	UV = a_TexCoord;
	worldPos = vec3(u_Transform * vec4(a_Position + worldPositionOffset, 1.0));
	worldNormal = vec3(u_Transform * vec4(a_Normal, 0.0));

	vec3 T = normalize(vec3(u_NormalMatrix * vec4(normalize(a_Tangent), 0.0)));
	vec3 B = normalize(vec3(u_NormalMatrix * vec4(normalize(a_Bitangent), 0.0)));
	vec3 N = normalize(vec3(u_NormalMatrix * vec4(normalize(a_Normal), 0.0)));
	TBN = mat3(T, B, N);
}

#type fragment
#version 330 core

in vec4 frag_Color;
in vec2 UV;
in vec3 worldPos;
in vec3 worldNormal;
in mat3 TBN;
// Output
layout(location = 0) out vec3 out_BaseColor;
layout(location = 1) out float out_Metallic;
layout(location = 2) out float out_Roughness;
layout(location = 3) out vec3 out_WorldPos;
layout(location = 4) out vec3 out_WorldNormal;
layout(location = 5) out vec3 out_Emissive;

//$FRAG_FUNCTIONS
$FRAG_INPUTS

vec3 SampleNormal(vec3 n)
{
	n = normalize(n * 2.0 - 1.0); 
	n = normalize(TBN * n); 
	return n;
}

void main(void)
{
	vec3 baseColor = $FRAG_INPUT("Base Color", vec3, { vec3(1.0) });
	float opacity = $FRAG_INPUT("Opacity", float, { 1.0 });

	if (opacity <= 0.5) 
	{
		discard;
	}

	vec3 normal = $FRAG_INPUT("Normal", vec3, { normalize(worldNormal) });

	out_BaseColor = vec3(0.0);
	out_Metallic = 0.0;
	out_Roughness = 1.0;
	out_WorldPos = worldPos;
	out_WorldNormal = normalize(normal);
	out_Emissive = baseColor;
}