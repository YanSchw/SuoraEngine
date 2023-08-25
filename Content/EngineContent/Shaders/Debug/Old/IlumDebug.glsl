#type vertex
#version 330 core
//$RENDER_PASS Forward
//$SHADER_ID 11

layout(location = 0) in vec3 a_Position;
layout(location = 3) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
out vec2 UV;
out vec3 vPos;

uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;


void main(void)
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
	
	vPos = a_Position;
	UV = a_TexCoord;
}

#type fragment
#version 330 core

in vec3 vPos;
in vec2 UV;
out vec4 out_Color;

uniform sampler2D u_Texture;
uniform int u_Index;
uniform vec3 u_Pos;

//vec3 doesn't have to be normalized,
//Translates from world space vector to a coordinate inside our 6xsize shadow map
vec2 GetSampleCoordinate(vec3 v3)
{
	vec2 coord;
	float slice;
	v3.z = -v3.z;
	
	if (abs(v3.x) >= abs(v3.y) && abs(v3.x) >= abs(v3.z))
	{
		v3.y = -v3.y;
		if (v3.x > 0) //Positive X
		{
			slice = 0;
			v3 /= v3.x;
			coord = v3.yz;
		}
		else
		{
			v3.z = -v3.z;
			slice = 1; //Negative X
			v3 /= v3.x;
			coord = v3.yz;
		}
	}
	else if (abs(v3.y) >= abs(v3.x) && abs(v3.y) >= abs(v3.z))
	{
		if (v3.y > 0)
		{
			slice = 2; // PositiveY;
			v3 /= v3.y;
			coord = v3.xz;
		}
		else
		{
			v3.z = -v3.z;
			slice = 3; // NegativeY;
			v3 /= v3.y;
			coord = v3.xz;
		}
	}
	else
	{
		v3.y = -v3.y;
		//Z
		if (v3.z < 0) //Pos Z
		{
			slice = 4;
			v3 /= v3.z;
			coord = v3.yx;
		}
		else
		{
			v3.x = -v3.x;
			slice = 5; // NegativeY;
			v3 /= v3.z;
			coord = v3.yx;
		}
	}
	
	// a possible precision problem?
	const float sixth = 1.0f / 6;
	
	//now we are in [-1,1]x[-1,1] space, so transform to texCoords
	coord = (coord + vec2(1, 1)) * 0.5f;
	
	//now transform to slice position
	coord.y = coord.y * sixth + slice * sixth;
	return coord;
}
float IlumUVCoord(float u, int index)
{
	u /= 64.0;
	u += (1.0/64.0) * index; 
	return u;
}
vec4 SampleIlum(vec3 N, int index)
{
	const float sixth = 1.0 / 6;
	N = normalize(N);
	vec2 uv = GetSampleCoordinate(N);
	
	
	vec2 coords = vec2(IlumUVCoord(1.0 - uv.x, index), uv.y);
	return vec4(texture(u_Texture, coords).rgb, 1.0);

/*
	if (uv.y >= sixth * 0 && uv.y <= sixth * 1) 
	{
		//vec2 coords = (vec2(1.0, 0.0) - vec2(uv.x, uv.y) * 2);
		vec2 coords = vec2(IlumUVCoord(1.0 - uv.x, u_Index), uv.y);
		return texture(u_Texture, coords).rgba;
	}
	if (uv.y >= sixth * 1 && uv.y <= sixth * 2) 
	{
		vec2 coords = (vec2(1.0, 0.0) - vec2(uv.x, uv.y));
		coords = vec2(IlumUVCoord(coords.x, u_Index), coords.y * -1);
		return texture(u_Texture, coords).rgba;
	}
	if (uv.y >= sixth * 2 && uv.y <= sixth * 3) 
	{
		vec2 coords = (vec2(2.0, 1.0) - vec2(uv.x, (uv.y - sixth * 0) / 1) * 2);
		coords = vec2(IlumUVCoord(coords.x, u_Index), coords.y);
		return texture(u_Texture, coords).rgba;
	}

	if (uv.y >= sixth * 3 && uv.y <= sixth * 4) 
	{
		vec2 coords = (vec2(1.0, 1.0) - vec2(uv.x, (uv.y - sixth * 0) / 1) * 2);
		coords = vec2(IlumUVCoord(coords.x, u_Index), coords.y);
		return texture(u_Texture, coords).rgba;
	}
	if (uv.y >= sixth * 4 && uv.y <= sixth * 5) 
	{
		vec2 coords = (vec2(1.0, 2.0) - vec2(((uv.y - sixth * 0) / 1), uv.x) * 2);
		coords = vec2(coords.x, 1.0 - coords.y);
		coords = vec2(IlumUVCoord(coords.x, u_Index), coords.y);
		return texture(u_Texture, coords).rgba;
	}
	if (uv.y >= sixth * 5 && uv.y <= sixth * 6)
	{
		vec2 coords = (vec2(1.0, 1.0) - vec2(((uv.y - sixth * 0) / 1), uv.x) * 2);
		coords = vec2(IlumUVCoord(coords.x, u_Index), coords.y);
		return texture(u_Texture, coords).rgba;
	} 
*/
	return vec4(1.0, 0.0, 0.0, 1.0);
}

float Remap(float value, float in1, float in2, float out1, float out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}
void main(void)
{
	vec3 NORMAL = normalize(vPos - u_Pos);
		
	out_Color = SampleIlum(NORMAL, u_Index);
}