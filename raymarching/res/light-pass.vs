#version 400


struct BaseLight
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct Attenuation
{
	float constant;
	float linear;
	float exp;
};


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

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Norm;
layout(location = 2) in vec2 a_Tex;

uniform mat4 u_MVPMatrix;
layout(std140, binding = 1) uniform PointLightBlock
{
	PointLight pLight;
};

void main()
{
	gl_Position = u_MVPMatrix * vec4(pLight.radius * a_Pos + pLight.pos, 1.0);
}