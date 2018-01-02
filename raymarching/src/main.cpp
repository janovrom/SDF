#define USE_ANTTWEAKBAR
#include "../../common/common.h"
#include "../../common/models/elephant.h"
#include "../../common/models/screenquad.h"
#include "../include/gbuffer.h"

const unsigned int WINDOW_WIDTH		= 800;
const unsigned int WINDOW_HEIGHT	= 600;

bool		g_WireMode				= false;
GLuint		g_Program;
GLuint		g_RaymarchingProgram;
GLuint		g_ModelVBO;
GLuint		g_ModelVAO;
glm::vec3	g_Color					= glm::vec3(1, 0, 0);
GBuffer		m_gbuffer;


#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{

	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		printf("glError %u in file %s @ line %d: %s\n", glErr,
			file, line, gluErrorString(glErr));
		retCode = 1;
	}
	return retCode;
}


void updateUserData()
{
}

void DSGeometryPass()
{
	m_gbuffer.BindForWrite();
	printOpenGLError();

	// Clear frame buffer and set OpenGL states
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printOpenGLError();
	  
	glPolygonMode(GL_FRONT_AND_BACK, g_WireMode ? GL_LINE : GL_FILL);
	printOpenGLError();
	 
	// Update shader program  
	glUseProgram(g_Program); 
	glUniform1i(glGetUniformLocation(g_Program, "u_NumModels"), 1); 
	printOpenGLError();  

	// Specify model geometry and send it to GPU
	glBindVertexArray(g_ModelVAO);
	// TODO: Replace by instance rendering - use one OGL call glDrawArraysInstanced (instead of many glDrawArrays)
	//       to render many models. Update vertex shader to calculate location (translation) and color of every model.
	glUniform4fv(glGetUniformLocation(g_Program, "u_Color"), 1, &g_Color.x);
	//glUniform3f(glGetUniformLocation(g_Program, "u_Translate"), 1.0f - g_NumModels + 2.0f*iColumn, 0.0f, 1.0f - g_NumModels + 2.0f*iRow);
	glDrawArrays(GL_TRIANGLES, 0, Tools::Mesh::NUM_ELEPHANT_INDICES);
	printOpenGLError();   
	 
	glBindVertexArray(0);  
	glUseProgram(0);  
	printOpenGLError();
	   
	// Draw screen quad for raymarching 
	//glDisable(GL_DEPTH_TEST); 
	//glDepthMask(GL_FALSE);   
	glUseProgram(g_RaymarchingProgram); 
	//m_gbuffer.BindForRead();   
	glActiveTexture(GL_TEXTURE0);  
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetDepthTexture(  ));
	glUniform1i(glGetUniformLocation(g_RaymarchingProgram, "u_DepthTex"), 0);
	Tools::DrawScreenQuad();
	glBindTexture(GL_TEXTURE_2D,  0);  
	glUseProgram(0);  
	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);   

}

void DSLightPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	printOpenGLError();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printOpenGLError();

	m_gbuffer.BindForRead();
	printOpenGLError();

	GLsizei HalfWidth = (GLsizei)(WINDOW_WIDTH / 2.0f);
	GLsizei HalfHeight = (GLsizei)(WINDOW_HEIGHT / 2.0f);

	m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	printOpenGLError();

	m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_COLOR);
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		0, HalfHeight, HalfWidth, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	printOpenGLError();

	m_gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
	printOpenGLError(); 
	glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		HalfWidth, HalfHeight, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	printOpenGLError();
	  
	glBindTexture(GL_TEXTURE_2D, m_gbuffer.GetDepthTexture());
	Tools::Texture::Show2DTexture(m_gbuffer.GetDepthTexture(), HalfWidth, 0, HalfWidth, HalfHeight);
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
	printOpenGLError();
	DSLightPass();

	glReadBuffer(0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	printOpenGLError();
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
}

void TW_CALL compileShaders(void *clientData)
{
	// Create shader program object
	Tools::Shader::CreateShaderProgramFromFile(g_Program, "vertex.vs", NULL, NULL, NULL, "fragment.fs");
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