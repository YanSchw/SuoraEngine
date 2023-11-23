#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 3) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;
out vec4 vPos;
flat out int vInstanceID;

uniform mat4 u_ViewProjection;
uniform sampler2D u_SurfelWorldPosition;
uniform vec3 u_CameraPos;

float DistFuncVertex(float dist)
{
	const float a = 0.085; // 0.025; // 0.14
	const float S = 1.5;
	const float k = 0.1284;
	return (a * S) / (a + (S - a) * pow(2.718281828, k * -1.0 * dist * S));
}
void main(void)
{
	vec2 surfelUV = vec2(float(gl_InstanceID % 64) / 64.0, float(int(gl_InstanceID / 64)) / 64.0);
	vec3 surfelPos = texture(u_SurfelWorldPosition, surfelUV).xyz;
	gl_Position = u_ViewProjection * vec4(a_Position * DistFuncVertex(distance(u_CameraPos, surfelPos)) + surfelPos, 1.0);
	
	if (surfelPos == vec3(0.0))
	{
		gl_Position = vec4(0.0);
	}

	vPos = gl_Position;
	UV = a_TexCoord;
	vInstanceID = gl_InstanceID;
}

#type fragment
#version 430 core

flat in int vInstanceID;
in vec4 vPos;
in vec2 UV;
out vec4 out_Color;


layout (binding = 0, r32ui) uniform uimage2D output_buffer;

uniform sampler2D u_LightPropagation;
uniform sampler2D u_SurfelWorldPosition;
uniform sampler2D u_GBufferWorldPosition;
uniform sampler2D u_GBufferBaseColor;
uniform vec3 u_CameraPos;

float DistFuncVertex(float dist)
{
	const float a = 0.125; // 0.025; // 0.14
	const float S = 1.5;
	const float k = 0.1284;
	return (a * S) / (a + (S - a) * pow(2.718281828, k * -1.0 * dist * S));
}

float DistFunc(float dist)
{
	return pow(2.718281828, dist * -11.1);
}

void main(void)
{
	//out_Color = vec4(1.0, 0.0, 0.0, 1.0); return;
	vec2 surfelUV = vec2(float(vInstanceID % 64) / 64.0, float(int(vInstanceID / 64)) / 64.0);
		
	vec3 ndc = vPos.xyz / vPos.w; //perspective divide/normalize
	vec2 screenCoords = ndc.xy * 0.5 + 0.5; //ndc is -1 to 1 in GL. scale for 0 to 1
	
	vec3 gBufferWorldPos = texture(u_GBufferWorldPosition, screenCoords).xyz;
	vec3 surfelWorldPos = texture(u_SurfelWorldPosition, surfelUV).xyz;

	if (distance(gBufferWorldPos, surfelWorldPos) > DistFuncVertex(distance(u_CameraPos, surfelWorldPos))) discard;


	float lightConstant = DistFuncVertex(distance(u_CameraPos, surfelWorldPos));
	float lightLinear = 1.0;
	float lightQuadratic = 1.0;
	float dist = distance(gBufferWorldPos, surfelWorldPos);
	float attenuation = lightConstant / (lightConstant + lightLinear * dist + lightQuadratic * (dist * dist));


    float opacity = clamp(attenuation * 1.5, 0.0, 1.0);//clamp(DistFunc(distance(gBufferWorldPos, surfelWorldPos)) - 0.25, 0.0, 1.0);

	/** Screendoor Transparency */
	mat4 threshold = mat4
    (
		1.0 / 17.0,   9.0 / 17.0,   3.0 / 17.0,   11.0 / 17.0,
		13.0 / 17.0,  5.0 / 17.0,   15.0 / 17.0,  7.0 / 17.0,
		4.0 / 17.0,   12.0 / 17.0,  2.0 / 17.0,   10.0 / 17.0,
		16.0 / 17.0,  8.0 / 17.0,   14.0 / 17.0,  6.0 / 17.0
    );
	int x = int(gl_FragCoord.x - 0.5);
    int y = int(gl_FragCoord.y - 0.5);
    if (pow(opacity, 3.0) < threshold[(x + vInstanceID) % 4][(y + vInstanceID )% 4])
		discard;

	out_Color = vec4(texture(u_LightPropagation, surfelUV).xyz * texture(u_GBufferBaseColor, screenCoords).xyz /** opacity*/ * 2.5, 0.1);
}