#version 440

struct DirectionalLight
{
	vec3 color;
	float ambientIntensity;
	vec3 dir;
	float diffuseIntensity;
};

uniform sampler2D u_PosTex;
uniform sampler2D u_ColTex;
uniform sampler2D u_NormTex;
uniform sampler2D u_lightDepth;
uniform vec3 u_EyePosWorld;
uniform mat4 u_LightView;
uniform mat4  u_LightProjection;
uniform vec4 u_Times;


layout(std140, binding = 2) uniform DirectionalLightBlock
{
	DirectionalLight dLight;
};

out vec4 FragColor;



//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec4 permute(vec4 x) { return mod(((x*34.0) + 1.0)*x, 289.0); }
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

float cloudColor(vec3 p)
{
	vec2 q = p.xz / sqrt(p.y);
	float noise = 0.0;
	noise = cnoise(q / 50.0 + u_Times[0] / 10.0) + cnoise(q / 10.0 + u_Times[0] / 20.0) + cnoise(q / 30.0 + u_Times[0] / 60.0);

	noise = (noise + 3.0) / 3.0;
	noise = clamp(noise-0.75, 0, 1);
	if (noise < 0.35)
		return 1.0;
	else
		return 1.0 - sin(noise - 0.35) * 4.0;
}

float CalcShadowFactor(vec4 pos_worldSpace, vec3 rd)
{
	vec4 pos_lightSpace = u_LightProjection * u_LightView * pos_worldSpace;
	vec3 proj = pos_lightSpace.xyz / pos_lightSpace.w;
	proj = 0.5 * proj + 0.5;
	proj = clamp(proj, 0.0, 1.0);
	// Depth for objects
	float depth = texture(u_lightDepth, proj.xy).r;
	// Depth for clouds
	float st = (IntersectSphereRay(pos_worldSpace.xyz, rd));
	float cloudFactor = 1.0;
#ifdef CLOUD_SHADOWS
	if (st > 0)
	{
		// We hit the skybox
		cloudFactor = cloudColor(pos_worldSpace.xyz + rd * st);
	}
#endif
	if (proj.z < depth + 0.0001)
	{
		return 1.0 * cloudFactor;
	}
	else
	{
		return 0.25 * cloudFactor;
	}
}

vec4 CalcLightInternal(DirectionalLight light, vec3 lightDirection, vec3 worldPos, vec3 normal)
{
	vec4 ambientColor = vec4(light.color * light.ambientIntensity, 1.0);
	float diffuseFactor = dot(normal, -lightDirection);
	float shadowFactor = CalcShadowFactor(vec4(worldPos, 1.0), -lightDirection);

	vec4 diffuseColor = vec4(0, 0, 0, 0);
	vec4 specularColor = vec4(0, 0, 0, 0);

	if (diffuseFactor > 0.0)
	{
		diffuseColor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0);

		vec3 toEye = normalize(u_EyePosWorld - worldPos);
		vec3 reflected = normalize(reflect(lightDirection, normal));
		float specularFactor = dot(toEye, reflected);
		if (specularFactor > 0.0)
		{
			float gMatSpecularIntensity = 1.0;
			float gSpecularPower = 16.0;
			specularFactor = pow(specularFactor, gSpecularPower);
			specularColor = vec4(light.color * gMatSpecularIntensity * specularFactor, 1.0);
		}
	}

	return (ambientColor + shadowFactor * (diffuseColor + specularColor));
}

void main()
{
	vec3 worldPos = texelFetch(u_PosTex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 color = texelFetch(u_ColTex, ivec2(gl_FragCoord.xy), 0);

	vec3 normal = texelFetch(u_NormTex, ivec2(gl_FragCoord.xy), 0).xyz;
	normal = normalize(normal);

	FragColor = color*CalcLightInternal(dLight, dLight.dir, worldPos, normal);
	//FragColor = u_LightProjection * u_LightView * vec4(worldPos, 1.0);
	//FragColor = vec4(vec3(dLight.dir), 1.0);
}