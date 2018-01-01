#pragma once

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
		GBUFFER_NUM_TEXTURES,
	};

	GBuffer();
	~GBuffer();

	bool Init(unsigned int windowWidth, unsigned int windowHeight);
	void BindForWrite();
	void BindForRead();
	void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

	GLuint GetDepthTexture()
	{
		return m_depthTexture;
	}

private:
	GLuint m_fbo = -1;
	GLuint m_textures[GBUFFER_NUM_TEXTURES];
	GLuint m_depthTexture = -1;
};
