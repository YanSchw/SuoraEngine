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
uniform vec4 u_ForwardClearColor;

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

    // Ambient lighting (IBL).
	vec3 ambientLighting;
	{
		// Sample diffuse irradiance at normal direction.
		vec3 irradiance = vec3(0.0); // texture(irradianceTexture, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		vec3 F = fresnelSchlick(cosLo, F0);

		// Get diffuse contribution factor (as with direct lighting).
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		vec3 diffuseIBL = kd * albedo * irradiance;

		// Sample pre-filtered specular reflection environment at correct mipmap level.
		int specularTextureLevels = textureQueryLevels(u_PrefilterMap);
		vec3 ReflectionNormal = N; //reflect(u_View, N);
		vec3 specularIrradiance = sampleHDRI(u_PrefilterMap, normalize(ReflectionNormal), roughness * specularTextureLevels).rgb * vec3(1.0 - roughness);

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		vec2 specularBRDF = texture(u_BrdfLUT, vec2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
		ambientLighting = MinVec3(diffuseIBL) + MinVec3(specularIBL);
	}
    
	// Final fragment color.
	if (worldPos != vec3(0.0))
	{
		out_Color = vec4(directLighting + ambientLighting, 1.0);
	}
	else
	{
	    if (directLighting != vec3(0.0))
		{
			out_Color = vec4(directLighting, 1.0);
		}
		else
		{
			out_Color = u_ForwardClearColor;
		}
	}
}