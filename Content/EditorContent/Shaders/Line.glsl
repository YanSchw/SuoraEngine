#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
out vec3 Pos;
out vec2 UV;

void main(void)
{
	gl_Position = vec4(a_Position, 1.0);
  Pos = a_Position;
	UV = a_TexCoord;
}

#type fragment
#version 330 core

in vec3 Pos;
in vec2 UV;
out vec4 out_Color;

uniform float u_Zoom;
uniform vec2 u_ViewportSize;
uniform vec2 u_PointBegin;
uniform vec2 u_PointEnd;
uniform vec4 u_Color;

float Remap(float value, float in1, float in2, float out1, float out2)
{
	return out1 + (value - in1) * (out2 - out1) / (in2 - in1);
}

// bezier curve with 2 control points
// A is the starting point, B, C are the control points, D is the destination
// t from 0 ~ 1
vec2 Bezier(vec2 A, vec2 B, vec2 C, vec2 D, float t) 
{
  vec2 E = mix(A, B, t);
  vec2 F = mix(B, C, t);
  vec2 G = mix(C, D, t);

  vec2 H = mix(E, F, t);
  vec2 I = mix(F, G, t);

  vec2 P = mix(H, I, t);

  return P;
}

float DistanceToLineSegment_Aspect(vec3 p, vec3 a, vec3 b, float aspect)
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    vec3 dist = pa - ba*h;
    return (u_ViewportSize.x > u_ViewportSize.y) ? length( vec2(dist.x * aspect, dist.y / aspect) )
                                                 : length( vec2(dist.x / aspect, dist.y * aspect) );
}
float DistanceToLineSegment(vec3 p, vec3 a, vec3 b)
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}
float DistanceToBezier(vec2 p, vec2 A, vec2 B, vec2 C, vec2 D, float t)
{
    vec2 bezier = Bezier(A, B, C, D, t);
    return length(p - bezier);
}

bool InBezierCurve(vec2 p, vec2 A, vec2 B, vec2 C, vec2 D, float thickness)
{
    bool b = false;
    for (float t = 0.0; t <= 1.0; t += 0.01)
    {
        b = DistanceToBezier(p, A, B, C, D, t) < thickness;
        if (b) break;
    }

    return b;
}

void main(void)
{
    float thickness = 0.0025 * u_Zoom;
    float scaleU = u_ViewportSize.x / 1000.0f;
    float scaleV = u_ViewportSize.y / 1000.0f;
    vec3 aspect = vec3(scaleU, scaleV, 0.0);
    vec3 PointBegin = vec3(u_PointBegin.x / u_ViewportSize.x * 2.0, u_PointBegin.y / u_ViewportSize.y * 2.0, 0.0);
    vec3 PointEnd = vec3(u_PointEnd.x / u_ViewportSize.x * 2.0, u_PointEnd.y / u_ViewportSize.y * 2.0, 0.0);
    
    if (DistanceToLineSegment(Pos * aspect, PointBegin * aspect, PointEnd * aspect) < thickness)
    {
		vec3 pixelSize = vec3(1.0 / u_ViewportSize, 1.0);
		
        out_Color = u_Color;
    }
    else
    {
        out_Color = vec4(0.0);
    }
}