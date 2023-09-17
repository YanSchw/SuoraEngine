#type vertex
#version 330 core

$DEFERRED

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Normal;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;
layout(location = 6) in int a_ClusterID;
out vec2 UV;
out vec4 frag_Color;
out vec3 worldPos;
out vec3 worldNormal;
out mat3 TBN;
flat out int v_Cluster;

uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;
uniform mat4 u_NormalMatrix;

//$VERT_FUNCTIONS
$VERT_INPUTS

void main(void)
{
    vec3 worldPositionOffset = $VERT_INPUT("World Position Offset", vec3, { vec3(0.0) });
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position + worldPositionOffset, 1.0);
	
	frag_Color = a_Color;
	UV = a_TexCoord;
	worldPos = vec3(u_Transform * vec4(a_Position + worldPositionOffset, 1.0));
	
	worldNormal = vec3(u_Transform * vec4(a_Normal, 0.0));
	//worldNormal = normalize(vec3(u_Transform * normalize(vec4(a_Position, 0.0))));

	vec3 T = normalize(vec3(u_Transform * vec4(normalize(a_Tangent), 0.0)));
	vec3 B = normalize(vec3(u_Transform * vec4(normalize(a_Bitangent), 0.0)));
	vec3 N = normalize(vec3(u_Transform * vec4(normalize(a_Normal), 0.0)));
	TBN = mat3(T, B, N);

	v_Cluster = a_ClusterID;
}

#type fragment
#version 330 core

in vec4 frag_Color;
in vec2 UV;
in vec3 worldPos;
in vec3 worldNormal;
in mat3 TBN;
flat in int v_Cluster;
// Output
layout(location = 0) out vec3 out_BaseColor;
layout(location = 1) out float out_Metallic;
layout(location = 2) out float out_Roughness;
layout(location = 3) out vec3 out_WorldPos;
layout(location = 4) out vec3 out_WorldNormal;
layout(location = 5) out vec3 out_Emissive;
layout(location = 6) out int out_MeshID;
layout(location = 7) out int out_Cluster;

uniform int u_MeshID;

//$FRAG_FUNCTIONS
$FRAG_INPUTS

// Normal, Position and UV
// From: https://gamedev.stackexchange.com/questions/86530/is-it-possible-to-calculate-the-tbn-matrix-in-the-fragment-shader
mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 SampleNormal(vec3 n, float flip)
{
	n = normalize(n * 2.0 - vec3(1.0)); 
	n = normalize(TBN * n); 
	if (flip == 1.0) n.g = 1.0 - n.g;
	return n;
}

void main(void)
{
	vec3 baseColor = $FRAG_INPUT("Base Color", vec3, { vec3(1.0) });
	float opacity = $FRAG_INPUT("Opacity", float, { 1.0 });
	
	if (opacity <= 0.05) 
	{
		discard;
		return;
	}

	/** Screendoor Transparency */
	mat4 threshold = mat4
    (
		1.0 / 17.0,   9.0 / 17.0,   3.0 / 17.0,   11.0 / 17.0,
		13.0 / 17.0,  5.0 / 17.0,   15.0 / 17.0,  7.0 / 17.0,
		4.0 / 17.0,   12.0 / 17.0,  2.0 / 17.0,   10.0 / 17.0,
		16.0 / 17.0,  8.0 / 17.0,   14.0 / 17.0,  6.0 / 17.0
    );
    int x = int(gl_FragCoord.x - 0.5);
    int y = int(gl_FragCoord.y - 0.5);
    if (opacity < threshold[x % 4][y % 4])
	{
		discard;
		return;
	}

	float metallic = $FRAG_INPUT("Metallic", float, { 0.0 });
	float roughness = $FRAG_INPUT("Roughness", float, { 0.5 });

	vec3 emissive = $FRAG_INPUT("Emissive Color", vec3, { vec3(0.0) });

	vec3 normal = $FRAG_INPUT("Normal", vec3, { vec3(0.0) });
	
	out_BaseColor = baseColor;
	out_Metallic = metallic;
	out_Roughness = roughness;
	out_WorldPos = worldPos;
	out_WorldNormal = normalize(worldNormal * 2.0 + normal);
	out_Emissive = emissive;
	out_MeshID = u_MeshID;
	out_Cluster = v_Cluster;
}