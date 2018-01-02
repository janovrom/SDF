#version 400 core

precision highp float;

in vec2 v_Vertex;
in vec3 v_Normal_worldspace;
in vec3 v_Pos_worldspace;

uniform sampler2D u_DepthTex;
uniform mat4 u_MVPMatrix;
uniform float u_Near;
uniform float u_Far;

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;
layout(location = 3) out vec3 DepthOut;

// Operations over multiple models.
//-------------------------------------------------------------------------
vec2 opUn(vec2 d1, vec2 d2)
{
	return d1.x > d2.x ? d2 : d1;
}

float opIn(float d1, float d2)
{
	return max(d1, d2);
}

float opSub(float d1, float d2)
{
	return max(-d1, d2);
}
//-------------------------------------------------------------------------


// Model creation - distance functions.
//-------------------------------------------------------------------------

// Torus
// t.x: diameter
// t.y: thickness
// Adapted from: http://iquilezles.org/www/articles/distfunctions/distfunctions.html
float sdTorus(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}

float sdHexPrism(vec3 p, vec2 t)
{
	vec3 q = abs(p);
	return max(q.y - t.y, max(q.z, 0.5 * q.z + 0.866025 * q.x) - t.x);
}

//-------------------------------------------------------------------------


// Domain operations
//-------------------------------------------------------------------------
float opRepHexPrisms(vec3 p, vec3 c)
{
	vec3 q = mod(p, c) - 0.5 * c;
	q.y = p.y + 2.0;
	q.z = mod(mod(p.x - q.x, 2.0 * c.x) * 0.5 * c.z + p.z, c.z) - 0.5 * c.z;
	return sdHexPrism(q, vec2(0.5 * c.x, (sin((p.z - q.z) + sin(p.x - q.x)) + 1) * 0.5));
}
//-------------------------------------------------------------------------


// This is the distance field function.  The distance field represents the closest distance to the surface
// of any object we put in the scene.  If the given point (point p) is inside of an object, we return a
// negative answer.
vec2 map(vec3 p)
{
	vec2 res = vec2(sdTorus(p, vec2(5, 1.5)), 46.7);
	vec2 hp = vec2(opRepHexPrisms(p, 4.0 * vec3(1.0, 1.0, 0.86)), 7.69);
	res = opUn(res, hp);
	return res;
}

vec3 normal(vec3 p)
{
	vec2 eps = vec2(0.0001, 0);
	return normalize(vec3(
		map(p + eps.xyy).x - map(p - eps.xyy).x, 
		map(p + eps.yxy).x - map(p - eps.yxy).x, 
		map(p + eps.yyx).x - map(p - eps.yyx).x));
}

float linearDepth(float depthSample)
{
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * u_Near * u_Far / (u_Far + u_Near - depthSample * (u_Far - u_Near));
	return zLinear;
}

int raymarch(vec3 ro, vec3 rd)
{
	vec3 color = vec3(0);

	const int maxstep = 64;
	const vec3 lightDir = normalize(vec3(1,-1,1));
	float t = 0;
	float depth = texelFetch(u_DepthTex, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0).x;
	float tmax = linearDepth(depth);
	for (int i = 0; i < maxstep; ++i)
	{
		vec3 p = ro + rd * t;
		float precis = 0.0005 * t;
		// Do the depth test
		if (t > tmax + t * 0.001)
		{
			return -1;
		}
		vec2 d = map(p);
		if (d.x < precis)
		{
			vec3 n = normal(p);
			vec3 col = 0.45 + 0.35*abs(sin(vec3(0.05, 0.08, 0.10))*(d.y - 1.0));
			WorldPosOut = p;
			DiffuseOut = col;
			NormalOut = n;
			float zc = (u_MVPMatrix * vec4(p, 1.0)).z;
			float wc = (u_MVPMatrix * vec4(p, 1.0)).w;
			float depthP = zc / wc;
			gl_FragDepth = depthP;
			return 1;
			//return col;// *dot(-lightDir, n)
		}
		t += d.x;
	}

	WorldPosOut = color;
	DiffuseOut = color;
	NormalOut = color;
	gl_FragDepth = 1.0;
	//return color;
	return 1;

}

void main() {
	//FragColor = vec4((v_Vertex.x + 1) / 2.0, (v_Vertex.y + 1) / 2.0, 0.0, 0.2);
	if (raymarch(v_Pos_worldspace, v_Normal_worldspace) < 0)
	{
		discard;
	}
}
