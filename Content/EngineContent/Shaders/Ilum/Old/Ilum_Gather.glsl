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

// Cubemap
uniform sampler2D u_Top;		
uniform sampler2D u_Bottom;		
uniform sampler2D u_Left;		
uniform sampler2D u_Right;		
uniform sampler2D u_Forward;	
uniform sampler2D u_Backward;	
uniform sampler2D u_EquirectangularLUT;

in vec2 UV;
out vec4 out_Color;

#define PI 3.1415926538

vec3 EquirectangularLUT(vec2 uv)
{
	vec3 dir = texture(u_EquirectangularLUT, uv).xyz;
	return dir * vec3(2.0) - vec3(1.0);
}

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

vec4 Sample(vec3 worldNormal)
{
	const float sixth = 1.0f / 6;
	worldNormal = normalize(worldNormal);
	vec2 uv = GetSampleCoordinate(worldNormal);
	
	if (uv.y >= sixth * 0 && uv.y <= sixth * 1) return texture(u_Right, (vec2(1.0, 1.0) - vec2(uv.x, (uv.y - sixth * 0) / sixth) * 2)).rgba;
	if (uv.y >= sixth * 1 && uv.y <= sixth * 2) return texture(u_Left, (vec2(2.0, 1.0) - vec2(uv.x, (uv.y - sixth * 1) / sixth) * 2)).rgba;
	if (uv.y >= sixth * 2 && uv.y <= sixth * 3) return texture(u_Top, (vec2(2.0, 1.0) - vec2(uv.x, (uv.y - sixth * 2) / sixth) * 2)).rgba;

	if (uv.y >= sixth * 3 && uv.y <= sixth * 4) return texture(u_Bottom, (vec2(1.0, 1.0) - vec2(uv.x, (uv.y - sixth * 3) / sixth) * 2)).rgba;
	if (uv.y >= sixth * 4 && uv.y <= sixth * 5) 
	{
		vec2 coords = (vec2(1.0, 2.0) - vec2(((uv.y - sixth * 4) / sixth), uv.x) * 2);
		coords = vec2(coords.x, 1.0 - coords.y);
		return texture(u_Forward, coords).rgba;
	}
	if (uv.y >= sixth * 5 && uv.y <= sixth * 6) return texture(u_Backward, (vec2(1.0, 1.0) - vec2(((uv.y - sixth * 5) / sixth), uv.x) * 2)).rgba;

	return vec4(1.0, 0.0, 0.0, 1.0);
}


vec3 sphericalToWorld(vec2 sphCoord, float r)
{
    return vec3(
    	r * sin(sphCoord.y) * cos(sphCoord.x),
        r * sin(sphCoord.y) * sin(sphCoord.x),
        r * cos(sphCoord.y)
    );
}

vec2 worldToSpherical(vec3 flatCoord, float r)
{
    return vec2(
        atan(flatCoord.x, flatCoord.y),
        acos(flatCoord.z / r)
    );   
}

vec3 ToWorldNormal(float u, float v)
{
	float theta = 2 * PI * u; //sector
	float phi = PI * v; //ring
	
	float x = cos(theta) * sin(phi);
	float y = -sin(-PI/2 + phi);
	float z = sin(theta) * sin(phi);

	return vec3(x, y, z);

}

/// Source: https://www.shadertoy.com/view/3l2SDR
// faceIdx is face number 0:back, 1:left 2:front 3:right 4:top 5:bottom
vec3 getFaceUVW(vec2 uv, float faceIdx, vec2 faceSize)
{
    float a = 2.0*uv.x/faceSize.x;
    float b = 2.0*uv.y/faceSize.y;
    	 if (faceIdx<0.5) return vec3(-1., 	1.-a, 3.-b);// back
    else if (faceIdx<1.5) return vec3(a-3., -1.,  3.-b);// left
    else if (faceIdx<2.5) return vec3(1., 	a-5., 3.-b);// front
    else if (faceIdx<3.5) return vec3(7.-a, 1.,   3.-b);// right
    else if (faceIdx<4.5) return vec3(b-1., a-5., 1.  );// top
    					  return vec3(5.-b, a-5., -1. );// bottom
}
        
vec3 EquiRectToCubeMap(vec2 uv)
{
    vec2 gridSize = vec2(4,3); // 4 faces on x, and 3 on y
	vec2 faceSize = 1.0 / gridSize; // 1.0 because normalized coords
    vec2 faceIdXY = floor(uv * gridSize); // face id XY x:0->2 y:0->3
    
    // define the y limit for draw faces
    vec2 limY = vec2(0, uv.y);
    if (faceIdXY.x > 1.5 && faceIdXY.x < 2.5) // top & bottom faces
    	limY = vec2(0,faceSize.y*3.);
    else // all others
        limY = vec2(faceSize.y,faceSize.y*2.);

	// get face id
    float faceId = 0.;
    if (faceIdXY.y<0.5) 	faceId = 4.;		 // top
    else if(faceIdXY.y>1.5) faceId = 5.;		 // bottom
    else 				    faceId = faceIdXY.x; // all others

    // face coord uvw
    vec3 p = getFaceUVW(uv,faceId,faceSize);
        
	return p;
}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 ReverseSphericalMap(vec2 uv)
{
	float dy = sin((uv.y - 0.5) * PI);
	float Atan = (uv.x - 0.5) * 2 * PI;
	float angle = Atan * 180 / PI;
	float len = 1.0;

	float theta = angle * PI / 180;
	float dx, dz;
	// while (length(normalize(vec3(dy, dx, dz))) < 0.75 || length(normalize(vec3(dy, dx, dz))) > 1.25)
	// {
	// 	dx = len * cos(theta);
	// 	dy = len * sin(theta);
	// 	if (length(normalize(vec3(dy, dx, dz))) < 1.0) len += 0.05;
	// 	if (length(normalize(vec3(dy, dx, dz))) > 1.0) len -= 0.05;
	// }
	for (int i = 0; i < 1000; i++)
    {
		dx = len * cos(theta);
		dy = len * sin(theta);
		if (length(normalize(vec3(dy, dx, dz))) < 1.0) len += 0.002;
		if (length(normalize(vec3(dy, dx, dz))) > 1.0) len -= 0.002;
    }

	return normalize(vec3(dy, dx, dz));
}

float rand(vec2 co)
{
    return (fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;
}

vec4 Convolute(vec3 worldNormal)
{
	worldNormal = normalize(worldNormal);
	vec4 col = vec4(0.0);
	float count = 0.0;

	for (int i = 0; i < 64; i++)
	{
		vec3 dir = normalize(worldNormal * 32.0 + 16.0 * vec3(rand(vec2(worldNormal.xy * i)), rand(vec2(worldNormal.yz * i)), rand(vec2(worldNormal.xz * i))));
		float loss = 1;//dot(worldNormal, dir);
		col += Sample(dir) * loss * loss;
		count += 1.0;
	}

	return col / count;
}

uniform int u_Face;

void main(void)
{
/*
	vec4 color = vec4(0.0);
	float count = 0;
	for (int i = 0; i < 64 || count == 0; i++)
	{
		float t = float(i);
		vec3 dir = vec3(rand(UV + vec2(1.0 + t, 0.3 - t)), rand(UV + vec2(1.0 + t, 0.3 * t)), rand(UV + vec2(6.0 * t, 7.8 + t)));
		vec2 uv = SampleSphericalMap(dir);
		float dist = distance(uv, UV);
		float dist2 = distance(vec2(uv.x - 1.0, uv.y), UV); if (dist2 < dist) dist = dist2;
		float dist3 = distance(vec2(uv.x + 1.0, uv.y), UV); if (dist3 < dist) dist = dist3;
		float dist4 = distance(vec2(uv.x, uv.y - 1.0), UV); if (dist4 < dist) dist = dist4;
		float dist5 = distance(vec2(uv.x, uv.y + 1.0), UV); if (dist5 < dist) dist = dist5;
		if (dist <= 0.1)
		{
			color += Sample(dir);
			count += 1.0;
		}
	}
	out_Color = color / count;
	*/
	
	if (u_Face == 0)
	{
		out_Color = Convolute(vec3(UV.y, UV.x, -1));
		return;
	}
	if (u_Face == 1)
	{
		out_Color = Convolute(vec3(1.0 - UV.y, 1.0 - UV.x, +1));
		return;
	}
	if (u_Face == 2)
	{
		out_Color = Convolute(vec3(UV.x, -1, 1.0 - UV.y));
		return;
	}
	if (u_Face == 3)
	{
		out_Color = Convolute(vec3(UV.x, +1, UV.y));
		return;
	}
	if (u_Face == 4)
	{
		out_Color = Convolute(vec3(-1, UV.y, 1.0 - UV.x));
		return;
	}
	if (u_Face == 5)
	{
		out_Color = Convolute(vec3(+1, UV.y, UV.x));
		return;
	}

}