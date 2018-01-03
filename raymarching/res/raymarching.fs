#version 400 core

//precision highp float;

in vec2 v_Vertex;
in vec3 v_Normal_worldspace;
in vec3 v_Pos_worldspace;

uniform sampler2D u_DepthTex;
uniform mat4 u_MVPMatrix;
uniform float u_Near;
uniform float u_Far;
uniform vec4 u_Times;
uniform vec3 u_EyeDirWorld;

layout(location = 0) out vec3 WorldPosOut;
layout(location = 1) out vec3 DiffuseOut;
layout(location = 2) out vec3 NormalOut;
layout(location = 3) out vec3 DepthOut;


// Noise
//-------------------------------------------------------------------------

//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec2 fade(vec2 t) { return t*t*t*(t*(t*6.0 - 15.0) + 10.0); }
vec4 permute(vec4 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }
float cnoise(vec2 P)
{
	vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
	vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
	Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
	vec4 ix = Pi.xzxz;
	vec4 iy = Pi.yyww;
	vec4 fx = Pf.xzxz;
	vec4 fy = Pf.yyww;
	vec4 i = permute(permute(ix) + iy);
	vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
	vec4 gy = abs(gx) - 0.5;
	vec4 tx = floor(gx + 0.5);
	gx = gx - tx;
	vec2 g00 = vec2(gx.x, gy.x);
	vec2 g10 = vec2(gx.y, gy.y);
	vec2 g01 = vec2(gx.z, gy.z);
	vec2 g11 = vec2(gx.w, gy.w);
	vec4 norm = 1.79284291400159 - 0.85373472095314 *
		vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
	g00 *= norm.x;
	g01 *= norm.y;
	g10 *= norm.z;
	g11 *= norm.w;
	float n00 = dot(g00, vec2(fx.x, fy.x));
	float n10 = dot(g10, vec2(fx.y, fy.y));
	float n01 = dot(g01, vec2(fx.z, fy.z));
	float n11 = dot(g11, vec2(fx.w, fy.w));
	vec2 fade_xy = fade(Pf.xy);
	vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
	float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
	return 2.3 * n_xy;
}

////	Simplex 3D Noise 
////	by Ian McEwan, Ashima Arts
////
//vec4 permute(vec4 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }
//vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
//
//float snoise(vec3 v)
//{
//	const vec2  C = vec2(1.0 / 6.0, 1.0 / 3.0);
//	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
//
//	// First corner
//	vec3 i = floor(v + dot(v, C.yyy));
//	vec3 x0 = v - i + dot(i, C.xxx);
//
//	// Other corners
//	vec3 g = step(x0.yzx, x0.xyz);
//	vec3 l = 1.0 - g;
//	vec3 i1 = min(g.xyz, l.zxy);
//	vec3 i2 = max(g.xyz, l.zxy);
//
//	//  x0 = x0 - 0. + 0.0 * C 
//	vec3 x1 = x0 - i1 + 1.0 * C.xxx;
//	vec3 x2 = x0 - i2 + 2.0 * C.xxx;
//	vec3 x3 = x0 - 1. + 3.0 * C.xxx;
//
//	// Permutations
//	i = mod(i, 289.0);
//	vec4 p = permute(permute(permute(
//		i.z + vec4(0.0, i1.z, i2.z, 1.0))
//		+ i.y + vec4(0.0, i1.y, i2.y, 1.0))
//		+ i.x + vec4(0.0, i1.x, i2.x, 1.0));
//
//	// Gradients
//	// ( N*N points uniformly over a square, mapped onto an octahedron.)
//	float n_ = 1.0 / 7.0; // N=7
//	vec3  ns = n_ * D.wyz - D.xzx;
//
//	vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)
//
//	vec4 x_ = floor(j * ns.z);
//	vec4 y_ = floor(j - 7.0 * x_);    // mod(j,N)
//
//	vec4 x = x_ *ns.x + ns.yyyy;
//	vec4 y = y_ *ns.x + ns.yyyy;
//	vec4 h = 1.0 - abs(x) - abs(y);
//
//	vec4 b0 = vec4(x.xy, y.xy);
//	vec4 b1 = vec4(x.zw, y.zw);
//
//	vec4 s0 = floor(b0)*2.0 + 1.0;
//	vec4 s1 = floor(b1)*2.0 + 1.0;
//	vec4 sh = -step(h, vec4(0.0));
//
//	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
//	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;
//
//	vec3 p0 = vec3(a0.xy, h.x);
//	vec3 p1 = vec3(a0.zw, h.y);
//	vec3 p2 = vec3(a1.xy, h.z);
//	vec3 p3 = vec3(a1.zw, h.w);
//
//	//Normalise gradients
//	vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
//	p0 *= norm.x;
//	p1 *= norm.y;
//	p2 *= norm.z;
//	p3 *= norm.w;
//
//	// Mix final noise value
//	vec4 m = max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
//	m = m * m;
//	return 42.0 * dot(m*m, vec4(dot(p0, x0), dot(p1, x1),
//		dot(p2, x2), dot(p3, x3)));
//}
//-------------------------------------------------------------------------



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

float sdPlane(vec3 p, vec4 n)
{
	// n must be normalized
	return dot(p, n.xyz) + n.w;
}

float sdHexPrism(vec3 p, vec2 t)
{
	vec3 q = abs(p);
	return max(q.y - t.y, max(q.z, 0.5 * q.z + 0.866025 * q.x) - t.x);
}

float sdBox(vec3 p, vec3 b)
{
	vec3 d = abs(p) - b;
	return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

//-------------------------------------------------------------------------


// Domain operations
//-------------------------------------------------------------------------
float opRepHexPrisms(vec3 p, vec3 c)
{
	vec3 q = mod(p, c) - 0.5 * c;
	q.y = p.y + 2.0;
	q.z = mod(mod(p.x - q.x, 2.0 * c.x) * 0.5 * c.z + p.z, c.z) - 0.5 * c.z;
	q = p;
	return sdHexPrism(q, vec2(0.5 * c.x, (sin((p.z - q.z) + sin(p.x - q.x)) + 1) * 0.5));
}

float opRepBoxes(vec3 p, vec3 c)
{
	vec3 q = mod(p, c) - 0.5 * c.x;
	q.y = p.y + 2.0;
	c.y *= sin(p.x - q.x) + 2.0;
	c.y *= 20;
	return sdBox(q, c * 0.1);
}

float opMapHexPrims(vec3 p, vec3 c)
{
	// Hex prism by me-voronoi
	p = p / 5.0;
	vec2 q = floor(p).xz; // squares 5x5
	float h = sin(q.x) + sin(q.y);
	return max(p.y - h, 0.0);
	//vec2 tl = q +	vec2(1.0, -1.0) * 5.0;
	//vec2 t = q +	vec2(1.0, 0.0) * 5.0;
	//vec2 tr = q +	vec2(1.0, 1.0) * 5.0;
	//vec2 bl = q +	vec2(-1.0, -1.0) * 5.0;
	//vec2 b = q +	vec2(-1.0, 0.0) * 5.0;
	//vec2 br = q +	vec2(-1.0, 1.0) * 5.0;

	//float tmin = min(dot(tl, p.xz), min(dot(t, p.xz), dot(tr, p.xz)));
	//float bmin = min(dot(bl, p.xz), min(dot(b, p.xz), dot(br, p.xz)));
	//float pdis = dot(q, p.xz);
	//if (pdis < tmin && pdis < bmin)
	//{
	//	return max(sin(p.x), 5.0);
	//}
	//else
	//{
	//	return sqrt(min(tmin, bmin));
	//}
}
//-------------------------------------------------------------------------


// Distance deformations
//-------------------------------------------------------------------------

// Artifact: wrong normals
//float smin(float a, float b, float k)
//{
//	a = pow(a, k); b = pow(b, k);
//	return pow((a*b) / (a + b), 1.0 / k);
//}

// Artifact: adds bounding box
//float smin(float a, float b, float k)
//{
//	float res = exp(-k*a) + exp(-k*b);
//	return -log(res) / k;
//}

float opGround(vec3 p)
{
	return sin(p.x) + sin(p.z);
}

float smin(float a, float b, float k)
{
	//float s = 0.5 + 0.5*tanh(b);
	//return a * s + (1 - s) * b;
	float h = clamp(0.5 + 0.5*(b - a) / k, 0.0, 1.0);
	return mix(b, a, h) - k*h*(1.0 - h);
}

float opDisplaceGround(vec3 p)
{
	//p += cnoise(p.xz / 128.0)*8.0;
	//p += cnoise(p.xz / 256.0)*16.0;
	//p += cnoise(p.xz * 16.0) / 64.0;
	float d1 = sdPlane(p, vec4(0.0, 1.0, 0.0, 200.0));
	float maxDist = 128.0;
	float total = 0;
	float frequency = 1;
	float amplitude = 1;
	float persistence = 0.25;
	float maxValue = 0;  // Used for normalizing result to 0.0 - 1.0

	//if (d1 < 8.0)
	//{
	//	d1 += cnoise(p.xz / 4.0)/8.0;
	//}
	//d1 += /*snoise(p/32.0)+*/cnoise(p.xz/2.0)/8.0 + cnoise(p.xz / 4.0)/8.0 + cnoise(p.xz / 64.0)*16.0;
	float d2 = opGround(p);
	return d1 - d2;
}

float opBlendBoxTorus(vec3 p)
{
	float d1 = sdBox(p, vec3(1.0, 1.5, 1.5));
	float d2 = sdTorus(p, vec2(1.5, 0.5));
	return smin(d1, d2, 0.15);
}
//-------------------------------------------------------------------------


// Domain deformations
//-------------------------------------------------------------------------
float opTwistTorus(vec3 p, vec2 t)
{
	float c = cos(sin(u_Times.x) * 0.2*p.y + u_Times.x);
	float s = sin(sin(u_Times.x) * 0.2*p.y + u_Times.x);
	mat2  m = mat2(c, -s, s, c);
	vec3  q = vec3(m*p.xz, p.y);
	return sdTorus(q, t);
}
//-------------------------------------------------------------------------


// This is the distance field function.  The distance field represents the closest distance to the surface
// of any object we put in the scene.  If the given point (point p) is inside of an object, we return a
// negative answer.
vec2 map(vec3 p)
{
	
	vec2 res = vec2(opBlendBoxTorus(p), 46.7);
	//vec2 res = vec2(opTwistTorus(p, vec2(5, 1.5)), 46.7);
	//vec2 hp = vec2(opMapHexPrims(p, 4.0 * vec3(1.0, 1.0, 0.86)), 7.69);
	//vec2 hp = vec2(opRepBoxes(p, vec3(1.0)), 7.6);
	//vec2 hp = vec2(opRepHexPrisms(p, 4.0 * vec3(1.0, 1.0, 0.86)), 7.69);
	vec2 hp = vec2(opDisplaceGround(p), 17.32);
	res = opUn(res, hp);
	return hp;
}

vec3 normal(vec3 p, float precis)
{
	vec2 eps = vec2(0.0001 * precis, 0);
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
	//float S = 2 << 32 - 1;
	//return -S * u_Far * u_Near / (depthSample *(u_Far - u_Near) - u_Far * S);
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
		float precis = 0.01 * t;
		// Do the depth test
		if (t * dot(rd, u_EyeDirWorld) > tmax/*+ t * 0.001*/)
		{
			return -1;
		}
		vec2 d = map(p);
		if (d.x < precis)
		{
			vec3 n = normal(p, t);
			vec3 col = 0.45 + 0.35*abs(sin(vec3(0.05, 0.08, 0.10))*(d.y - 1.0));
			WorldPosOut = p;
			DiffuseOut = col;
			NormalOut = n;
			vec4 P = u_MVPMatrix * vec4(p, 1.0);
			float zc = P.z;
			float wc = P.w;
			float depthP = zc / wc;
			depthP = 0.5 * depthP + 0.5;
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
