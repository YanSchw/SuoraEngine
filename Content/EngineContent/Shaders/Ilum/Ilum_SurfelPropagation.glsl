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
uniform sampler2D u_Accumulator;
uniform sampler2D u_SurfelDirectLight;
uniform sampler2D u_SurfelWorldPos;
uniform sampler2D u_SurfelWorldNormal;
uniform int u_Seed;

out vec3 out_Irradiance;

float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}
vec2 Remap(vec2 value, vec2 in1, vec2 in2, vec2 out1, vec2 out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float DistFunc(float dist)
{
	return pow(2.718281828, dist * -0.1);
}
float DistUV(vec2 eqUV, vec3 N)
{
	vec2 uv = SampleSphericalMap(N);
	float dist = distance(uv, eqUV);
	float dist2 = distance(vec2(uv.x - 1.0, uv.y), eqUV); if (dist2 < dist) dist = dist2;
	float dist3 = distance(vec2(uv.x + 1.0, uv.y), eqUV); if (dist3 < dist) dist = dist3;
	float dist4 = distance(vec2(uv.x, uv.y - 1.0), eqUV); if (dist4 < dist) dist = dist4;
	float dist5 = distance(vec2(uv.x, uv.y + 1.0), eqUV); if (dist5 < dist) dist = dist5;
	
	return pow(2.718281828, dist * -5.0);
}
vec3 SampleOnce(vec3 probePos, int sampleCount, vec3 worldNormal)
{
	vec3 surfelRadiance;
	vec3 surfelPos = vec3(0.0);
	vec3 surfelNormal = vec3(0.0);
	float surfelDistance;

	int J = 0;
	while (surfelPos == vec3(0.0))
	{
		vec2 _uv = vec2(rand(vec2(u_Seed + J++) - probePos.xy), rand(vec2(u_Seed - J) - 2.5 * probePos.xz));

		surfelRadiance = texture(u_SurfelDirectLight, _uv).rgb;
		surfelPos = texture(u_SurfelWorldPos, _uv).xyz;
		surfelNormal = texture(u_SurfelWorldNormal, _uv).xyz;
		surfelDistance = distance(probePos, surfelPos);

		if (J >= sampleCount) return vec3(0.0);
	}

	for (int It = 0; It < sampleCount - J; It++)
	{
		vec2 _uv = vec2(rand(vec2(u_Seed - It) + probePos.yz), rand(vec2(u_Seed + 2 * It) + probePos.xz));
		vec3 newSurfelPos = texture(u_SurfelWorldPos, _uv).xyz;

		if (dot(normalize(surfelPos - probePos), normalize(newSurfelPos - probePos)) >= 0.75 && newSurfelPos != vec3(0.0))
		{
			float newSurfelDistance = distance(probePos, newSurfelPos);
			if (newSurfelDistance < surfelDistance)
			{
				surfelRadiance = texture(u_SurfelDirectLight, _uv).rgb;
				surfelNormal = texture(u_SurfelWorldNormal, _uv).xyz;
				surfelPos = newSurfelPos;
				surfelDistance = newSurfelDistance;
			}
		}
	}

	return surfelRadiance * DistFunc(surfelDistance) * max(0.0, dot(worldNormal, normalize(surfelPos - probePos)));
}

void main(void)
{
	vec3 irradiance = vec3(0.0);
	
	vec3 probePos = texture(u_SurfelWorldPos, UV).xyz;
	vec3 probeNormal = texture(u_SurfelWorldNormal, UV).xyz;

	for (int i = 0; i < 256; i++)
	irradiance += SampleOnce(probePos, 16, probeNormal);
	irradiance /= 244.0;


	out_Irradiance = mix(texture(u_Accumulator, UV).rgb, irradiance, 0.125);
}