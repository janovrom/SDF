#pragma once

#ifndef __GBUFFER_H__
#define __GBUFFER_H__

#include "../../common/GLEW/glew.h"
#include "../../common/GLEW/wglew.h"
#include "../../common/GLFW/glfw3.h"


class GBuffer
{
public:
	enum GBUFFER_TEXTURE_TYPE
	{
		GBUFFER_TEXTURE_TYPE_POSITION,
		GBUFFER_TEXTURE_TYPE_COLOR,
		GBUFFER_TEXTURE_TYPE_NORMAL,
		GBUFFER_NUM_TEXTURES
	};

	GBuffer();
	~GBuffer();

	bool Init(unsigned int windowWidth, unsigned int windowHeight);
	void BindForGeometryPass();
	void BindForStencilPass();
	void BindForLightPass();
	void BindForFinalPass();
	void StartFrame();
	void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

	GLuint GetDepthTexture()
	{
		return m_depthTexture;
	}

	GLuint GetTexture(GBUFFER_TEXTURE_TYPE TextureType)
	{
		return m_textures[TextureType];
	}

private:
	GLuint m_fbo = -1;
	GLuint m_textures[GBUFFER_NUM_TEXTURES];
	GLuint m_depthTexture = -1;
	GLuint m_finalTexture = -1;
};

#endif // !__GBUFFER_H__