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
#version 330 core

#define PI 3.1415926538
const float Epsilon = 0.00001;
const vec3 Fdielectric = vec3(0.04);

in vec2 UV;
layout(location = 0) out vec4 out_DirectLight;
// layout(location = 0) out vec4 out_Radiance;
// layout(location = 1) out vec4 out_Specular;
// layout(location = 2) out vec4 out_Fresnel;

uniform sampler2D u_BaseColor;
uniform sampler2D u_Roughness;
uniform sampler2D u_Metallness;
uniform sampler2D u_WorldPos;
uniform sampler2D u_WorldNormal;
uniform vec3 u_ViewPos;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform vec3 u_LightDirection;

uniform bool u_Volumetric;
uniform bool u_ShadowMapping;
uniform bool u_SoftShadows;
uniform mat4 u_LightProjection[8];
uniform sampler2D u_ShadowMap[8];
uniform int u_CascadeCount;
uniform int u_CascadeBeginIndex;



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


float Remap(float value, float in1, float in2, float out1, float out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}

float ShadowMap(int cascade, vec3 worldPos, vec3 worldNormal)
{
	// Sets lightCoords to cull space
	vec4 fragPosLight = u_LightProjection[cascade] * vec4(worldPos, 1.0);
	vec3 lightCoords = fragPosLight.xyz / fragPosLight.w;
	if(lightCoords.z <= 1.0 && u_ShadowMapping)
	{
		float shadow = 0.0;
		// Get from [-1, 1] range to [0, 1] range just like the shadow map
		lightCoords = (lightCoords + 1.0) / 2.0;
		float currentDepth = lightCoords.z;
		// Prevents shadow acne
		//float bias = max(0.025 * (1.0 - dot(worldNormal, u_LightDirection)), 0.0005);
		float cascadeBias = 0.00125;
		if (cascade == 1) cascadeBias = 0.002;
		if (cascade == 2) cascadeBias = 0.012;
		if (cascade == 3) cascadeBias = 0.021;
		float bias = max(0.025 * (1.0 - dot(worldNormal, u_LightDirection)), 0.0005) * cascadeBias;

		// Smoothens out the shadows
		int sampleRadius = u_SoftShadows ? 1 : 0;
		vec2 pixelSize = 1.0 / textureSize(u_ShadowMap[cascade], 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for (int x = -sampleRadius; x <= sampleRadius; x++)
		    {
				vec2 shadowUV = lightCoords.xy + vec2(x, y) * pixelSize;
				if (shadowUV.x < 0.0 || shadowUV.x > 1.0 || shadowUV.y < 0.0 || shadowUV.y > 1.0)
				{
					return -1.0;
					//return (cascade < u_CascadeCount) ? ShadowMap(cascade + 1) : 0.0;
				} 
				
		        float closestDepth = texture(u_ShadowMap[cascade], shadowUV).r;
				if (currentDepth > closestDepth + bias)
					shadow += 0.95;     
		    }    
		}
		// Get average shadow
		shadow /= pow((sampleRadius * 2 + 1), 2);
		return shadow;	
	}
	return 0.0;	
}

void main(void)
{
	vec3 worldPos = texture(u_WorldPos, UV).xyz;
	vec3 worldNormal = u_Volumetric ? (u_ViewPos - worldPos) : normalize(texture(u_WorldNormal, UV).xyz);

	float shadow = 0.0;
	if(u_ShadowMapping)
	{
		for (int i = u_CascadeBeginIndex; i < u_CascadeCount; i++)
		{
			shadow = ShadowMap(i, worldPos, worldNormal);
			if (shadow >= 0.0) break;
		}
		if (shadow < 0.0) 
		{
			shadow = 0.0;
		}
	}

	float nDotL = 1.0 - max(dot(worldNormal, u_LightDirection), 0.75);

	// Sample input textures to get shading model params.
	vec3 albedo = texture(u_BaseColor, UV).xyz;
	float roughness = texture(u_Roughness, UV).x;
	float metalness = texture(u_Metallness, UV).x;

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


	vec3 Li = -u_LightDirection;
	vec3 Lradiance = u_LightColor * u_LightIntensity;

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
	out_DirectLight = vec4((diffuseBRDF + specularBRDF) * Lradiance * cosLi, 1.0) * (1.0 - shadow);

}