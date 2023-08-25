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
layout(location = 0) out vec4 out_Color;
uniform isampler2D u_Texture;

/// Source: https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// Gold Noise (c)2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated seeding method. 

float PHI = 1.61803398874989484820459;  // PHI = Golden Ratio   

float gold_noise(in vec2 xy, in float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}
vec3 RandomColor(int val)
{
	float r = gold_noise(vec2(float(val + 1), float(val + 1)), 1.0);
	float g = gold_noise(vec2(float(val + 2), float(val + 5)), 3.2);
	float b = gold_noise(vec2(float(val + 8), float(val + 2)), 6.8);
	return vec3(r, g, b);
}

void main(void)
{
	int cluster = texture(u_Texture, UV).r;

	if (cluster == 0)
	{
		out_Color = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		out_Color = vec4(RandomColor(cluster), 1.0);
	}

}