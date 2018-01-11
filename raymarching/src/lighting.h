#pragma once

#ifndef __LIGHTING_H__
#define __LIGHTING_H__

#include "lights.h"
#include "glerror.h"
#include "../../common/GLEW/glew.h"
#include "../../common/glm/glm.hpp"
#include "../../common/glm/gtc/matrix_transform.hpp"


class PointLighting
{
public:
	PointLighting();
	~PointLighting();
	bool Init(unsigned int windowWidth, unsigned int windowHeight, PointLight p);
	void BindForWrite(unsigned int cubeFace, GLuint program);
	void BindForRead(unsigned int textureUnit);
	void BindLight();

private:

	struct CameraDirection
	{
		GLenum cubeFace;
		glm::vec3 target;
		glm::vec3 up;
	};

	CameraDirection m_cameraDirections[6] =
	{
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, -1.0f, 0.0f)},
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, -1.0f)},
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)	},
		{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f),	glm::vec3(0.0f, -1.0f, 0.0f)},
		{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
	};

	bool InitShadowMap(unsigned int windowWidth, unsigned int windowHeight);
	bool InitLight(PointLight p);

	GLuint m_shadowFBO;
	GLuint m_shadowMap;
	GLuint m_depth;
	GLuint m_pointLightUniform;

	PointLight m_PointLight;
};

#endif // !__LIGHTING_H__
