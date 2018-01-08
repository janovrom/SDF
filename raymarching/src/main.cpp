#define USE_ANTTWEAKBAR
#include "../../common/common.h"
#include "../../common/models/elephant.h"
#include "../../common/models/screenquad.h"
#include "../../common/models/sphere.h"
#include "gbuffer.h"
#include "scene.h"
#include "glerror.h"

const unsigned int WINDOW_WIDTH		= 800;
const unsigned int WINDOW_HEIGHT	= 600;

bool		g_WireMode				= false;
GLuint		g_Program;
GLuint		g_RaymarchingProgram;
GLuint		g_PointLightProgram;
GLuint		g_ModelVBO;
GLuint		g_ModelVAO;
glm::vec3	g_Color					= glm::vec3(1, 0, 0);
GBuffer		m_gbuffer;

double		g_Time;
 
void updateUserData()
{
}

void DSGeometryPass()
{
	m_gbuffer.BindForWrite();
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
	

	   
	//// Draw screen quad for raymarching 
	//glUseProgram(g_RaymarchingProgram); 
	////m_gbuffer.BindForRead();   
	//glActiveTexture(GL_TEXTURE0);  
	//glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetDepthTexture(  ));
	//glUniform1i(glGetUniformLocation(g_RaymarchingProgram, "u_DepthTex"), 0);
	//glUniform4f(glGetUniformLocation(g_RaymarchingProgram, "u_Times"), glfwGetTime(), g_Time * 1000.0f, g_Time, g_Time * g_Time);
	//Tools::DrawScreenQuad(); 
	//glBindTexture(GL_TEXTURE_2D,  0);  
	//glUseProgram(0);  

	// GBuffer is filled, stencil needs it and shouldn't change it
	glDepthMask(GL_FALSE);   
	glDisable(GL_DEPTH_TEST); 
}

void BeginLightPasses()
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	m_gbuffer.BindForRead();
	glClear(GL_COLOR_BUFFER_BIT);
}

void DSPointLightPass()
{
	glUseProgram(g_PointLightProgram);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION));
	printOpenGLError();
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_COLOR));
	printOpenGLError();
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetTexture(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL));
	for (unsigned int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		/*glUniform1f(glGetUniformLocation(g_PointLightProgram, "u_Radius"), PLightsRadii[i]);
		printOpenGLError();
		glUniform3fv(glGetUniformLocation(g_PointLightProgram, "u_Center"), 1, &PLights[i].pos.x);
		printOpenGLError();*/
		glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_PosTex"), 0);
		glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_ColTex"), 1);
		glUniform1i(glGetUniformLocation(g_PointLightProgram, "u_NormTex"), 2);
		printOpenGLError();
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, PLights[i]);
		printOpenGLError();
		Tools::DrawSphere();
	}
	glUseProgram(0);
}

void DSLightPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GLsizei QWidth = (GLsizei)(WINDOW_WIDTH / 4.0f);
	GLsizei QHeight = (GLsizei)(WINDOW_HEIGHT / 4.0f);

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

	DSGeometryPass();
	BeginLightPasses();
	DSPointLightPass();
	printOpenGLError();
	DSLightPass();

	glReadBuffer(0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	printOpenGLError();
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

	// Create vertex array buffer object
	glGenVertexArrays(1, &g_ModelVAO);
	glBindVertexArray(g_ModelVAO);
	glGenBuffers(1, &g_ModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_ModelVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Tools::Mesh::ELEPHANT_VERTEX_AND_NORMAL_ARRAY_4D_STREAM), Tools::Mesh::ELEPHANT_VERTEX_AND_NORMAL_ARRAY_4D_STREAM, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	 
	// Load scene objects
	LoadScene();

	g_Time = glfwGetTime(); // in seconds
}

void TW_CALL compileShaders(void *clientData)
{
	// Create shader program object
	Tools::Shader::CreateShaderProgramFromFile(g_Program, "vertex.vs", NULL, NULL, NULL, "fragment.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_PointLightProgram, "light-pass.vs", NULL, NULL, NULL, "pointlight-pass.fs");
	Tools::Shader::CreateShaderProgramFromFile(g_RaymarchingProgram, "raymarching.vs", NULL, NULL, NULL, "raymarching.fs");
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