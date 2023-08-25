#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;

uniform float u_FarPlane;

void main(void)
{
	gl_Position = vec4(a_Position.xy, 0.999999, 1.0);
	UV = a_TexCoord;
}

#type fragment
#version 330 core

in vec2 UV;
// Output
layout(location = 0) out vec3 out_BaseColor;
layout(location = 1) out float out_Metallic;
layout(location = 2) out float out_Roughness;
layout(location = 3) out vec3 out_WorldPos;
layout(location = 4) out vec3 out_WorldNormal;
layout(location = 5) out vec3 out_Emissive;

uniform vec3 u_LightDirection;
uniform vec3 u_ViewDirection;
uniform vec3 u_ViewPos;
uniform vec2 u_Resolution;
uniform mat4 u_ViewProjection;

vec3 GetSky(vec2 uv, vec2 pos)
{
    float atmosphereHeight = 1.2;
    atmosphereHeight -= u_ViewDirection.y * (u_ViewDirection.y < 0.0 ? 0.75 : 0.2);

    float atmosphere = sqrt(atmosphereHeight - uv.y);
    vec3 skyColor = vec3(0.2, 0.4, 0.8);
    
    float scatter = pow(pos.y / u_Resolution.y, 1.0 / 15.0);
    scatter = 1.0 - clamp(scatter, 0.8, 1.0);
    
    vec3 scatterColor = mix(vec3(1.0), vec3(1.0, 0.3, 0.0) * 1.5, scatter);
    return mix(skyColor, vec3(scatterColor), atmosphere / 1.3);
    
}

vec3 GetSun(vec2 uv, vec2 pos)
{
	float sun = 1.0 - distance(uv, pos);
    sun = clamp(sun,0.0,1.0);
    
    float glow = sun;
    glow = clamp(glow,0.0,1.0);
    
    sun = pow(sun,100.0);
    sun *= 100.0;
    sun = clamp(sun,0.0,1.0);
    
    glow = pow(glow,6.0) * 1.0;
    glow = pow(glow,(uv.y));
    glow = clamp(glow,0.0,1.0);
    
    sun *= pow(dot(uv.y, uv.y), 1.0 / 1.65);
    
    glow *= pow(dot(uv.y, uv.y), 1.0 / 2.0);
    
    sun += glow;
    
    vec3 sunColor = vec3(1.0,0.6,0.05) * sun;
    
    return vec3(sunColor);
}

void main(void)
{
	vec2 pos = vec2(u_Resolution.x / 2.0, u_Resolution.y / 2.0);
    vec4 sunPos = u_ViewProjection * vec4(/*u_ViewPos*/ - u_LightDirection, 1.0);
    
    vec3 sky = GetSky(UV, pos);
    vec3 sun = vec3(0.0);//GetSun(UV, sunPos.xy);
    
	vec3 fragColor = vec3(sky + sun);

	out_BaseColor = vec3(0.0);//fragColor;
	out_Emissive = fragColor;//vec3(0.05);
	
}