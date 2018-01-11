#define USE_ANTTWEAKBAR
#include "../../common/common.h"
#include "../../common/models/elephant.h"
#include "../../common/models/screenquad.h"
#include "../../common/models/sphere.h"
#include "gbuffer.h"
#include "scene.h"
#include "glerror.h"
#include "sdf.h"

const unsigned int WINDOW_WIDTH		= 800;
const unsigned int WINDOW_HEIGHT	= 600;

bool		g_WireMode				= false;
GLuint		g_Program;
GLuint		g_RaymarchingProgram;
GLuint		g_PointLightProgram;
GLuint		g_NullProgram;
GLuint		g_DirLightProgram;
GLuint		g_SphereVAO;
glm::vec3	g_Color					= glm::vec3(1, 0, 0);
GBuffer		m_gbuffer;
SDF			m_sdf;
double		g_Time;
 
void updateUserData()
{
}

void DSGeometryPass()
{
	m_gbuffer.BindForGeometryPass();
	printOpenGLError();

	// Updates to depth buffer only in geometry pass
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Clear frame buffer and set OpenGL states
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printOpenGLError();
	  
	glPolygonMode(GL_FRONT_AND_BACK, g_WireMode ? GL_LINE : GL_FILL);
	printOpenGLError();
	 
	// Update shader program  
	glUseProgram(g_Program); 
	RenderSceneGeometry(g_Program);
	glUseProgram(0);  
	printOpenGLError();
	glDisable(GL_BLEND);
	

	   
	// Draw screen quad for raymarching 
	glUseProgram(g_RaymarchingProgram); 
	//m_gbuffer.BindForRead();   
	glActiveTexture(GL_TEXTURE0);  
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetDepthTexture(  ));
	glUniform1i(glGetUniformLocation(g_RaymarchingProgram, "u_DepthTex"), 0);
	glUniform4f(glGetUniformLocation(g_RaymarchingProgram, "u_Times"), glfwGetTime(), g_Time * 1000.0f, g_Time, g_Time * g_Time);
	Tools::DrawScreenQuad(); 
	glBindTexture(GL_TEXTURE_2D,  0);  
	glUseProgram(0);  

	// GBuffer is filled, stencil needs it and shouldn't change it
	glDepthMask(GL_FALSE);   
	glDisable(GL_DEPTH_TEST); 
}

void DSStencilPass(unsigned int idx)
{
	glUseProgram(g_NullProgram);

	m_gbuffer.BindForStencilPass();
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, PLights[idx]);
	printOpenGLError();

	// Render both faces
	glDisable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Populate the stencil buffer for now, so always pass the test
	glStencilFunc(GL_ALWAYS, 0, 0);

	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	Tools::DrawSphere();
	printOpenGLError();

	glUseProgram(0);
}

void DSPointLightPass(unsigned int idx)
{
	glUseProgram(g_PointLightProgram);
	m_gbuffer.BindForLightPass();
	printOpenGLError();

	glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_PosTex"), 0);
	glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_ColTex"), 1);
	glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_NormTex"), 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, PLights[idx]);
	printOpenGLError();

	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Draw sphere
	Tools::DrawSphere();
	printOpenGLError();

	// Restore settings
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glUseProgram(0);
}

void DSRenderPointLights()
{
	// Enable stencil test for point in sphere test
	glEnable(GL_STENCIL_TEST);
	// Bind textures to texture units 0-2
	//m_gbuffer.BindForLightPass();
	// Bind uniform textures
	printOpenGLError();

	for (unsigned int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		// Handle DSStencilPass separately to prevent stencil overflow
		DSStencilPass(i);
		printOpenGLError();
		// Draw lights
		DSPointLightPass(i);
		printOpenGLError();
	}
	glUseProgram(0);
	// Disable stencil test for another passes
	glDisable(GL_STENCIL_TEST);
}

void DSDirectionalLightPass()
{
	glUseProgram(g_DirLightProgram);
	m_gbuffer.BindForLightPass();
	glUniform1i(glGetUniformLocation(g_DirLightProgram, "u_PosTex"), 0);
	glUniform1i(glGetUniformLocation(g_DirLightProgram, "u_ColTex"), 1);
	glUniform1i(glGetUniformLocation(g_DirLightProgram, "u_NormTex"), 2);
	for (unsigned int i = 0; i < NUM_DIRECTIONAL_LIGHTS; ++i)
	{
		printOpenGLError();
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, DLights[i]);
		printOpenGLError();
		Tools::DrawScreenQuad();
		printOpenGLError();

	}
	glUseProgram(0);
}

void DSFinalLightPass()
{
	m_gbuffer.BindForFinalPass();
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void DSIntermediateLightPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GLsizei QWidth = (GLsizei)(WINDOW_WIDTH / 4.0f);
	GLsizei QHeight = (GLsizei)(WINDOW_HEIGHT / 4.0f);
#ifdef COMPUTE_SDF
	{ // launch compute shaders!
		m_sdf.LaunchComputeShader(m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_COLOR));
	}
#endif
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_COLOR));
	Tools::Texture::Show2DTexture(m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_COLOR),
		WINDOW_WIDTH - QWidth, WINDOW_HEIGHT - 1 * QHeight, QWidth, QHeight);

	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION));
	Tools::Texture::Show2DTexture(m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION),
		WINDOW_WIDTH - QWidth, WINDOW_HEIGHT - 2 * QHeight, QWidth, QHeight);

	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL));
	Tools::Texture::Show2DTexture(m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL),
		WINDOW_WIDTH - QWidth, WINDOW_HEIGHT - 3 * QHeight, QWidth, QHeight);


	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetDepthTexture());
	Tools::Texture::Show2DTexture(m_gbuffer.GetDepthTexture(), 
		WINDOW_WIDTH - QWidth, WINDOW_HEIGHT - 4*QHeight, QWidth, QHeight);
	glBindTexture(GL_TEXTURE_2D, 0);  
	//printOpenGLError();
	//glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
	//	HalfWidth, 0, WINDOW_WIDTH, HalfHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	printOpenGLError();
}

void display()
{
	// Update user data if number of models changed
	updateUserData();

	m_gbuffer.StartFrame();
	DSGeometryPass();
	DSRenderPointLights();
	//DSDirectionalLightPass();
	printOpenGLError();
	DSFinalLightPass();
	DSIntermediateLightPass();

	//glReadBuffer(0);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//printOpenGLError();
	g_Time = glfwGetTime() - g_Time;
}

void initGL()
{
	// Init GBuffer
	m_gbuffer.Init(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Set default camera's distance from the scene
	Variables::Shader::SceneZOffset = 16.0f;

	// Set OpenGL state variables 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// Enable depth test
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); //Enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.

	// Load shader program
	compileShaders();

	// Load scene objects
	LoadScene();

	g_Time = glfwGetTime(); // in seconds
}

void TW_CALL compileShaders(void *clientData)
{
	// Create shader program object
	Tools::Shader::CreateShaderProgramFromFile(g_Program, "vertex.vs", NULL, NULL, NULL, "fragment.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_PointLightProgram, "pointlight-pass.vs", NULL, NULL, NULL, "pointlight-pass.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_NullProgram, "pointlight-pass.vs", NULL, NULL, NULL, "null-pass.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_DirLightProgram, "directionallight-pass.vs", NULL, NULL, NULL, "directionallight-pass.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_RaymarchingProgram, "raymarching.vs", NULL, NULL, NULL, "raymarching.fs");
	m_sdf.InitShader();
}

void initGUI(TwBar* menu)
{
#ifdef USE_ANTTWEAKBAR
	TwDefine(" Controls position='10 10' size='240 330' refresh=0.1 ");
#else
	printf("Controls:\n");
	printf("  [i/I]   ... inc/dec value of user integer variable\n");
	printf("  [f/F]   ... inc/dec value of user floating-point variable\n");
	printf("  [w]     ... toggle wire mode\n");
	printf("  [c]     ... recompile shaders\n");
#endif
}

void keyboardChanged(int key, int action, int mods)
{
#ifndef USE_ANTTWEAKBAR
	switch (key)
	{
	case GLFW_KEY_W: g_WireMode = !g_WireMode; break;
	case GLFW_KEY_M: g_NumModels = glm::clamp(g_NumModels + ((mods == GLFW_MOD_SHIFT) ? -1 : 1), 1, 1000); break;
	}
#endif
}

int main(int argc, char* argv)
{
	int OGL_CONFIGURATION[] = {
		GLFW_CONTEXT_VERSION_MAJOR, 4,
		GLFW_CONTEXT_VERSION_MINOR, 4,
		GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE,
		GLFW_OPENGL_DEBUG_CONTEXT,  GL_TRUE,
		GLFW_OPENGL_PROFILE,        /*GLFW_OPENGL_COMPAT_PROFILE*/ GLFW_OPENGL_CORE_PROFILE, 
		0
	};

	return common_main(WINDOW_WIDTH, WINDOW_HEIGHT, "[PGR2] Instanced rendering",
		OGL_CONFIGURATION, // OGL configuration hints
		initGL,            // Init GL callback function
		initGUI,           // Init GUI callback function
		display,           // Display callback function
		NULL,              // Window resize callback function
		keyboardChanged,   // Keyboard callback function
		NULL,              // Mouse button callback function
		NULL);             // Mouse motion callback function
}