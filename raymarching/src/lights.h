#pragma once

#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "../../common/glm/glm.hpp"

#define POINT_LIGHT				1
#define DIRECTIONAL_LIGHT		2

struct BaseLight
{
	glm::vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct Attenuation
{
	float constant;
	float linear;
	float exp;
};

struct DirectionalLight
{
	glm::vec3 color;
	float ambientIntensity;
	glm::vec3 dir;
	float diffuseIntensity;
};

struct PointLight
{
	BaseLight light;
	Attenuation attenuation;
	glm::vec3 pos;
	float radius;
};

inline float CalcPLightBSphere(const PointLight& light)
{
	float MaxChannel = fmax(fmax(light.light.color.x, light.light.color.y), light.light.color.z);

	float ret = (-light.attenuation.linear + sqrtf(light.attenuation.linear * light.attenuation.linear -
		4 * light.attenuation.exp * (light.attenuation.constant - 256 * MaxChannel * light.light.diffuseIntensity)))
		/ (2 * light.attenuation.exp);
	return ret;
}

#endif // !__LIGHTS_H__