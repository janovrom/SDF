#include "lighting.h"

bool PointLighting::Init(unsigned int windowWidth, unsigned int windowHeight, PointLight p)
{
	return InitShadowMap(windowWidth, windowHeight) && InitLight(p);
}

bool PointLighting::InitShadowMap(unsigned int windowWidth, unsigned int windowHeight)
{
	// Create FBO
	glGenFramebuffers(1, &m_shadowFBO);

	// Create the depth buffer
	glGenTextures(1, &m_depth);
	glBindTexture(GL_TEXTURE_2D, m_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create cube map
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	// Generate six faces of the cube map
	for (unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);

	// No read and write to color channels
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	printOpenGLError();
	return true;
}

bool PointLighting::InitLight(PointLight p)
{
	// Delete buffer if it already exists
	if (m_pointLightUniform != 0)
		glDeleteBuffers(1, &m_pointLightUniform);

	m_PointLight = p;
	glGenBuffers(1, &m_pointLightUniform);
	glBindBuffer(GL_UNIFORM_BUFFER, m_pointLightUniform);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight), (void*)&m_PointLight, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return true;
}

void PointLighting::BindForWrite(unsigned int cubeFace, GLuint program)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, m_shadowMap, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glm::mat4x4 View = glm::lookAt(m_PointLight.pos, m_cameraDirections[cubeFace].target, m_cameraDirections[cubeFace].up);
	glUniformMatrix4fv(glGetUniformLocation(program, "u_ModelViewMatrix"), 1, GL_FALSE, &View[0][0]);
}

void PointLighting::BindForRead(unsigned int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowMap);
}

void PointLighting::BindLight()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_pointLightUniform);
}

PointLighting::PointLighting() {}
PointLighting::~PointLighting(){}