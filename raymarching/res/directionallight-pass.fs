#version 400

struct DirectionalLight
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
	vec3 dir;
};

uniform sampler2D u_PosTex;
uniform sampler2D u_ColTex;
uniform sampler2D u_NormTex;
uniform vec3 u_EyePosWorld;

layout(std140, binding = 0) uniform DirectionalLightBlock
{
	DirectionalLight dLight;
};

out vec4 FragColor;

vec4 CalcLightInternal(DirectionalLight light, vec3 lightDirection, vec3 worldPos, vec3 normal)
{
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

	return (ambientColor + diffuseColor + specularColor);
}

void main()
{
	vec3 worldPos = texelFetch(u_PosTex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 color = texelFetch(u_ColTex, ivec2(gl_FragCoord.xy), 0);
	vec3 normal = texelFetch(u_NormTex, ivec2(gl_FragCoord.xy), 0).xyz;
	normal = normalize(normal);

	FragColor = color*CalcLightInternal(dLight, dLight.dir, worldPos, normal);
	//FragColor = vec4(color, 1.0);
}