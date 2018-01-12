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
uniform sampler2DShadow u_lightDepth;
uniform vec3 u_EyePosWorld;
uniform mat4 u_LightView;
uniform mat4  u_LightProjection;


layout(std140, binding = 2) uniform DirectionalLightBlock
{
	DirectionalLight dLight;
};

out vec4 FragColor;

float CalcShadowFactor(vec4 pos_lightSpace)
{
	vec3 proj = pos_lightSpace.xyz / pos_lightSpace.w;
	proj = 0.5 * proj + 0.5;
	return texture(u_lightDepth, proj);

}

vec4 CalcLightInternal(DirectionalLight light, vec3 lightDirection, vec3 worldPos, vec3 normal)
{
	vec4 ambientColor = vec4(light.color * light.ambientIntensity, 1.0);
	float diffuseFactor = dot(normal, -lightDirection);
	float shadowFactor = CalcShadowFactor(u_LightProjection * u_LightView * vec4(worldPos, 1.0));

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