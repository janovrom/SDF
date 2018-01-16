#version 400 core

//precision highp float;

in vec2 v_Vertex;
in vec3 v_Normal_worldspace;
in vec3 v_Pos_worldspace;

uniform sampler2D u_DepthTex;
uniform sampler2D u_PosTex;
uniform sampler2D u_NoiseTex;
uniform mat4 u_MVPMatrix;
uniform float u_Near;
uniform float u_Far;
uniform vec4 u_Times;
uniform vec3 u_EyeDirWorld;
uniform vec3 u_EyePosWorld;
uniform int u_UserVariableInt;
uniform int u_UserVariableInt2;

layout(location = 0) out vec4 WorldPosOut;
layout(location = 1) out vec4 DiffuseOut;
layout(location = 2) out vec4 NormalOut;
//layout(location = 3) out vec3 DepthOut;

const float SAW[5] = {
	1.0, -1.0, 1.0, -1.0, 1.0
};

const float SINUS[9] = {
	0.0, 0.5, 0.707, 0.866, 1.0, 0.866, 0.707, 0.5, 0.0
};

const vec3 SAND = vec3(1, 0.68, 0.38);
const vec3 DARK_SAND = vec3(0.58, 0.44, 0.1);
const vec3 STONES = vec3(0.66, 0.63, 0.55);
const vec3 SOIL = vec3(0.29, 0.22, 0.16);
const vec3 SKY = vec3(0.12, 0.45, 0.99);

//const vec3 COLORS[10] =		
//{
//	vec3(0.93, 0.79, 0.69)
//};

float Sine(float p)
{
	float val = mod(4.0 * p / 3.1415, 8.0);
	float frac = fract(val);
	int idx = int(val);

	return mix(SINUS[idx++], SINUS[idx], frac);
}

float Noise2D(vec2 p)
{
	return texture(u_NoiseTex, p).r;
}

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

//	Classic Perlin 3D Noise 
//	by Stefan Gustavson
//
vec4 permute(vec4 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }
vec3 fade(vec3 t) { return t*t*t*(t*(t*6.0 - 15.0) + 10.0); }

float cnoise(vec3 P)
{
	vec3 Pi0 = floor(P); // Integer part for indexing
	vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
	Pi0 = mod(Pi0, 289.0);
	Pi1 = mod(Pi1, 289.0);
	vec3 Pf0 = fract(P); // Fractional part for interpolation
	vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
	vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
	vec4 iy = vec4(Pi0.yy, Pi1.yy);
	vec4 iz0 = Pi0.zzzz;
	vec4 iz1 = Pi1.zzzz;

	vec4 ixy = permute(permute(ix) + iy);
	vec4 ixy0 = permute(ixy + iz0);
	vec4 ixy1 = permute(ixy + iz1);

	vec4 gx0 = ixy0 / 7.0;
	vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
	gx0 = fract(gx0);
	vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
	vec4 sz0 = step(gz0, vec4(0.0));
	gx0 -= sz0 * (step(0.0, gx0) - 0.5);
	gy0 -= sz0 * (step(0.0, gy0) - 0.5);

	vec4 gx1 = ixy1 / 7.0;
	vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
	gx1 = fract(gx1);
	vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
	vec4 sz1 = step(gz1, vec4(0.0));
	gx1 -= sz1 * (step(0.0, gx1) - 0.5);
	gy1 -= sz1 * (step(0.0, gy1) - 0.5);

	vec3 g000 = vec3(gx0.x, gy0.x, gz0.x);
	vec3 g100 = vec3(gx0.y, gy0.y, gz0.y);
	vec3 g010 = vec3(gx0.z, gy0.z, gz0.z);
	vec3 g110 = vec3(gx0.w, gy0.w, gz0.w);
	vec3 g001 = vec3(gx1.x, gy1.x, gz1.x);
	vec3 g101 = vec3(gx1.y, gy1.y, gz1.y);
	vec3 g011 = vec3(gx1.z, gy1.z, gz1.z);
	vec3 g111 = vec3(gx1.w, gy1.w, gz1.w);

	vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
	g000 *= norm0.x;
	g010 *= norm0.y;
	g100 *= norm0.z;
	g110 *= norm0.w;
	vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
	g001 *= norm1.x;
	g011 *= norm1.y;
	g101 *= norm1.z;
	g111 *= norm1.w;

	float n000 = dot(g000, Pf0);
	float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
	float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
	float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
	float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
	float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
	float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
	float n111 = dot(g111, Pf1);

	vec3 fade_xyz = fade(Pf0);
	vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
	vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
	float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
	return 2.2 * n_xyz;
}

//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec2 fade(vec2 t) { return t*t*t*(t*(t*6.0 - 15.0) + 10.0); }
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

float sdCylinder(vec3 p, vec2 h)
{
	vec2 d = abs(vec2(length(p.xz), p.y)) - h;
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdCircle(vec3 p, float r)
{
	return sdCylinder(p, vec2(r, 0.5));
	//vec2 q = vec2(max(length(p.xz) - r, 0.0), p.y);
	//return length(q);
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
	float b = sdBox(p + vec3(0, 10, 15.0), vec3(25.0 + y, 5.0, 40.0 + (y)));
	float d1 = sdPlane(p, vec4(0.0, 1.0, 0.0, 2.0));
	vec2 sinkPos = p.xz / 8.0;
	float sink = dot(sinkPos, sinkPos);
	sink = min(sink - 60.0, 0);
	float dy = 0.0;
	float d2 = 0.0;
	float d3 = 0.0;

#ifdef NOISE_TEXTURE
	p += Noise2D((p.xz + 0.5) / 128.0) * 18.0;
	dy = Noise2D(p.yx / 24.0) / 16.0;
	d2 = sawFunction(dy / 4.0 + Noise2D(p.xz / 512.0)) * min((length(p / 10.0)), 128.0);
	d3 = sawFunction(dy + (p) / 128.0 + Noise2D(dy + p.xz / 164.0 + vec2((Noise2D(dy + p.xz / 220.0)) / 120.0) * 32.0)) * 10.0;
#else
	p += cnoise(p.xz / 128.0)*18.0;
	dy = cnoise(p.yx / 24.0) / 16.0;
	d2 = sawFunction(dy /4.0 +cnoise(p.xz / 512.0)) * min((length(p / 10.0)), 128.0);
	d3 = sawFunction(dy + (p) / 128.0 + cnoise(dy + p.xz / 164.0 + vec2((cnoise(dy + p.xz/220.0)) / 120.0) * 32.0)) * 10.0;
#endif
	
	float sm = smin(d2, d3, 32.0);
	//return d1 + sm - (sink / 2.0);
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
	p.y += SINE(l / 2.0 - u_Times.x * 2.0) / (l * 0.125);
	float d = sdCircle(p + vec3(0, 8.0, 0), 75.0);

	return d;
}
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Color mappings

float IntersectSphereRay(vec3 ro, vec3 rd)
{
	// ray to center
	float dotProduct = dot(-ro, rd);
	// We add some size approximation for light.
	float d = dotProduct*dotProduct - dot(-ro, -ro) + 1000.0*1000.0;

	if (d < 0)
		return -1.0;

	float t = dotProduct - sqrt(d);

	if (t < 0)
	{
		t = (dotProduct + sqrt(d));
		if (t < 0)
			return -1.0;
	}

	return t;
}

float IntersectEllipsoidRay(vec3 ro, vec3 rd, vec3 axis)
{
	ro = ro / axis;
	rd = rd / axis;
	// ray to center
	float dotProduct = dot(ro, rd);
	float d = dotProduct*dotProduct - dot(ro, ro);

	if (d < 0)
		return -1.0;

	float t = dotProduct - sqrt(d);

	if (t < 0)
	{
		t = (dotProduct + sqrt(d));
		if (t < 0)
			return -1.0;
	}

	return t;
}

float IntersectPlaneRay(vec3 ro, vec3 rd)
{
	return -(dot(ro, vec3(0, -1, 0)) + 1000.0) / dot(rd, vec3(0, -1, 0));
}

vec3 sandColor(vec3 p)
{
	float noise = 0.0;
	float dnoise = 0.0;
#ifdef NOISE_TEXTURE
	p /= 24.0;
	noise = ((Noise2D(p.xz / 16.0) * SINE(p.x / 16.0)) + 2.0) / 4.0;
	dnoise = ((Noise2D(p.xz / 2.0) + Noise2D(p.xz)) + 2.0) / 4.0;
#else
	noise = ((cnoise(p.xz * 16.0) * SINE(p.x * 16.0)) + 2.0) / 4.0;
	dnoise = ((cnoise(p.xz * 2.0) + cnoise(p.xz)) + 2.0) / 4.0;
#endif

	dnoise *= dnoise;
	vec3 sand = mix(SAND, DARK_SAND, 1.0-noise);
	vec3 soil = mix(SOIL, STONES, dnoise);
	float l = length(p.xz);
#ifdef NOISE_TEXTURE
		l *= 24.0;
#endif
	float t = max(sign((-p.y)), 0.0) * max(0, (75-l)) / 75.0;
	t *= t;
	return mix(sand, soil, t);
}

vec3 waterColor(vec3 p)
{
	return vec3(0.92, 0.96, 1.0);
}

vec3 cloudColor(vec3 p)
{
	vec2 q = p.xz / sqrt(p.y);
	float noise = 0.0;
#ifdef NOISE_TEXTURE
	noise = Noise2D(q / 500.0 + u_Times[0] / 1000.0) + Noise2D(q / 100.0 + u_Times[0] / 2000.0) + Noise2D(q / 300.0 + u_Times[0] / 6000.0);
#else
	noise = cnoise(q / 50.0 + u_Times[0] / 10.0) + cnoise(q / 10.0 + u_Times[0] / 20.0) + cnoise(q / 30.0 + u_Times[0] / 60.0);
#endif
	
	noise = (noise + 3.0) / 3.0;
	noise = clamp(noise - 0.75, 0, 1);
	return mix(SKY, vec3(1.0), noise);
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
	vec2 hp = vec2(opDisplaceGround(p), 1.0);
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

vec2 mapRefra(vec3 p)
{
	p.y -= 5.0;
	return vec2(opDisplaceGround(p), 1.0);
}

vec3 raymarchReflected(vec3 ro, vec3 rd, vec3 defaultColor)
{
	// Didn't hit anything so hit the bounding sphere
	float st = (IntersectSphereRay(ro, rd));
	if (st > 0)
	{
		// We hit the skybox
		vec3 p = ro + st * rd;
		WorldPosOut = vec4(vec3(1000.0), 1.0);
		return cloudColor(p);
	}
	else
	{
		return vec3(1.0, 0,0);
	}
}

// Raymarch only over SDF
vec3 raymarchRefracted(vec3 ro, vec3 rd, vec3 defaultColor)
{
	const int maxstep = 8;
	float t = 0;
	float lastDist = 0;
	float omega = 1.4;
	float step = 0.0;
	for (int i = 0; i < maxstep; ++i)
	{
		vec3 p = ro + rd * t;
		// No hit, go to skybox
		if (length(p) > 1500.0)
			break;

		vec2 d = mapRefra(p);
		// Use relaxation
		//bool relaxFailed = omega > 1.0 && (abs(d.x) + lastDist) < step;
		//if (relaxFailed)
		//{
		//	t -= step;
		//	omega = 1.0;
		//	continue;
		//}
		//else
		//{
		//	step = (d.x) * omega;
		//	omega *= 1.0055;
		//}
		step = d.x;
		lastDist = abs(d.x);
		float precis = abs(lastDist) / t;
		if (0.002 > precis)
		{
			// I have got a hit
			vec4 col = vec4(1.0, 0.8, 0.9, 1.0);
			if (d.y < 0.5)
			{
				return vec3(1.0,0,0);
			}
			else
			{
				return sandColor(p);
			}
		}
		// Make adaptive step when going around flat object
		t += step;
	}

	return defaultColor;
}

float schlickApproximation(vec3 v, vec3 n, float n1, float n2)
{
	float cosTheta = dot(v, n);
	float r0 = pow((n1 - n2) / (n1 + n2), 2.0);
	return clamp(r0 + (1 - r0) * pow(1 - cosTheta, 5), 0.0, 1.0);
}

int raymarch(vec3 ro, vec3 rd)
{
	vec4 color = vec4(0);

	const int maxstep = 64;
	float t = 0;
	float depth = texelFetch(u_DepthTex, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0).x;
	float tmax = linearDepth(depth);
	vec4 wPos = texelFetch(u_PosTex, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0);
	float lastDist = 0;
	float omega = 1.4;
	float step = 0.0;
	for (int i = 0; i < maxstep; ++i)
	{
		vec3 p = ro + rd * t;
		if (length(p) > 1500.0)
			break;


		// Do the depth test
		float dist = length(p - u_EyePosWorld);
		//if (t * dot(rd, u_EyeDirWorld) > tmax)
		if (wPos.a > 0 && dist > length(wPos.xyz - u_EyePosWorld))
		{
			return -1;
		}
		vec2 d = map(p);
		// Use relaxation
		bool relaxFailed = omega > 1.0 && (abs(d.x) + lastDist) < step;
		if (relaxFailed)
		{
			t -= step;
			omega = 1.0;
			continue;
		}
		else
		{
			step = d.x * omega;
			omega *= 1.0055;
		}
		lastDist = abs(d.x);
		float precis = abs(lastDist) / t;
		if (0.002 > precis)
		{
			vec3 n = normal(p, t);
			//vec3 col = 0.45 + 0.35*abs(sin(vec3(0.05, 0.08, 0.10))*(d.y - 1.0));
			vec4 col = vec4(1.0,0.8,0.9, 1.0);
			if(d.y < 0.5)
			{
				// Water has reflection and refraction - do it and accumulate
#ifdef REFLE_REFRA
				vec3 refracted = normalize(refract(-rd, n, 1.3333));
				vec3 reflected = normalize(reflect(-rd, n));
				vec3 colWater = waterColor(p);
				vec3 colRefra = raymarchRefracted(p + refracted * 0.1, refracted, colWater) * colWater;
				vec3 colRefle = raymarchReflected(p + reflected * 0.1, reflected, colWater);
				float st = schlickApproximation(-rd, n, 1.00029, 1.3333);
				vec3 colResult = mix(colRefra, colRefle, st);
				col = vec4(colResult, 1.0);
#else
				col = vec4(waterColor(p), 1.0);
#endif
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
		t += step;
	}

	// Didn't hit anything so hit the bounding sphere
	float st = abs(IntersectSphereRay(ro, rd));
	if (st > 0)
	{
		// We hit the skybox
		vec3 p = ro + st * rd;
		WorldPosOut = vec4(vec3(1000.0), 1.0);
		DiffuseOut = vec4(cloudColor(p), 1.0);
		NormalOut = vec4(vec3(0,1.0,0), 1.0);
		gl_FragDepth = 1.0;
		return 1;
	}
	else
	{
		WorldPosOut = color;
		DiffuseOut = vec4(1,0,0, 1.0);
		NormalOut = color;
		gl_FragDepth = 1.0;
		return 0;
	}
}

void main() {
	switch (raymarch(v_Pos_worldspace, normalize(v_Normal_worldspace)))
	{
	case -1:
		discard;
		break;
	case 0:
		DiffuseOut = vec4(0, 1, 0, 1.0);
		break;
	case 1:
		break;
	default:
		break;
	}
}
