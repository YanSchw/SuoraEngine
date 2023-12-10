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

/*
SSAO GLSL Shader

Inspired by https://github.com/spite/Wagner/blob/master/fragment-shaders/ssao-simple-fs.glsl
License: MIT

November 6th 2023
*/

#define PI 3.14159265

uniform sampler2D u_Texture;
uniform sampler2D u_Depth;
uniform vec4 u_AOColor;
uniform vec2 u_Resolution;
uniform float u_Near;
uniform float u_Far;
uniform float u_Intensity;
uniform float u_Alpha;
uniform float u_NearGaussBellFactor;
uniform int u_Samples; // max 64.0
uniform float u_Radius;

float width = u_Resolution.x;
float height = u_Resolution.y;

float aoclamp = 0.125; //depth clamp - reduces haloing at screen edges
bool noise = true; //use noise instead of pattern for sample dithering
float noiseamount = 0.0002; //dithering amount

float diffarea = 0.3; //self-shadowing reduction
float gdisplace = 0.4; //gauss bell center //0.4


vec2 rand(vec2 coord) //generating noise/pattern texture for dithering
{
  float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
  float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;

  if (noise)
  {
    noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
    noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
  }
  return vec2(noiseX,noiseY) * noiseamount;
}

float ReadDepth(vec2 coord)
{
  if (UV.x<0.0||UV.y<0.0) return 1.0;
  else 
  {
    float z_b = texture2D(u_Depth, coord ).x;
    float z_n = 2.0 * z_b - 1.0;
    return (2.0 * u_Near) / (u_Far + u_Near - z_n * (u_Far-u_Near));
  }
}

int CompareDepthsFar(float depth1, float depth2) 
{
  float garea = 2.0; //gauss bell width
  float diff = (depth1 - depth2) * 100.0; //depth difference (0-100)
  //reduce left bell width to avoid self-shadowing
  if (diff<gdisplace)
  {
    return 0;
  } 
  else 
  {
    return 1;
  }
}

float CompareDepths(float depth1, float depth2)
{
  float garea = 2.0; //gauss bell width
  float diff = (depth1 - depth2) * 100.0 * u_NearGaussBellFactor; //depth difference (0-100)
  //reduce left bell width to avoid self-shadowing
  if (diff < gdisplace)
  {
    garea = diffarea;
  }

  float gauss = pow(2.7182, -2.0 * (diff - gdisplace) * (diff - gdisplace) / (garea * garea));
  return gauss;
}

float CalcAO(float depth, float dw, float dh)
{
  float dd = (1.0 - depth) * u_Radius;

  float temp = 0.0;
  float temp2 = 0.0;
  float coordw = UV.x + dw * dd;
  float coordh = UV.y + dh * dd;
  float coordw2 = UV.x - dw * dd;
  float coordh2 = UV.y - dh * dd;

  vec2 coord = vec2(coordw, coordh);
  vec2 coord2 = vec2(coordw2, coordh2);

  float cd = ReadDepth(coord);
  int far = CompareDepthsFar(depth, cd);
  temp = CompareDepths(depth, cd);
  //DEPTH EXTRAPOLATION:
  if (far > 0)
  {
    temp2 = CompareDepths(ReadDepth(coord2),depth);
    temp += (1.0 - temp) * temp2;
  }

  return temp;
}

vec4 Lerp(vec4 a, vec4 b, float v)
{
    return a + ((b - a) * v);
}

void main(void)
{
  vec2 noise = rand(UV);
  float depth = ReadDepth(UV);

  float w = (1.0 / width) / clamp(depth, aoclamp, 1.0) + (noise.x * (1.0 - noise.x));
  float h = (1.0 / height) / clamp(depth, aoclamp, 1.0) + (noise.y * (1.0 - noise.y));

  float pw = 0.0;
  float ph = 0.0;

  float ao = 0.0;

  float dl = PI * (3.0 - sqrt(5.0));
  float dz = 1.0 / float(u_Samples);
  float l = 0.0;
  float z = 1.0 - dz/2.0;

  for (int i = 0; i < 64; i++)
  {
    if (i > u_Samples) break;
    float r = sqrt(1.0 - z);

    pw = cos(l) * r;
    ph = sin(l) * r;
    ao += CalcAO(depth, pw * w, ph * h);
    z = z - dz;
    l = l + dl;
  }


  ao /= float(u_Samples);
  ao = 1.0-ao;

  vec4 fragColor = texture(u_Texture, UV);

  ao = clamp(ao, 0.0, 1.0);
  ao = pow(ao, u_Intensity);

  float alpha = clamp(u_Alpha, 0.0, 1.0);
  
  out_Color = vec4(Lerp(fragColor, u_AOColor, (1.0 - ao) * alpha).rgb, 1.0);

}