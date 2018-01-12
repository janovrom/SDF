#include "shadowmap.h"

bool ShadowMap::Init(unsigned int shadowMapWidth, unsigned int shadowMapHeight, DirectionalLight d)
{
	m_light = d;
	m_ortho = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, -200.0f, 200.0f);
	m_view = glm::lookAt(glm::vec3(0), m_light.dir, glm::normalize(glm::cross(glm::vec3(1.0,0,0),m_light.dir)));

	// Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Create depth texture
	glGenTextures(1, &m_depthTex);
	glBindTexture(GL_TEXTURE_2D, m_depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	float color[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

	// No read and write to color channels 	
	glDrawBuffer(GL_NONE); 	
	//glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); 	
	if (status != GL_FRAMEBUFFER_COMPLETE) 
	{ 
		printf("FB error, status: 0x%x\n", status); 		
		return false; 
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void ShadowMap::BindForWrite(GLuint program)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	printOpenGLError();
	glUniformMatrix4fv(glGetUniformLocation(program, "u_LightView"), 1, GL_FALSE, &m_view[0][0]);
	printOpenGLError();
	glUniformMatrix4fv(glGetUniformLocation(program, "u_LightProjection"), 1, GL_FALSE, &m_ortho[0][0]);
	printOpenGLError();
}

void ShadowMap::BindForRead(unsigned int texUnit, GLuint program)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	printOpenGLError();
	glBindTexture(GL_TEXTURE_2D, m_depthTex);
	printOpenGLError();
	glUniformMatrix4fv(glGetUniformLocation(program, "u_LightView"), 1, GL_FALSE, &m_view[0][0]);
	printOpenGLError();
	glUniformMatrix4fv(glGetUniformLocation(program, "u_LightProjection"), 1, GL_FALSE, &m_ortho[0][0]);
	printOpenGLError();
	glUniform1i(glGetUniformLocation(program, "u_lightDepth"), texUnit);
	printOpenGLError();
}

GLuint ShadowMap::GetTexture()
{
	return m_depthTex;
}

ShadowMap::ShadowMap(){}
ShadowMap::~ShadowMap(){}