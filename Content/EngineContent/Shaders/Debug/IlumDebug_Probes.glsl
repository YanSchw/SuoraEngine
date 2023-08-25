#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 3) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;
out vec3 vPos;
flat out int vInstanceID;

uniform mat4 u_ViewProjection;
uniform vec3 u_LightGridPos;
uniform vec3 u_LightGridStep;

void main(void)
{

	ivec3 probeGridPos = ivec3 (gl_InstanceID / 16 / 16,
								gl_InstanceID / 16 % 16,
								gl_InstanceID % 16 % 16);
	vec3 probePos = (u_LightGridPos * u_LightGridStep) - (7.5 * u_LightGridStep) + (vec3(probeGridPos.x, probeGridPos.y, probeGridPos.z) * u_LightGridStep);
	gl_Position = u_ViewProjection * vec4(a_Position * 0.1 + probePos, 1.0);

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

uniform sampler2D u_IluminationCache;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
vec2 Remap(vec2 value, vec2 in1, vec2 in2, vec2 out1, vec2 out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}

void main(void)
{
	vec3 NORMAL = normalize(vPos);
	vec2 eqUV = SampleSphericalMap(NORMAL);
	vec2 coords = vec2(float(vInstanceID % 64), float(vInstanceID / 64));

	vec2 _uv  = Remap(eqUV, vec2(0.0), vec2(1.0), vec2(0.015625 * float(coords.x), 0.015625 * float(coords.y)), vec2(0.015625 * float(coords.x + 1.0), 0.015625 * float(coords.y + 1.0)));
	_uv = mix(_uv, vec2(0.015625 * float(coords.x + 0.5), 0.015625 * float(coords.y + 0.5)), 0.03);
	out_Color = vec4(texture(u_IluminationCache, _uv).rgb, 1.0);
}