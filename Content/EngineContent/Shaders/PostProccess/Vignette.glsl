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

in vec2 UV;
out vec4 out_Color;

uniform sampler2D u_Texture;
uniform float u_Intensity;
uniform float u_Extend;
uniform vec4 u_VignetteColor;

vec4 Lerp(vec4 a, vec4 b, float v)
{
    return a + ((b - a) * v);
}

void main(void)
{
	vec4 fragColor = texture(u_Texture, UV);
    vec2 uv = UV;   
    uv *=  1.0 - uv.yx; 
    
    float vig = uv.x * uv.y * u_Intensity; 
    vig = pow(vig, u_Extend);
    vig = clamp(vig, 0.0, 1.0);

    out_Color = vec4(Lerp(u_VignetteColor, fragColor, vig).rgb, 1.0);
}