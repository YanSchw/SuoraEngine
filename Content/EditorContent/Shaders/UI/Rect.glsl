// Rect Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

out vec2 UV;


void main()
{
	UV = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 UV;

uniform vec2 u_Size;
uniform vec4 u_Color;
uniform float u_Roundness;
uniform bool u_UseTexture;
uniform sampler2D u_Texture;

float DistanceInPixels(vec2 From, vec2 To)
{
	From *= u_Size;
	To *= u_Size;

	return sqrt(pow(To.x - From.x,2) + pow(To.y - From.y,2));
}

void main()
{
    if(u_UseTexture)
	{
		vec4 tex = texture(u_Texture, UV);
		color = u_Color * tex;
		
		float alpha = 0.0;
		int sampleRadius = 1;
		vec2 pixelSize = 1.0 / textureSize(u_Texture, 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++)
		{
		    for (int x = -sampleRadius; x <= sampleRadius; x++)
		    {
				vec2 smoothUV = UV + vec2(x, y) * pixelSize;
				if (smoothUV.x < 0.0 || smoothUV.x > 1.0 || smoothUV.y < 0.0 || smoothUV.y > 1.0)
				{
					alpha += 1.0;
					//return (cascade < u_CascadeCount) ? ShadowMap(cascade + 1) : 0.0;
				} 
				
		        alpha += texture(u_Texture, smoothUV).a;
				    
		    }    
		}
		alpha /= pow((sampleRadius * 2 + 1), 2);
		color.a *= alpha;
		color.a *= tex.a;
	}
	else
	{
		color = u_Color;
	}

	if(u_Roundness != 0)
	{
		///////////////////////////////////////////////////
		//  R  |
		//-----P1                                 P2
		//
		//
		//
		//     P3                                 P4
		//
		///////////////////////////////////////////////////

		float Roundness = (u_Roundness < 0) ? u_Roundness * -1 : u_Roundness;
		
		vec2 relative = vec2(Roundness / u_Size.x, Roundness / u_Size.y);
		vec2 P1 = vec2(0 + relative.x, 1 - relative.y);
		vec2 P2 = vec2(1 - relative.x, 1 - relative.y);
		vec2 P3 = vec2(0 + relative.x, 0 + relative.y);
		vec2 P4 = vec2(1 - relative.x, 0 + relative.y);

		if(UV.x < P1.x && UV.y > P1.y)
		{
			if(DistanceInPixels(P1, UV) > Roundness) color = vec4(color.xyz, 0);
		}
		else if(UV.x > P2.x && UV.y > P2.y)
		{
			if(DistanceInPixels(P2, UV) > Roundness) color = vec4(color.xyz, 0);
		}
		else if(UV.x < P3.x && UV.y < P3.y && u_Roundness > 0)
		{
			if(DistanceInPixels(P3, UV) > Roundness) color = vec4(color.xyz, 0);
		}
		else if(UV.x > P4.x && UV.y < P4.y && u_Roundness > 0)
		{
			if(DistanceInPixels(P4, UV) > Roundness) color = vec4(color.xyz, 0);
		}

	}

}