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

// Source: https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/pbr_fs.glsl

#define PI 3.1415926538
const float Epsilon = 0.00001;
const vec3 Fdielectric = vec3(0.04);

in vec2 UV;
out vec4 out_Color;

uniform vec3 u_View;

uniform sampler2D u_BaseColor;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_WorldPosition;
uniform sampler2D u_Metallic;
uniform sampler2D u_Roughness;
uniform sampler2D u_Radiance;
uniform sampler2D u_Specular;
uniform sampler2D u_Fresnel;
uniform sampler2D u_IrradianceMap;
uniform sampler2D u_PrefilterMap;
uniform sampler2D u_BrdfLUT;


float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float Remap(float value, float in1, float in2, float out1, float out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}
vec2 RemapVec2(vec2 value, vec2 in1, vec2 in2, vec2 out1, vec2 out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}

// Describes the ratio of surface reflection at different surface angles.
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 sampleHDRI(sampler2D texture, vec3 dir, float lod)
{
	return textureLod(texture, SampleSphericalMap(dir), lod).rgb;
}
float max3 (vec3 v) 
{
  return max (max (v.x, v.y), v.z);
}

uniform sampler2D u_IlumCache;
uniform int u_IlumSeed;
uniform vec3 u_LightGridPos;
uniform vec3 u_LightGridStep;

vec4 Ilum(vec3 worldPos, vec3 worldNormal)
{
	vec3 irradiance = vec3(0.0);
	
	vec3 ProbePos[4];
	float ProbeDist[4];
	for (int i = 0; i < 4; i++)
	{
		ProbePos[i] = vec3(0.0);
		ProbeDist[i] = 999999.9 + float(i);
	}

	vec3 nextProbe = vec3(0.0, 0.0, 0.0);
	for (int x = 0; x < 15; x++)
	{
		nextProbe.x = float(x);
		float probeWorldPos = (u_LightGridPos.x * u_LightGridStep.x) - (7.5 * u_LightGridStep.x) + (float(x) * u_LightGridStep.x);
		if (worldPos.x < probeWorldPos) break;
	}
	for (int y = 0; y < 15; y++)
	{
		nextProbe.y = float(y);
		float probeWorldPos = (u_LightGridPos.y * u_LightGridStep.y) - (7.5 * u_LightGridStep.y) + (float(y) * u_LightGridStep.y);
		if (worldPos.y < probeWorldPos) break;
	}
	for (int z = 0; z < 15; z++)
	{
		nextProbe.z = float(z);
		float probeWorldPos = (u_LightGridPos.z * u_LightGridStep.z) - (7.5 * u_LightGridStep.z) + (float(z) * u_LightGridStep.z);
		if (worldPos.z < probeWorldPos) break;
	}

	bool bDone = false;
	while (!bDone)
	{
		bDone = true;
		for (int x = int(max(nextProbe.x - 1.0, 0.0)); x < int(min(nextProbe.x + 1.0, 15.0)); x++)
		{
			for (int y = int(max(nextProbe.y - 1.0, 0.0)); y < int(min(nextProbe.y + 1.0, 15.0)); y++)
			{
				for (int z = int(max(nextProbe.z - 1.0, 0.0)); z < int(min(nextProbe.z + 1.0, 15.0)); z++)
				{
					vec3 probeWorldPos = (u_LightGridPos * u_LightGridStep) - (7.5 * u_LightGridStep) + (vec3(float(x), float(y), float(z)) * u_LightGridStep);
					float dist = distance(worldPos, probeWorldPos);
					if (dist < ProbeDist[3])
					{
						bDone = false;
						ProbeDist[3] = dist;
						ProbePos[3] = vec3(x, y, z);

						// Sort
						bool _done = false;
						while (!_done)
						{
							_done = true;
							for (int i = 1; i < 4; i++)
							{
								if (ProbeDist[i - 1] > ProbeDist[i])
								{
									_done = false;
									vec3 tempPos = ProbePos[i];			ProbePos[i] = ProbePos[i - 1];			ProbePos[i - 1] = tempPos;
									float tempDist = ProbeDist[i];		ProbeDist[i] = ProbeDist[i - 1];		ProbeDist[i - 1] = tempDist;
								}
							}
						}
					}
				}
			}
		}
		nextProbe = ProbePos[0];
	}
	
	vec2 eqUV = SampleSphericalMap(worldNormal);
	for (int i = 0; i < 4; i++)
	{
		float factor = (i == 0 ? 0.666 : (i == 1 ? 0.222 : (i == 2 ? 0.074 : 0.037)));
		int probeID = int(ProbePos[i].z) + int(ProbePos[i].y) * 16 + int(ProbePos[i].x) * 256;
		vec2 coords = vec2(float(probeID % 64), float(probeID / 64));
		vec2 _uv  = RemapVec2(eqUV, vec2(0.0), vec2(1.0), vec2(0.015625 * float(coords.x), 0.015625 * float(coords.y)), vec2(0.015625 * float(coords.x + 1.0), 0.015625 * float(coords.y + 1.0)));
		//_uv = mix(_uv, vec2(0.015625 * float(coords.x + 0.5), 0.015625 * float(coords.y + 0.5)), 0.03);
		irradiance += texture(u_IlumCache, _uv).rgb * factor;
	}

	return vec4(irradiance, 1.0);
}

vec3 MinVec3(vec3 in_)
{
	return vec3(max(0.0, in_.x), max(0.0, in_.y), max(0.0, in_.z));
}

void main(void)
{
    // Sample input textures to get shading model params.
	vec3 albedo = texture(u_BaseColor, UV).rgb;
	float metalness = texture(u_Metallic, UV).r;
	float roughness = texture(u_Roughness, UV).r;
	vec3 N = texture(u_WorldNormal, UV).rgb;
	vec3 worldPos = texture(u_WorldPosition, UV).rgb;

	// Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(u_View - worldPos);
	
	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
	vec3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(Fdielectric, albedo, metalness);




	
	// Light
	vec3 directLighting = MinVec3(texture(u_Radiance, UV).rgb);

	// Ambient
	/*vec3 irradiance = sampleHDRI(u_IrradianceMap, normal, 7);
    irradiance = irradiance / max3(irradiance) / PI; // 
    irradiance = vec3(0.1); // 
    vec3 diffuse = albedo * irradiance;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = sampleHDRI(u_PrefilterMap, reflect(u_View, normal), roughness * MAX_REFLECTION_LOD);
    vec2 brdf = texture(u_BrdfLUT, vec2(max(dot(normal, u_View), 0.0), roughness)).rg;
    vec3 reflection = prefilteredColor * (kS * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + reflection * vec3(1.0 - roughness)); // 1.5 factor to increase reflection contribution for showcase
    
	vec3 fragmentColor = ambient + Lo;
	out_Color = vec4(fragmentColor, 1.0);
*/

    // Ambient lighting (IBL).
	vec3 ambientLighting;
	{
		// Sample diffuse irradiance at normal direction.
		vec3 irradiance = vec3(0.1);//Ilum(worldPos, N).rgb * 5.0; // texture(irradianceTexture, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		vec3 F = fresnelSchlick(cosLo, F0);

		// Get diffuse contribution factor (as with direct lighting).
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		vec3 diffuseIBL = kd * albedo * irradiance + (vec3(0.3) * albedo);

		// Sample pre-filtered specular reflection environment at correct mipmap level.
		int specularTextureLevels = textureQueryLevels(u_PrefilterMap);
		vec3 ReflectionNormal = N; //reflect(u_View, N); // N;
		vec3 specularIrradiance = sampleHDRI(u_PrefilterMap, normalize(ReflectionNormal + 15.0 * N), roughness * specularTextureLevels).rgb * vec3(1.0 - roughness);

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		vec2 specularBRDF = texture(u_BrdfLUT, vec2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
		ambientLighting = MinVec3(diffuseIBL) + MinVec3(specularIBL);
	}
    

	// Final fragment color.
	out_Color = vec4(directLighting + ambientLighting, 1.0);
}