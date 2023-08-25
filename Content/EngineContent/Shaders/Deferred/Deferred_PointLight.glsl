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

#define PI 3.1415926538
const float Epsilon = 0.00001;
const vec3 Fdielectric = vec3(0.04);

struct PointLightMatrixStruct
{
	mat4 ViewTop;
	mat4 ViewBottom;
	mat4 ViewLeft;
	mat4 ViewRight;
	mat4 ViewForward;
	mat4 ViewBackward;
};

layout(std430, binding = 0) readonly buffer ssbo
{
	PointLightMatrixStruct PointLightViews[];
};

in vec2 UV;
layout(location = 0) out vec4 out_DirectLight;

uniform sampler2D u_BaseColor;
uniform sampler2D u_Metallic;
uniform sampler2D u_Roughness;
uniform sampler2D u_WorldPos;
uniform sampler2D u_WorldNormal;
uniform sampler2D u_ShadowAtlas;
uniform vec3 u_ViewPos;
uniform int u_PointLights;
uniform int u_ShadowMapCount;
uniform vec3 u_PointLightPos[32];
uniform vec3 u_PointLightColor[32];
uniform float u_PointLightRadius[32];
uniform float u_PointLightIntensity[32];
uniform int u_PointLightShadowIndex[32];


uniform mat4 u_ViewTop[32];
uniform mat4 u_ViewBottom[32];
uniform mat4 u_ViewLeft[32];
uniform mat4 u_ViewRight[32];
uniform mat4 u_ViewForward[32];
uniform mat4 u_ViewBackward[32];

float Remap(float value, float in1, float in2, float out1, float out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}

const float near = 0.1; 
const float far  = 45.0; 
 
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
	
}
float InShadow(int index, vec3 fragWorldPos)
{
	float shadowDist = 0.0;
	float width = 1.0 / float(u_ShadowMapCount);
	const float sixth = 1.0f / 6;

	vec4 fragPosLight = u_ViewTop[index] * vec4(fragWorldPos, 1.0);
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	// Get from [-1, 1] range to [0, 1] range just like the shadow map
	lightCoords = (lightCoords + 1.0) / 2.0;
	if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
	{
		// Top
		lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 2,  sixth * 3);
	}
	else
	{
		fragPosLight = u_ViewBottom[index] * vec4(fragWorldPos, 1.0);
		lightCoords = fragPosLight.xyz / fragPosLight.w;
		lightCoords = (lightCoords + 1.0) / 2.0;
		if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
		{
			// Bottom
			lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 3,  sixth * 4);
		}
		else
		{
			fragPosLight = u_ViewLeft[index] * vec4(fragWorldPos, 1.0);
			lightCoords = fragPosLight.xyz / fragPosLight.w;
			lightCoords = (lightCoords + 1.0) / 2.0;
			if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
			{
				// Left
				lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 1,  sixth * 2);
			}
			else
			{
				fragPosLight = u_ViewRight[index] * vec4(fragWorldPos, 1.0);
				lightCoords = fragPosLight.xyz / fragPosLight.w;
				lightCoords = (lightCoords + 1.0) / 2.0;
				if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
				{
					// Right
					lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 0,  sixth * 1);
				}
				else
				{
					fragPosLight = u_ViewForward[index] * vec4(fragWorldPos, 1.0);
					lightCoords = fragPosLight.xyz / fragPosLight.w;
					lightCoords = (lightCoords + 1.0) / 2.0;
					if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
					{
						// Forward
						lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 4,  sixth * 5);
					}
					else
					{
						fragPosLight = u_ViewBackward[index] * vec4(fragWorldPos, 1.0);
						lightCoords = fragPosLight.xyz / fragPosLight.w;
						lightCoords = (lightCoords + 1.0) / 2.0;
						if (lightCoords.x >= 0.0 && lightCoords.x <= 1.0 && lightCoords.y >= 0.0 && lightCoords.y <= 1.0 && fragPosLight.w >= 0.0)
						{
							// Backward
							lightCoords.y = Remap(lightCoords.y, 0.0, 1.0, sixth * 5,  sixth * 6);
						}
					}
				}
			}
		}
	}
	
	lightCoords.x = Remap(lightCoords.x, 0.0, 1.0, width * float(index),  width * float(index) + width);
	shadowDist = texture(u_ShadowAtlas, lightCoords.xy).r;
	float currentDepth = lightCoords.z;

	if (currentDepth < shadowDist + 0.00001) return 1.0;
	return 0.0;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main(void)
{
	vec3 light = vec3(0.0);

	// Sample input textures to get shading model params.
	vec3 albedo = texture(u_BaseColor, UV).rgb;
	float metalness = texture(u_Metallic, UV).r;
	float roughness = texture(u_Roughness, UV).x;
	vec3 worldPos = texture(u_WorldPos, UV).xyz;
	vec3 worldNormal = texture(u_WorldNormal, UV).xyz;

	// Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(u_ViewPos - worldPos);

	// Get current fragment's normal and transform to world space.
	vec3 N = worldNormal;
	
	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
	vec3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(Fdielectric, albedo, metalness);

	int i = u_PointLights;
	while (i-- > 0)
	{
		float lightConstant = u_PointLightRadius[i];
		float lightLinear = 1.0;
		float lightQuadratic = 1.0;
		float dist = distance(worldPos, u_PointLightPos[i]);
		float attenuation = u_PointLightRadius[i] / (lightConstant + lightLinear * dist + lightQuadratic * (dist * dist));

		vec3 Li = normalize(u_PointLightPos[i] - worldPos);
		vec3 Lradiance = u_PointLightColor[i] * attenuation * u_PointLightIntensity[i];

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		light += (diffuseBRDF + specularBRDF) * Lradiance * cosLi * ((u_PointLightShadowIndex[i] != -1) ? InShadow(u_PointLightShadowIndex[i], (worldPos)) : 1.0);
	}

	out_DirectLight = vec4(light, 1.0);
	
}