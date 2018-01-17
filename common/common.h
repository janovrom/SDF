//-----------------------------------------------------------------------------
//  [PGR2] Common function definitions
//  27/02/2008
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_H__
#define __COMMON_H__

#include "GLEW/glew.h"
#include "GLEW/wglew.h"
#include "GLFW/glfw3.h"
#include <assert.h>
#include "../common/glm/glm.hpp"
#include "../common/glm/gtx/random.hpp"
#include "../common/glm/gtx/constants.hpp"
#include "../common/glm/gtc/matrix_transform.hpp"
#include "../common/glm/gtc/type_precision.hpp"
#include <chrono>
#include "tools.h"
#ifdef USE_ANTTWEAKBAR
#   include "../common/AntTweakBar/AntTweakBar.h"
#else
#   define TW_CALL
    typedef void* TwBar;
#endif

// FUNCTION POINTER TYPES______________________________________________________
/* Function pointer types */
typedef void (* TInitGLCallback)(void);
typedef void (* TInitGUICallback)(TwBar*);
typedef void (* TDisplayCallback)(void);
typedef void (* TWindowSizeChangedCallback)(const glm::ivec2&);
typedef void (* TMouseButtonChangedCallback)(int, int);
typedef void (* TMousePositionChangedCallback)(double, double);
typedef void (* TKeyboardChangedCallback)(int, int, int);

// FORWARD DECLARATIONS________________________________________________________
void TW_CALL compileShaders(void * = NULL);

// INTERNAL CONSTANTS__________________________________________________________
#define PGR2_SHOW_MEMORY_STATISTICS 0xF0000001
#define PGR2_DISABLE_VSYNC          0xF0000002
#define PGR2_DISABLE_BUFFER_SWAP    0xF0000004

// INTERNAL USER CALLBACK FUNCTION POINTERS____________________________________
namespace Callbacks {
    namespace GUI {
        //-----------------------------------------------------------------------------
        // Name: Set()
        // Desc: Default GUI "set" callback
        //-----------------------------------------------------------------------------
        template <typename T> void TW_CALL Set(const void* value, void* clientData) {
            *static_cast<T*>(clientData) = *static_cast<const T*>(value);
        }
        //-----------------------------------------------------------------------------
        // Name: Set()
        // Desc: Default GUI "set" callback
        //-----------------------------------------------------------------------------
        template <typename T> void TW_CALL SetCompile(const void* value, void* clientData) {
            *static_cast<T*>(clientData) = *static_cast<const T*>(value);
            compileShaders();
        }
        //-----------------------------------------------------------------------------
        // Name: Get()
        // Desc: Default GUI "get" callback
        //-----------------------------------------------------------------------------
        template <typename T> void TW_CALL Get(void* value, void* clientData) {
            *static_cast<T*>(value) = *static_cast<T*>(clientData);
        }
    }

    namespace User {
        TDisplayCallback              Display               = NULL;
        TWindowSizeChangedCallback    WindowSizeChanged     = NULL;
        TMouseButtonChangedCallback   MouseButtonChanged    = NULL;
        TMousePositionChangedCallback MousePositionChanged  = NULL;
        TKeyboardChangedCallback      KeyboardChanged       = NULL;
    } // end of namespace User

    //-----------------------------------------------------------------------------
    // Name: WindowSizeChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void WindowSizeChanged(GLFWwindow* window, int width, int height) {
        height = glm::max(height, 1);

        glViewport(0, 0, width, height);
        Variables::WindowSize = glm::ivec2(width, height);

        if (User::WindowSizeChanged)
            User::WindowSizeChanged(Variables::WindowSize);

        // Send the new window size to AntTweakBar
 #ifdef USE_ANTTWEAKBAR
        TwWindowSize(width, height);
        char settings[256] = {0};
        sprintf(settings, " Statistics position='%d 10'  ", Variables::WindowSize.x - 240);
        TwDefine(settings);
 #endif
    }


    //-----------------------------------------------------------------------------
    // Name: KeyboardChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void KeyboardChanged(GLFWwindow* window, int key, int scancode, int action, int mods) {
#ifndef USE_ANTTWEAKBAR
        if (action != GLFW_PRESS)
            return;

        switch(key) {
        case GLFW_KEY_Z: Variables::Shader::SceneZOffset += (mods == GLFW_MOD_SHIFT) ? -0.5f : 0.5f; return;
        case GLFW_KEY_I: Variables::Shader::Int += (mods == GLFW_MOD_SHIFT) ? -1 : 1; return;
        case GLFW_KEY_F: Variables::Shader::Float += (mods == GLFW_MOD_SHIFT) ? -0.01f : 0.01f; return;
        case GLFW_KEY_C: 
            printf("\n");
            compileShaders();
            return;
        }
#else
        if (mods == 1)
            key += 32;
        if (action == 2)
            action = 1;

        if (!TwEventCharGLFW(key, action))
 #endif
            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_ESCAPE)
                    exit(0);
      
                if (User::KeyboardChanged)
                    User::KeyboardChanged(key, action, mods);
            }
    }


    //-----------------------------------------------------------------------------
    // Name: MouseButtonChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void MouseButtonChanged(GLFWwindow* window, int button, int action, int mods) {
#ifdef USE_ANTTWEAKBAR
        TwEventMouseButtonGLFW(button, action);
#else
        Variables::MousePressed  = ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS));
         // Make mouse position invalid when mouse button pressed
        if (Variables::MousePressed)
            Variables::MousePosition = glm::ivec2(-1, -1);
#endif
    }


    //-----------------------------------------------------------------------------
    // Name: MousePositionChanged()
    // Desc: internal
    //-----------------------------------------------------------------------------
    void MousePositionChanged(GLFWwindow* window, double x, double y) {
 #ifdef USE_ANTTWEAKBAR
         TwEventMousePosGLFW(static_cast<int>(x + 0.5), static_cast<int>(y + 0.5));
 #else
        if (Variables::MousePosition.x < 0)
            Variables::MousePosition = glm::ivec2(static_cast<int>(x), static_cast<int>(y));

        if (Variables::MousePressed) {
            Variables::Shader::SceneRotation.x +=  0.9f * static_cast<float>(y - Variables::MousePosition.y);
            Variables::Shader::SceneRotation.y +=  0.9f * static_cast<float>(x - Variables::MousePosition.x);
            Variables::MousePosition = glm::ivec2(static_cast<int>(x), static_cast<int>(y));
        }
 #endif
    }

    //-----------------------------------------------------------------------------
    // Name: PrintOGLDebugLog()
    // Desc: 
    //-----------------------------------------------------------------------------
    void __stdcall PrintOGLDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar* message, const GLvoid* userParam) {
 #ifdef USE_ANTTWEAKBAR
        // Skip AntTweakBar GL error: API_ID_DEPRECATED_TARGET deprecated behaviour warning has been generated. Enable
        //                              of GL_LIGHTING capability is deprecated.
        if ((source == GL_DEBUG_SOURCE_API) && (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) && (id == 13))
            return;
#endif
        //if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        //    return;

        switch(source) {
        case GL_DEBUG_SOURCE_API            : printf("Source  : API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM  : printf("Source  : window system\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("Source  : shader compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY    : printf("Source  : third party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION    : printf("Source  : application\n"); break;
        case GL_DEBUG_SOURCE_OTHER          : printf("Source  : other\n"); break;
        default                             : printf("Source  : unknown\n"); break;
        }

        switch(type) {
        case GL_DEBUG_TYPE_ERROR              : printf("Type    : error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("Type    : deprecated behaviour\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : printf("Type    : undefined behaviour\n"); break;
        case GL_DEBUG_TYPE_PORTABILITY        : printf("Type    : portability issue\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE        : printf("Type    : performance issue\n"); break;
        case GL_DEBUG_TYPE_OTHER              : printf("Type    : other\n"); break;
        default                               : printf("Type    : unknown\n"); break;
        }

        printf("ID      : 0x%x\n", id);

        switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH  : printf("Severity: high\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: printf("Severity: medium\n"); break;
        case GL_DEBUG_SEVERITY_LOW   : printf("Severity: low\n"); break;
        default                      : printf("Severity: unknown\n"); break;
        }

        printf("Message : %s\n", message);
        printf("-------------------------------------------------------------------------------\n");
    }
}; // end of namespace Callbacks


//-----------------------------------------------------------------------------
// Name: common_main()
// Desc: 
//-----------------------------------------------------------------------------
int common_main(int window_width, int window_height, const char* window_title,
                int* opengl_config,
                TInitGLCallback               cbUserInitGL,
                TInitGUICallback              cbInitGUI,
                TDisplayCallback              cbUserDisplay,
                TWindowSizeChangedCallback    cbUserWindowSizeChanged,
                TKeyboardChangedCallback      cbUserKeyboardChanged,
                TMouseButtonChangedCallback   cbUserMouseButtonChanged,
                TMousePositionChangedCallback cbUserMousePositionChanged) {
    // Setup user callback functions
    assert(cbUserDisplay && cbUserInitGL);
    Callbacks::User::Display               = cbUserDisplay;
    Callbacks::User::WindowSizeChanged     = cbUserWindowSizeChanged;
    Callbacks::User::KeyboardChanged       = cbUserKeyboardChanged;
    Callbacks::User::MouseButtonChanged    = cbUserMouseButtonChanged;
    Callbacks::User::MousePositionChanged  = cbUserMousePositionChanged;
    
    // Setup internal variables
    Variables::WindowSize = glm::ivec2(window_width, window_height);

    // Setup temporary variables
    bool bDebugOutput      = false;
    bool bShowMemStat      = false;
    bool bDisableVSync     = false;
    bool bShowRotation     = false;
    bool bShowZOffset      = false;
    bool bAutoSwapDisabled = false;

    // Intialize GLFW   
    if (opengl_config != NULL) {
        while (*opengl_config != NULL) {
            glfwWindowHint(opengl_config[0], opengl_config[1]);
            if ((opengl_config[0] == GLFW_OPENGL_DEBUG_CONTEXT) && (opengl_config[1] == GL_TRUE))
                bDebugOutput = true;
            else if ((opengl_config[0] == PGR2_SHOW_MEMORY_STATISTICS) && (opengl_config[1] == GL_TRUE))
                bShowMemStat = true;
            else if ((opengl_config[0] == PGR2_DISABLE_VSYNC) && (opengl_config[1] == GL_TRUE))
                bDisableVSync = true;
            else if ((opengl_config[0] == PGR2_DISABLE_BUFFER_SWAP) && (opengl_config[1] == GL_TRUE))
                bAutoSwapDisabled = true;

            opengl_config += 2;
        }
    }

    // Create a window
    glfwInit();
    Variables::Window = glfwCreateWindow(Variables::WindowSize.x, Variables::WindowSize.y, window_title, NULL, NULL);
    glfwSetWindowPos(Variables::Window, 100, 100);

    glfwMakeContextCurrent(Variables::Window);

    glfwSetInputMode(Variables::Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        printf("Error: %s\n", glewGetErrorString(err));
        assert(0);
        return 1;
    }
    
    // Print debug info
    printf("VENDOR  : %s\nVERSION : %s\nRENDERER: %s\nGLSL    : %s\n\n", glGetString(GL_VENDOR), 
                                                                         glGetString(GL_VERSION), 
                                                                         glGetString(GL_RENDERER),
                                                                         glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Enable OGL debug 
    if (bDebugOutput && glfwExtensionSupported("GL_ARB_debug_output")) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Callbacks::PrintOGLDebugLog, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    // Disable VSync if required
    if (bDisableVSync)
        printf((glfwExtensionSupported("WGL_EXT_swap_control") && wglSwapIntervalEXT(0)) ? "VSync was disabled.\n" : "VSync couldn't be disabled!\n");

    // Check 
    if (bShowMemStat) {
        bShowMemStat = glfwExtensionSupported("GL_NVX_gpu_memory_info") == GLFW_TRUE;
        if (bShowMemStat) {
            glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &Statistic::GPUMemory::DedicatedMemory);
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &Statistic::GPUMemory::TotalMemory);
        }
    }

    // Init OGL
    if (cbUserInitGL)
        cbUserInitGL();

    glGenQueries(OpenGL::NUM_QUERIES, OpenGL::Query);

    // Set GLFW event callbacks
    glfwSetWindowSizeCallback(Variables::Window, Callbacks::WindowSizeChanged);
    glfwSetMouseButtonCallback(Variables::Window, Callbacks::MouseButtonChanged);
    glfwSetCursorPosCallback(Variables::Window, Callbacks::MousePositionChanged);
    glfwSetKeyCallback(Variables::Window, Callbacks::KeyboardChanged);
  
    // Init GUI
 #ifdef USE_ANTTWEAKBAR
    // Initialize AntTweakBar GUI
    if (!TwInit(TW_OPENGL, NULL)) {
        assert(0);
        return 2;
    }
    
    TwWindowSize(Variables::WindowSize.x, Variables::WindowSize.y);
    TwBar *menu = TwNewBar("Controls");
    char settings[256] = {0};
    sprintf(settings, " Controls position='10 10' size='270 %d' valueswidth=80 refresh=0.1 ", OpenGL::programs.empty() ? 230 : 280);
    TwDefine(settings);
    TwAddButton(menu, "compile_shaders", compileShaders, NULL, " group='Render' label='compile shaders' key=C help='Compile shader program.'");
     if (cbInitGUI)
       cbInitGUI(menu);

    // Scene panel setup
    for(int i = 0; i < OpenGL::programs.size(); i++) {
        const OpenGL::Program& program = OpenGL::programs[i];
        bShowRotation = (program.hasMVMatrix()) ? true : bShowRotation;
        bShowZOffset  = (program.hasZOffset()) ? true : bShowZOffset;
    }
    if (bShowRotation)
        TwAddVarRW(menu, "scene_rotation", TW_TYPE_QUAT4F, &Variables::Shader::SceneRotation, " group='Scene' label='rotation' open help='Toggle scene orientation.' ");
    if (bShowZOffset || bShowRotation)
        TwAddVarRW(menu, "offset_z", TW_TYPE_FLOAT, &Variables::Shader::SceneZOffset, " group='Scene' label='offset z' min=0 max=1000 step=0.5 keyIncr=Z keyDecr=z help='Scene translation.' ");

    TwAddVarRW(menu, "int_user_variable", TW_TYPE_INT32, &Variables::Shader::Int, " group='Shader variables' label='Use noise texture' min=0 max=1 step=1 keyIncr=I keyDecr=i help='User integer variable.' ");
	TwAddVarRW(menu, "int_user_variable_2", TW_TYPE_INT32, &Variables::Shader::Int2, " group='Shader variables' label='Switch sin off' min=0 max=1 step=1 keyIncr=F keyDecr=f help='Switch sine to interpolation.' ");
	TwAddVarRW(menu, "int_user_variable_3", TW_TYPE_INT32, &Variables::Shader::RR, " group='Shader variables' label='Enable refle/refra' min=0 max=1 step=1 keyIncr=F keyDecr=f help='Enable reflection and refraction for water.' ");
	TwAddVarRW(menu, "int_user_variable_4", TW_TYPE_INT32, &Variables::Shader::RMShadows, "group='Shader variables' label='Raymarch: shadow maps' min=0 max=1 step=1");
	TwAddVarRW(menu, "int_user_variable_5", TW_TYPE_INT32, &Variables::Shader::CloudShadows, "group='Shader variables' label='Cloud shadows' min=0 max=1 step=1");

	TwAddVarRW(menu, "user_stat_bool0", TW_TYPE_BOOL8, &Variables::Menu::ShowLight, "group='User variables' label='Show point lights' ");
	TwAddVarRW(menu, "user_stat_bool1", TW_TYPE_BOOL8, &Variables::Menu::ShowBuffers, "group='User variables' label='Show buffers' ");
	TwAddVarRW(menu, "user_stat_bool2", TW_TYPE_BOOL8, &Variables::Menu::ShowStats, "group='User variables' label='Update statistics' ");

    TwBar *menuStat = TwNewBar("Statistics");
    sprintf(settings, " Statistics position='%d 10' size='%d %d' valueswidth=70 refresh=0.1 ", Variables::WindowSize.x - 240, 230, 300);
    TwDefine(settings);
    TwAddVarRO(menuStat, "frame_id", TW_TYPE_INT32, &Statistic::Frame::ID, "group='Frame statistic' label='ID' ");
    TwAddVarRO(menuStat, "frame_gpu_time", TW_TYPE_INT32, &Statistic::Frame::GPUTime, "group='Frame statistic' label='GPU time' ");
    TwAddVarRO(menuStat, "frame_cpu_time", TW_TYPE_INT32, &Statistic::Frame::CPUTime, "group='Frame statistic' label='CPU time' ");
    
	TwAddVarRO(menuStat, "raymarch_geom", TW_TYPE_INT32, &Statistic::Frame::RaymarchGeom, "group='Geometric statistic' label='Raymarching'");
	TwAddVarRO(menuStat, "rasterize_geom", TW_TYPE_INT32, &Statistic::Frame::RasterizeGeom, "group='Geometric statistic' label='Rasterize'");

	TwAddVarRO(menuStat, "dir_Shadow", TW_TYPE_INT32, &Statistic::Frame::DirShadows, "group='Shadow statistic' label='Directional'");
	TwAddVarRO(menuStat, "point_Shadow", TW_TYPE_INT32, &Statistic::Frame::PointShadows, "group='Shadow statistic' label='Point'");

	TwAddVarRO(menuStat, "dir_lights", TW_TYPE_INT32, &Statistic::Frame::DirLights, "group='Light statistic' label='Directional'");
	TwAddVarRO(menuStat, "point_lights", TW_TYPE_INT32, &Statistic::Frame::PointLights, "group='Light statistic' label='Point'");
	
	if (bShowMemStat) {
        TwAddVarRO(menuStat, "dedicated_gpu_mem", TW_TYPE_INT32, &Statistic::GPUMemory::DedicatedMemory, "group='GPU memory' label='dedicated' ");
        TwAddVarRO(menuStat, "total_gpu_mem", TW_TYPE_INT32, &Statistic::GPUMemory::TotalMemory, "group='GPU memory' label='total' ");
        TwAddVarRO(menuStat, "used_gpu_mem", TW_TYPE_INT32, &Statistic::GPUMemory::AllocatedMemory, "group='GPU memory' label='allocated' ");
        TwAddVarRO(menuStat, "free_gpu_mem", TW_TYPE_INT32, &Statistic::GPUMemory::FreeMemory, "group='GPU memory' label='free' ");
    }
#else
    if (cbInitGUI)
        cbInitGUI(NULL);
#endif // USE_ANTTWEAKBAR

    // Main loop
    GLuint64 gpu_frame_start;
    GLuint64 gpu_frame_end;
    while (!glfwWindowShouldClose(Variables::Window)) {
        // Update auxiliary variables
        if (Callbacks::User::Display) {
            // Increase frame counter
            Statistic::Frame::ID++;

            // Update default shader variables
            GLint current_program = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);

            // Update transformations and default variables if used
            {
                using namespace Variables::Transformation;
                ModelView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Variables::Shader::SceneZOffset));
#ifdef USE_ANTTWEAKBAR
                const float STHETA = float(sin(acos(Variables::Shader::SceneRotation.w)));
                if (fabs(STHETA) > 0.00001f)
                    ModelView = glm::rotate(ModelView, 114.592f * acos(Variables::Shader::SceneRotation.w), glm::vec3(Variables::Shader::SceneRotation)*STHETA);
#else
                ModelView = glm::rotate(ModelView, Variables::Shader::SceneRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                ModelView = glm::rotate(ModelView, Variables::Shader::SceneRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
#endif
                ModelViewInverse = glm::inverse(ModelView);
                Projection = glm::perspective(60.0f, float(Variables::WindowSize.x)/Variables::WindowSize.y, 0.1f, 1000.0f);
                ModelViewProjection = Projection * ModelView;
				// Get eye direction
				glm::vec3 eye = glm::normalize(-glm::vec3(-ModelView[0][2], -ModelView[1][2], -ModelView[2][2]));
				glm::mat3x3 rotMat(ModelView);
				glm::vec3 d(ModelView[3]);
				glm::vec3 eyePos = -d * rotMat;
				//printf("Eye pos %f, %f, %f\n", eyePos.x, eyePos.y, eyePos.z);
    
				for (size_t i = 0; i < OpenGL::programs.size(); i++) {
                    const OpenGL::Program& program = OpenGL::programs[i];
                    glUseProgram(program.id);
                    if (program.MVPMatrix > -1) 
                        glUniformMatrix4fv(program.MVPMatrix, 1, GL_FALSE, &ModelViewProjection[0][0]);
                    if (program.ModelViewMatrix > -1) 
                        glUniformMatrix4fv(program.ModelViewMatrix, 1, GL_FALSE, &ModelView[0][0]);
                    if (program.ModelViewMatrixInv > -1) 
                        glUniformMatrix4fv(program.ModelViewMatrix, 1, GL_FALSE, &ModelViewInverse[0][0]);
                    if (program.ProjectionMatrix > -1) 
                        glUniformMatrix4fv(program.ProjectionMatrix, 1, GL_FALSE, &Projection[0][0]);
                    if (program.ZOffset > -1)
                        glUniform1f(program.ZOffset, Variables::Shader::SceneZOffset);
                    if (program.UserVariableInt > -1) 
                        glUniform1i(program.UserVariableInt, Variables::Shader::Int);
					if (program.UserVariableInt2 > -1)
						glUniform1i(program.UserVariableInt2, Variables::Shader::Int2);
                    if (program.UserVariableFloat > -1) 
                        glUniform1f(program.UserVariableFloat, Variables::Shader::Float);
                    if (program.FrameCounter > -1) 
                        glUniform1i(program.FrameCounter, Statistic::Frame::ID);
					if (program.Near > -1)
						glUniform1f(program.Near, 0.1f);
					if (program.Far > -1)
						glUniform1f(program.Far, 1000.0f);
					if (program.EyeDir > -1)
						glUniform3fv(program.EyeDir, 1, &eye.x);
					if (program.EyePos > -1)
						glUniform3fv(program.EyePos, 1, &eyePos.x);
                }
            }
            glUseProgram(current_program);

            // Clean the pipeline
            glFinish();

            const std::chrono::high_resolution_clock::time_point cpu_start = std::chrono::high_resolution_clock::now();
            glQueryCounter(OpenGL::Query[OpenGL::FRAME_START], GL_TIMESTAMP);
            Callbacks::User::Display();
            glQueryCounter(OpenGL::Query[OpenGL::FRAME_END], GL_TIMESTAMP);
            const std::chrono::high_resolution_clock::time_point cpu_end = std::chrono::high_resolution_clock::now();
            Statistic::Frame::CPUTime = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>( cpu_end - cpu_start ).count());

            // Get memory statistics if required
            if (bShowMemStat) {
                glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &Statistic::GPUMemory::FreeMemory);
                Statistic::GPUMemory::AllocatedMemory = Statistic::GPUMemory::TotalMemory - Statistic::GPUMemory::FreeMemory;
            }
        }

#ifdef USE_ANTTWEAKBAR
        // Draw tweak bars
        TwDraw();
#endif
        glGetQueryObjectui64v(OpenGL::Query[OpenGL::FRAME_START], GL_QUERY_RESULT, &gpu_frame_start);
        glGetQueryObjectui64v(OpenGL::Query[OpenGL::FRAME_END], GL_QUERY_RESULT, &gpu_frame_end);
        Statistic::Frame::GPUTime = static_cast<int>(gpu_frame_end - gpu_frame_start) / 1000000;
#ifndef USE_ANTTWEAKBAR
        printf("\rCPU time: %d, GPU time: %d", Statistic::Frame::CPUTime, Statistic::Frame::GPUTime);
#endif
        // Present frame buffer
        if (!bAutoSwapDisabled)
            glfwSwapBuffers(Variables::Window);

        glfwPollEvents();
    }
 
    // Terminate AntTweakBar and GLFW
#ifdef USE_ANTTWEAKBAR
    TwTerminate();
#endif
    // Terminate GLFW
    glfwTerminate();

    return 0;
} 


//-----------------------------------------------------------------------------
// Name: common_main()
// Desc: 
//-----------------------------------------------------------------------------
int common_main(int window_width, int window_height, const char* window_title,
                TInitGLCallback               cbUserInitGL,
                TInitGUICallback              cbInitGUI,
                TDisplayCallback              cbUserDisplay,
                TWindowSizeChangedCallback    cbUserWindowSizeChanged,
                TKeyboardChangedCallback      cbUserKeyboardChanged,
                TMouseButtonChangedCallback   cbUserMouseButtonChanged,
                TMousePositionChangedCallback cbUserMousePositionChanged) {
    return common_main(window_width, window_height, 
                       window_title,
                       NULL,
                       cbUserInitGL,
                       cbInitGUI,
                       cbUserDisplay,
                       cbUserWindowSizeChanged,
                       cbUserKeyboardChanged,
                       cbUserMouseButtonChanged,
                       cbUserMousePositionChanged);
}

#endif // !__COMMON_H__