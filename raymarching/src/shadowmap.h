#pragma once

#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include "../../common/GLEW/glew.h" 
#include "../../common/glm/glm.hpp" 
#include "../../common/glm/gtc/matrix_transform.hpp"
#include "lights.h"
#include "glerror.h"

#define SHADOW_CUBE_MAP_SIZE 64

struct CameraDirection
{
	GLenum cubeFace;
	glm::vec3 target;
	glm::vec3 up;
};

const CameraDirection CameraDirections[6] =
{
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f),	glm::vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, 1.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
	{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f),	glm::vec3(0.0f, -1.0f, 0.0f) },
	{ GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) }
};

class ShadowCube
{
public:
	ShadowCube();
	~ShadowCube();

	bool Init(unsigned int shadowMapWidth, unsigned int shadowMapHeight, PointLight p);
	void BindForWrite(GLuint program, unsigned int cubeFace);
	void BindForRead(unsigned int texUnit, GLuint program);
	GLuint GetCubeTexture();
	glm::vec3& GetLightPosition();

private:	

	GLuint m_fbo;
	GLuint m_depthTex;
	GLuint m_cubeTex;
	PointLight m_light;
	glm::mat4x4 m_views[6];
	glm::mat4x4 m_persp;
};

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	bool Init(unsigned int shadowMapWidth, unsigned int shadowMapHeight, DirectionalLight d);
	void BindForWrite(GLuint program);
	void BindForRead(unsigned int  texUnit, GLuint program);
	GLuint GetTexture();

private:
	GLuint m_fbo;
	GLuint m_depthTex;
	DirectionalLight m_light;
	glm::mat4x4 m_ortho;
	glm::mat4x4 m_view;
};

#endif // !__SHADOW_MAP_H__