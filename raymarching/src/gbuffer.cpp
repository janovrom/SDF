#include "gbuffer.h"
#include <stdio.h>
#include "glerror.h"

bool GBuffer::Init(unsigned int windowWidth, unsigned int windowHeight)
{
	// Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Create textures for framebuffer
	glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
	glGenTextures(1, &m_depthTexture);
	glGenTextures(1, &m_finalTexture);

	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
	}

	// Create final texture
	glBindTexture(GL_TEXTURE_2D, m_finalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES, GL_TEXTURE_2D, m_finalTexture, 0);
	
	// Create depth texture
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

	GLuint DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

void GBuffer::StartFrame()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::BindForGeometryPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	GLuint DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);
	printOpenGLError();
}

void GBuffer::BindForStencilPass()
{
	// must disable the draw buffers
	glDrawBuffer(GL_NONE);
}

void GBuffer::BindForLightPass()
{
	// Draw to intermediate final texture
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
	// Bind textures
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}

void GBuffer::BindForFinalPass()
{
	// Draw on screen
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	// Read from buffer and last color attachment
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0 + GBUFFER_NUM_TEXTURES);
}

void GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}

GBuffer::GBuffer()
{

}

GBuffer::~GBuffer()
{

}