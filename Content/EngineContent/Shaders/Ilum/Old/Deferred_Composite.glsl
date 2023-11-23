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


float Remap(float value, float in1, float in2, float out1, float out2)
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

vec2 GetSampleCoordinate(vec3 v3)
{
	vec2 coord;
	float slice;
	v3.z = -v3.z;
	
	if (abs(v3.x) >= abs(v3.y) && abs(v3.x) >= abs(v3.z))
	{
		v3.y = -v3.y;
		if (v3.x > 0) //Positive X
		{
			slice = 0;
			v3 /= v3.x;
			coord = v3.yz;
		}
		else
		{
			v3.z = -v3.z;
			slice = 1; //Negative X
			v3 /= v3.x;
			coord = v3.yz;
		}
	}
	else if (abs(v3.y) >= abs(v3.x) && abs(v3.y) >= abs(v3.z))
	{
		if (v3.y > 0)
		{
			slice = 2; // PositiveY;
			v3 /= v3.y;
			coord = v3.xz;
		}
		else
		{
			v3.z = -v3.z;
			slice = 3; // NegativeY;
			v3 /= v3.y;
			coord = v3.xz;
		}
	}
	else
	{
		v3.y = -v3.y;
		//Z
		if (v3.z < 0) //Pos Z
		{
			slice = 4;
			v3 /= v3.z;
			coord = v3.yx;
		}
		else
		{
			v3.x = -v3.x;
			slice = 5; // NegativeY;
			v3 /= v3.z;
			coord = v3.yx;
		}
	}
	// a possible precision problem?
	const float sixth = 1.0f / 6;
	//now we are in [-1,1]x[-1,1] space, so transform to texCoords
	coord = (coord + vec2(1, 1)) * 0.5f;
	//now transform to slice position
	coord.y = coord.y * sixth + slice * sixth;
	return coord;
}
float IlumUVCoord(float u, int index)
{
	u /= 64.0;
	u += (1.0/64.0) * index; 
	return u;
}
uniform sampler2D u_IlumBuffer;
uniform int u_IlumProbeIndex;
uniform vec3 u_IlumProbePos[64];
vec4 _SampleIlum(vec3 N, int index)
{
	const float sixth = 1.0 / 6;
	N = normalize(N);
	vec2 uv = GetSampleCoordinate(N);
	
	vec2 coords = vec2(IlumUVCoord(1.0 - uv.x, index), uv.y);
	return texture(u_IlumBuffer, coords, 0.1).rgba;

	return vec4(1.0, 0.0, 0.0, 1.0);
}
float GetProbeImportance(int index)
{
	if (index == u_IlumProbeIndex) return 0.33;
	if (abs(u_IlumProbeIndex - index) == 1) return 0.50;
	if (abs(u_IlumProbeIndex - index) == 2) return 0.67;
	if (abs(u_IlumProbeIndex - index) == 3) return 0.86;
	if (u_IlumProbeIndex <= 3)
	{
		if (abs(u_IlumProbeIndex + 64 - index) == 1) return 0.50;
		if (abs(u_IlumProbeIndex + 64 - index) == 2) return 0.67;
		if (abs(u_IlumProbeIndex + 64 - index) == 3) return 0.86;
	}
	if (u_IlumProbeIndex >= 60)
	{
		if (abs(64 - u_IlumProbeIndex - index) == 1) return 0.50;
		if (abs(64 - u_IlumProbeIndex - index) == 2) return 0.67;
		if (abs(64 - u_IlumProbeIndex - index) == 3) return 0.86;
	}
	return 1.0;
}
vec4 Ilum(vec3 worldPos, vec3 worldNormal)
{
	int Probes[4];
	float ProbeDists[4];
	{
		Probes[0] = 0; ProbeDists[0] = distance(worldPos, u_IlumProbePos[0]) * GetProbeImportance(Probes[0]);
		Probes[1] = 1; ProbeDists[1] = distance(worldPos, u_IlumProbePos[1]) * GetProbeImportance(Probes[1]);
		Probes[2] = 2; ProbeDists[2] = distance(worldPos, u_IlumProbePos[2]) * GetProbeImportance(Probes[2]);
		Probes[3] = 3; ProbeDists[3] = distance(worldPos, u_IlumProbePos[3]) * GetProbeImportance(Probes[3]);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 3; j > 0; j--)
			if (ProbeDists[j] > ProbeDists[j-1]) 
			{
				float fTemp = ProbeDists[j]; ProbeDists[j] = ProbeDists[j-1]; ProbeDists[j-1] = fTemp;
				int iTemp = Probes[j]; Probes[j] = Probes[j-1]; Probes[j-1] = iTemp;
			}
		}
		for (int probe = 4; probe < 64; probe++)
		{
			float dist = distance(worldPos, u_IlumProbePos[probe]) * GetProbeImportance(Probes[probe]);
			if (dist < ProbeDists[3])
			{
				ProbeDists[3] = dist;
				Probes[3] = probe;
				for (int i = 0; i < 3; i++)
				{
					for (int j = 3; j > 0; j--)
					if (ProbeDists[j] > ProbeDists[j-1]) 
					{
						float fTemp = ProbeDists[j]; ProbeDists[j] = ProbeDists[j-1]; ProbeDists[j-1] = fTemp;
						int iTemp = Probes[j]; Probes[j] = Probes[j-1]; Probes[j-1] = iTemp;
					}
				}
			}
		}
	}
	
	vec4 irradiance = vec4(0.0, 0.0, 0.0, 1.0);
	// vec2 normalUV = SampleSphericalMap(worldNormal);
	// irradiance += textureLod(u_IlumBuffer, vec2(Remap(normalUV.x, 0.0, 1.0, Rect1.x, Rect1.z), Remap(normalUV.y, 0.0, 1.0, Rect1.y, Rect1.w)), 0.5).rgb;

	irradiance += _SampleIlum(normalize(worldNormal * 8 + (u_IlumProbePos[Probes[0]] - worldPos)), Probes[0]) /* GetProbeImportance(Probes[0])*/ * 0.67;
	irradiance += _SampleIlum(normalize(worldNormal * 8 + (u_IlumProbePos[Probes[1]] - worldPos)), Probes[1]) /* GetProbeImportance(Probes[1])*/ * 0.22;
	irradiance += _SampleIlum(normalize(worldNormal * 8 + (u_IlumProbePos[Probes[2]] - worldPos)), Probes[2]) /* GetProbeImportance(Probes[2])*/ * 0.07407;
	irradiance += _SampleIlum(normalize(worldNormal * 8 + (u_IlumProbePos[Probes[3]] - worldPos)), Probes[3]) /* GetProbeImportance(Probes[3])*/ * 0.049382;

	return irradiance;
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
	vec3 directLighting = texture(u_Radiance, UV).rgb;

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
		vec3 irradiance = Ilum(worldPos, N).rgb * 2.0; // texture(irradianceTexture, N).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		vec3 F = fresnelSchlick(cosLo, F0);

		// Get diffuse contribution factor (as with direct lighting).
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		vec3 diffuseIBL = kd * albedo * irradiance; // + (vec3(0.3) * albedo);

		// Sample pre-filtered specular reflection environment at correct mipmap level.
		int specularTextureLevels = textureQueryLevels(u_PrefilterMap);
		vec3 ReflectionNormal = reflect(u_View, N); // N;
		vec3 specularIrradiance = sampleHDRI(u_PrefilterMap, normalize(ReflectionNormal + 15.0 * N), roughness * specularTextureLevels).rgb * vec3(1.0 - roughness);

		// Split-sum approximation factors for Cook-Torrance specular BRDF.
		vec2 specularBRDF = texture(u_BrdfLUT, vec2(cosLo, roughness)).rg;

		// Total specular IBL contribution.
		vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		// Total ambient lighting contribution.
		ambientLighting = diffuseIBL + specularIBL;
	}
    

	// Final fragment color.
	out_Color = vec4(directLighting + ambientLighting, 1.0);
}