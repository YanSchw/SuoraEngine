// Rect Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

out vec2 UV;
out vec4 v_Color;


void main()
{
	UV = a_TexCoord;
	v_Color = a_Color;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 UV;
in vec4 v_Color;

const float glyphEdge = 0.35;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

#define SUPERSAMPLE

// fwidth() is not supported by default on OpenGL ES. Enable it.
#if defined(GL_OES_standard_derivatives)
  #extension GL_OES_standard_derivatives : enable
#endif

float contour(float dist, float edge, float width) 
{
	return clamp(smoothstep(edge - width, edge + width, dist), 0.0, 1.0);
}

float getSample(vec2 texCoords, float edge, float width) 
{
	return contour(texture2D(u_Texture, texCoords).a, edge, width);
}

void main()
{
    float AtlasSize = float(textureSize(u_Texture, 0).x);
    vec4 tex = texture2D(u_Texture, UV);

    float dist  = tex.a;
    float width = fwidth(dist);
    vec4 textColor = u_Color * v_Color;
    float outerEdge = glyphEdge;

    #if defined(SUPERSAMPLE)
    float alpha = contour(dist, outerEdge, width);

    float dscale =  0.354; //0.525; // 0.354; // half of 1/sqrt2; you can play with this
    vec2 uv = UV.xy;
    vec2 duv = dscale * (dFdx(uv) + dFdy(uv));
    vec4 box = vec4(uv - duv, uv + duv);

    float asum = getSample(box.xy, outerEdge, width)
               + getSample(box.zw, outerEdge, width)
               + getSample(box.xw, outerEdge, width)
               + getSample(box.zy, outerEdge, width);

    // weighted average, with 4 extra points having 0.5 weight each,
    // so 1 + 0.5*4 = 3 is the divisor
    alpha = (alpha + 0.5 * asum) / 3.0;

    #else
    // No supersampling.
    float alpha = contour(dist, outerEdge, width);
    #endif

    gl_FragColor = tex * vec4(textColor.rgb, textColor.a * alpha);

	
}