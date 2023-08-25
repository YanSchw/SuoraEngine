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
uniform vec4 u_Color;
uniform int u_Thickness;

int Kernel()
{
	bool result = false;
	int sampleRadius = u_Thickness; // 4;
	vec2 pixelSize = 1.0 / textureSize(u_Texture, 0);

	// Dithering
	/*ivec2 absolute = ivec2(UV * textureSize(u_Texture, 0));
	if (absolute.x % 2 == 0 && absolute.y % 2 == 1) return 0;*/

	for (int y = -sampleRadius; y <= sampleRadius; y++)
	{
	    for (int x = -sampleRadius; x <= sampleRadius; x++)
	    {
			vec2 kernelUV = UV + vec2(x, y) * pixelSize;
			if (kernelUV.x < 0.0 || kernelUV.x > 1.0 || kernelUV.y < 0.0 || kernelUV.y > 1.0) continue;
		    
			vec4 kernelColor = texture(u_Texture, kernelUV);
			if (kernelColor.r == 1.0) result = true;
	    }    
	}

	if (result == true && texture(u_Texture, UV).r == 0.0)
	{
		return 1;
	}

	return 0;
}

void main(void)
{
	if (Kernel() == 0) discard;
	out_Color = vec4(u_Color);
}