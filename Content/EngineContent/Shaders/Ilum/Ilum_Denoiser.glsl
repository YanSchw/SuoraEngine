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

out vec4 out_Color;

uniform sampler2D imageData;
uniform vec2 u_Resolution;

uniform float uSigma;
uniform float uThreshold;
uniform float uKSigma;
uniform vec2 wSize;

void main(void)
{
	vec3 color = texture(imageData, UV).rgb;
    
    float sampleRadius = 3.0;
    int reduceAvg = -1;
	vec2 pixelSize = 1.0 / textureSize(imageData, 0);
	for (float y = -sampleRadius; y <= sampleRadius; y++)
	{
	    for (float x = -sampleRadius; x <= sampleRadius; x++)
	    {
			vec2 uv = UV + vec2(x, y) * pixelSize;
			if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
			{
                reduceAvg++;
				continue;
			} 
			color += texture(imageData, uv).rgb;
	    }    
	}
	// Get average shadow
	color /= pow((sampleRadius * 2.0 + 1.0), 2.0) - float(reduceAvg);

    out_Color = vec4(color, 1.0);
}