#version 400

struct PointLight
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
	float constant;
	float linear;
	float exp;
	vec3 pos;
	float radius;
};

uniform sampler2D u_PosTex;
uniform sampler2D u_ColTex;
uniform sampler2D u_NormTex;
uniform samplerCube u_ShadowCube;
uniform vec3 u_EyePosWorld;
layout(std140, binding = 1) uniform PointLightBlock
{
	PointLight pLight;
};

out vec4 FragColor;

float CalcShadowFactor(vec3 lightToPoint)
{
	float depth = texture(u_ShadowCube, (lightToPoint)).r;

	if (length(lightToPoint) < depth + 0.0001)
	{
		return 1.0;
	}
	else
	{
		return 0.25;
	}
}

vec4 CalcLightInternal(PointLight light, vec3 lightDirection, vec3 worldPos, vec3 normal)
{
	float shadowFactor = CalcShadowFactor(worldPos - light.pos);
	vec4 ambientColor = vec4(light.color * light.ambientIntensity, 1.0);
	float diffuseFactor = dot(normal, -lightDirection);

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

vec4 CalcPointLight(vec3 worldPos, vec3 normal)
{
	vec3 lightDirection = worldPos - pLight.pos;
	float d = length(lightDirection);
	lightDirection = normalize(lightDirection);

	vec4 color = CalcLightInternal(pLight, lightDirection, worldPos, normal);

	float attenuation = pLight.constant + pLight.linear * d + pLight.exp * d * d;
	attenuation = max(1.0, attenuation);

	return color / attenuation;
}


void main()
{
	vec3 worldPos	= texelFetch(u_PosTex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 color		= texelFetch(u_ColTex, ivec2(gl_FragCoord.xy), 0);
	vec3 normal		= texelFetch(u_NormTex, ivec2(gl_FragCoord.xy), 0).xyz;
	normal = normalize(normal);

	FragColor = color*CalcPointLight(worldPos, normal);
	//FragColor = vec4(color, 1.0);
}