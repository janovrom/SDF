#version 400 core

//precision highp float;

in vec2 v_Vertex;
in vec3 v_Normal_worldspace;
in vec3 v_Pos_worldspace;

uniform sampler2D u_DepthTex;
uniform sampler2D u_PosTex;
uniform mat4 u_MVPMatrix;
uniform float u_Near;
uniform float u_Far;
uniform vec4 u_Times;
uniform vec3 u_EyeDirWorld;
uniform vec3 u_EyePosWorld;

layout(location = 0) out vec4 WorldPosOut;
layout(location = 1) out vec4 DiffuseOut;
layout(location = 2) out vec4 NormalOut;
//layout(location = 3) out vec3 DepthOut;

const float SAW[5] = {
	1.0, -1.0, 1.0, -1.0, 1.0
};

const vec3 SAND = vec3(1, 0.68, 0.38);
const vec3 DARK_SAND = vec3(0.58, 0.44, 0.1);
const vec3 STONES = vec3(0.66, 0.63, 0.55);
const vec3 SOIL = vec3(0.29, 0.22, 0.16);

//const vec3 COLORS[10] =		
//{
//	vec3(0.93, 0.79, 0.69)
//};


// Length functions
//-------------------------------------------------------------------------

float len4(vec3 p)
{
	vec3 p4 = pow(p, vec3(2));
	return pow(dot(p4, p4), 0.25);
}

float len4(vec2 p)
{
	vec2 p4 = pow(p, vec2(2));
	return pow(dot(p4, p4), (0.25));
}

float len8(vec3 p)
{
	vec3 p8 = pow(p, vec3(4));
	return pow(dot(p8, p8), (0.125));
}

float len8(vec2 p)
{
	vec2 p8 = pow(p, vec2(4));
	return pow(dot(p8, p8), (0.125));
}

//-------------------------------------------------------------------------


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
// Some adapted from : http://iquilezles.org/www/articles/distfunctions/distfunctions.html
//-------------------------------------------------------------------------


float sdTorus(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz) - t.x, p.y);
	return length(q) - t.y;
}

float sdCircle(vec3 p, float r)
{
	vec2 q = vec2(max(length(p.xz) - r, 0.0), p.y);
	return length(q);
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

float sawFunction(vec3 p)
{
	float val = mod(p.x, 4);
	float frac = fract(val);
	int idx = int(val);

	return mix(SAW[idx++], SAW[idx], frac);
}

float sawFunction(float x)
{
	float val = mod(x, 4);
	float frac = fract(val);
	int idx = int(val);

	return mix(SAW[idx++], SAW[idx], frac);
}

float opDisplaceGround(vec3 p)
{
	float y = max(min(p.y, 5.0), -15.0);
	//y *= y;
	float b = sdBox(p + vec3(0, 10, 10.0), vec3(25.0 + y, 5.0, 35.0 + (y)));
	p += cnoise(p.xz / 128.0)*18.0;
	//p += cnoise(p.xz / 16.0)*2.0;
	//p += cnoise(p.xz * 16.0) / 64.0;
	//p += cnoise(p.yy *.0) / 32.0;
	float d1 = sdPlane(p, vec4(0.0, 1.0, 0.0, 2.0));
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
	
	vec2 sinkPos = p.xz / 8.0;
	float sink = dot(sinkPos, sinkPos);
	sink = min(sink - 60.0, 0);
	float dy = cnoise(p.yx / 24.0) / 16.0;
	float d2 = sawFunction(dy / 4.0 +cnoise(p.xz / 512.0)) * min((length(p / 10.0)), 128.0);
	float d3 = sawFunction(dy + (p) / 128.0 + cnoise(dy + p.xz / 164.0 + vec2((cnoise(dy + p.xz/220.0)) / 120.0) * 32.0)) * 10.0;
	//d2 = sqrt(d2);
	//d3 = smin(dy, d3, 16.0);
	//d3 *= (cnoise(p.xz / 86.0) + 1.0);
	float sm = smin(d2, d3, 32.0);
	return smin(d1 + sm - (sink / 2.0), b, 16.0);
}

float opBlendBoxTorus(vec3 p)
{
	float d1 = sdBox(p, vec3(1.0, 1.5, 1.5));
	float d2 = sdTorus(p, vec2(10.5, 2.5));
	return smin(d1, d2, 32.0);
}

float opWater(vec3 p)
{
	float l = len4(p.xz) + length(p.xz);
	p.y += sin(l / 2.0 - u_Times.x * 2.0) / (l * 0.125);
	float d = sdCircle(p + vec3(0, 8.0, 0), 75.0);

	return d;
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Color mappings

vec3 sandColor(vec3 p)
{
	float noise = ((cnoise(p.xz * 16.0) * sin(p.x * 16.0)) + 2.0) / 4.0;
	float dnoise = ((cnoise(p.xz * 2.0) + cnoise(p.xz)) + 2.0) / 4.0;
	dnoise *= dnoise;
	vec3 sand = mix(SAND, DARK_SAND, 1.0-noise);
	vec3 soil = mix(SOIL, STONES, dnoise);
	float t = max(sign((-p.y)), 0.0) * max(0, (75-length(p.xz))) / 75.0;
	t *= t;
	return mix(sand, soil, t);
}

vec3 waterColor(vec3 p)
{
	return vec3(0.92, 0.96, 1.0);
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
	p.y -= 5.0;
	vec2 res = vec2(opWater(p), 0.0);
	//vec2 res = vec2(opTwistTorus(p, vec2(5, 1.5)), 46.7);
	//vec2 hp = vec2(opMapHexPrims(p, 4.0 * vec3(1.0, 1.0, 0.86)), 7.69);
	//vec2 hp = vec2(opRepBoxes(p, vec3(1.0)), 7.6);
	//vec2 hp = vec2(opRepHexPrisms(p, 4.0 * vec3(1.0, 1.0, 0.86)), 7.69);
	vec2 hp = vec2(opDisplaceGround(p), 1.0);
	//vec2 hp = vec2(sdPlane(p, vec4(0.0, 1.0, 0.0, 0.0)), 17.32);
	//vec2 hp = vec2(opGround(p), 17.32);
	float rs = res.x;
	float hx = hp.x;
	res = opUn(res, hp);
	res.x = smin(hx, rs, 0.2);
	return res;
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
}

int raymarch(vec3 ro, vec3 rd)
{
	vec4 color = vec4(0);

	const int maxstep = 64;
	const vec3 lightDir = normalize(vec3(1,-1,1));
	float t = 0;
	float depth = texelFetch(u_DepthTex, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0).x;
	float tmax = linearDepth(depth);
	vec4 wPos = texelFetch(u_PosTex, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0);
	float lastDist = 0;
	for (int i = 0; i < maxstep; ++i)
	{
		vec3 p = ro + rd * t;
		float precis = 0.001 * t;
		// Do the depth test
		//if (t * dot(rd, u_EyeDirWorld) > tmax)
		float dist = length(p - u_EyePosWorld);
		if (wPos.a > 0 && dist > length(wPos.xyz - u_EyePosWorld))
		{
			return -1;
		}
		vec2 d = map(p);
		if (abs(d.x) < precis)
		{
			vec3 n = normal(p, t);
			//vec3 col = 0.45 + 0.35*abs(sin(vec3(0.05, 0.08, 0.10))*(d.y - 1.0));
			vec4 col = vec4(1.0,0.8,0.9, 1.0);
			if(d.y < 1.0)
			{
				col = vec4(waterColor(p), 0.75);
			}
			else
			{
				col = vec4(sandColor(p), 1.0);
			}
			WorldPosOut = vec4(p, 1.0);
			DiffuseOut = col;
			NormalOut = vec4(n,1.0);
			vec4 P = u_MVPMatrix * vec4(p, 1.0);
			float zc = P.z;
			float wc = P.w;
			float depthP = zc / wc;
			depthP = 0.5 * depthP + 0.5;
			gl_FragDepth = depthP;
			return 1;
			//return col;// *dot(-lightDir, n)
		}
		// Make adaptive step when going around flat object
		if (abs(lastDist - d.x) < 0.001)
			t += 4.0 * d.x;
		else
			t += d.x;
		lastDist = d.x;
	}

	WorldPosOut = color;
	DiffuseOut = vec4(0.82, 0.92, 0.93, 1.0);
	NormalOut = color;
	gl_FragDepth = 1.0;
	//return color;
	return 0;

}

void main() {
	//FragColor = vec4((v_Vertex.x + 1) / 2.0, (v_Vertex.y + 1) / 2.0, 0.0, 0.2);
	//if (raymarch(v_Pos_worldspace, v_Normal_worldspace) < 0)
	//{
	//	discard;
	//}
	switch (raymarch(v_Pos_worldspace, v_Normal_worldspace))
	{
	case -1:
		//DiffuseOut = vec4(1, 0, 0, 1.0);
		discard;
		break;
	case 0:
		DiffuseOut = vec4(0, 1, 0, 1.0);
		break;
	case 1:
		//DiffuseOut = vec4(0, 0, 1, 1.0);
		break;
	default:
		break;
	}
	//DiffuseOut = vec3(0.82, 0.92, 0.93);
}
