#pragma once

#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__

#include "../../common/GLEW/glew.h" 
#include "../../common/glm/glm.hpp" 
#include "../../common/glm/gtc/matrix_transform.hpp"
#include "lights.h"
#include "glerror.h"

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