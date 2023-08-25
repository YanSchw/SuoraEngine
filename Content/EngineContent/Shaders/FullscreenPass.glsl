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
layout(location = 1) out vec4 out_Color2;
layout(location = 2) out vec4 out_Color3;
layout(location = 3) out vec4 out_Color4;
uniform sampler2D u_Texture;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform sampler2D u_Texture4;

void main(void)
{
	out_Color = texture(u_Texture, UV);
	out_Color2 = texture(u_Texture2, UV);
	out_Color3 = texture(u_Texture3, UV);
	out_Color4 = texture(u_Texture4, UV);
}