//-----------------------------------------------------------------------------
//  [PGR2] Some helpful common functions
//  19/02/2014
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <math.h>
#include <string>
#include <vector>
#include "../common/glm/gtx/integer.hpp"
#include "../common/glm/core/func_exponential.hpp"
#include "models\scene_miro.h"

// INTERNAL VARIABLES DEFINITIONS______________________________________________
namespace Variables {
    GLFWwindow* Window        = NULL;
    glm::ivec2  WindowSize    = glm::ivec2(0);
    glm::ivec2  MousePosition = glm::ivec2(-1, -1);
    bool        MousePressed  = false;

    namespace Transformation { // Scene transformation matrixes ( readonly variables - calculated automatically every frame)
        glm::mat4 ModelView;
        glm::mat4 ModelViewInverse;
        glm::mat4 ModelViewProjection;
        glm::mat4 Projection;
    }

    namespace Shader {				// Shader default variables
		int		Int			= 0;    // Value will be automatically passed to 'u_UserVariableInt' uniform in all shaders
		int		Int2		= 0;    // Value will be automatically passed to 'u_UserVariableInt' uniform in all shaders
		int		RR			= 0;	// Use reflection and refraction
		int		RMShadows	= 0;	// Raymarched objects cast shadows
		int		CloudShadows= 0;	// Clouds cast shadows
        float	Float		= 0.0f; // Value will be automatically passed to 'u_UserFloatInt' uniform in all shaders

        glm::vec4 SceneRotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);    // Scene orientation
        GLfloat   SceneZOffset  = 2.0f;                                 // Scene translation along z-axis
    }; // end of namespace Variables::Shader

    namespace Menu {
        int		Int			=    0;     // Value will be automatically shown in the sub-menu 'Statistic'
        float	Float		= 0.0f;     // Value will be automatically shown in the sub-menu 'Statistic'
        int		Int1		=    0;     // Value will be automatically shown in the sub-menu 'Statistic'
        float	Float1		= 0.0f;     // Value will be automatically shown in the sub-menu 'Statistic'
		bool	ShowLight	= true;
		bool	ShowBuffers = true;
		bool	ShowStats	= false;
    }; // end of namespace Variables::Menu
}; // end of namespace Variables

namespace Statistic {
    namespace Frame {
        int   GPUTime   = 0;        // GPU frame time
        int   CPUTime		= 0;        // CPU frame time
		int   ID				= 0;    // Id of the frame
		unsigned int RaymarchGeom	= 0;
		unsigned int RasterizeGeom	= 0;
		unsigned int DirShadows			= 0;        
		unsigned int PointShadows		= 0;
		unsigned int DirLights				= 0;
		unsigned int PointLights			= 0;
    }; // end of namespace Statistic::Frame

    namespace GPUMemory {
        int DedicatedMemory = 0;    // Size of dedicated GPU memory
        int TotalMemory     = 0;    // Size of total GPU memory
        int AllocatedMemory = 0;    // Size of allocated GPU memory
        int FreeMemory      = 0;    // SIze of available GPU memory
    }; // end of namespace Statistic::GPUMemory
}; // end of namespace Statistic

namespace OpenGL {
    enum eQueries {
        FRAME_START = 0,
        FRAME_END,
        NUM_QUERIES
    };

    struct Program {
        Program(GLuint _id) : id(_id) {
            MVPMatrix			= glGetUniformLocation(id, "u_MVPMatrix");
            ModelViewMatrix		= glGetUniformLocation(id, "u_ModelViewMatrix");
            ModelViewMatrixInv	= glGetUniformLocation(id, "u_ModelViewMatrixInverse");
            ProjectionMatrix	= glGetUniformLocation(id, "u_ProjectionMatrix");
            ZOffset				= glGetUniformLocation(id, "u_ZOffset");
			UserVariableInt		= glGetUniformLocation(id, "u_UserVariableInt");
			UserVariableInt2	= glGetUniformLocation(id, "u_UserVariableInt2");
            UserVariableFloat	= glGetUniformLocation(id, "u_UserVariableFloat");
            FrameCounter		= glGetUniformLocation(id, "u_FrameCounter");
			Near				= glGetUniformLocation(id, "u_Near");
			Far					= glGetUniformLocation(id, "u_Far");
			EyeDir				= glGetUniformLocation(id, "u_EyeDirWorld");
			EyePos				= glGetUniformLocation(id, "u_EyePosWorld");
        }
        bool hasMVMatrix() const {
            return (MVPMatrix > -1) || (ModelViewMatrix > -1) || (ModelViewMatrixInv > -1);
        }
        bool hasZOffset() const {
            return (ZOffset > -1);
        }
        GLuint id;
        GLint  MVPMatrix;
        GLint  ModelViewMatrix;
        GLint  ModelViewMatrixInv;
        GLint  ProjectionMatrix;
        GLint  ZOffset;
        GLint  UserVariableInt;
		GLint	UserVariableInt2;
        GLint  UserVariableFloat;
        GLint  FrameCounter;
		GLint	Near;
		GLint	Far;
		GLint	EyeDir;
		GLint	EyePos;
    };
    std::vector<Program> programs;

    GLuint Query[NUM_QUERIES] = {0};
}; // end of namespace OpenGL


namespace Tools {
    //-----------------------------------------------------------------------------
    // Name: ReadFile()
    // Desc: 
    //-----------------------------------------------------------------------------
    char* ReadFile(const char* file_name, size_t* bytes_read = 0) {
        char* buffer = NULL;

        // Read input file
        if (file_name != NULL) {
            FILE* fin = fopen(file_name, "rb");
            if (fin != NULL) {
                fseek(fin, 0, SEEK_END);
                long file_size = ftell(fin);
                rewind(fin);

                if (file_size > 0) {
                    buffer = new char[file_size + 1];
                    size_t count = fread(buffer, sizeof(char), file_size, fin);
                    buffer[count] = '\0';
                    if (bytes_read) *bytes_read = count;
                }
                fclose(fin);
            }
        }

        return buffer;
    }


    namespace Mesh {
        //-----------------------------------------------------------------------------
        // Name: CreatePlane()
        // Desc: 
        //-----------------------------------------------------------------------------
        glm::vec3* CreatePlane(float size, int slice_x, int slice_y, int* num_vertices) {
            assert(num_vertices != NULL);

            const int NUM_VERTICES = 4*slice_x*slice_y;
            if (NUM_VERTICES < 4)
                return NULL;

            // Compute position deltas for moving down the X, and Z axis during mesh creation
            const glm::vec3 delta = glm::vec3(size / slice_x, size / slice_y, 0.0f);
            const glm::vec3 start = glm::vec3(-size * 0.5f, -size * 0.5f, 0.0f);   
            glm::vec3* mesh = new glm::vec3[NUM_VERTICES];
            glm::vec3* ptr  = mesh;

            for (int x = 0; x < slice_x; x++) {
                for (int y = 0; y < slice_y; y++) {
                    *ptr++ = start + delta * glm::vec3(    x,     y, 0.0f);
                    *ptr++ = start + delta * glm::vec3(x + 1,     y, 0.0f);
                    *ptr++ = start + delta * glm::vec3(x + 1, y + 1, 0.0f);
                    *ptr++ = start + delta * glm::vec3(    x, y + 1, 0.0f);
                }
            }

            if (num_vertices != NULL)
                *num_vertices = NUM_VERTICES;

            return mesh;
        }

        //-----------------------------------------------------------------------------
        // Name: CreatePlane()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreatePlane(GLsizei vertex_density, GLsizei index_density, GLenum mode, 
                         std::vector<glm::vec2>& vertices, std::vector<GLuint>& indices) {
            if ((vertex_density < 2) || ((index_density < 2) && (index_density != 0)))
                return false;
            if ((mode != GL_TRIANGLES) && (mode != GL_TRIANGLE_STRIP) && (mode != GL_QUADS))
                return false;

        // Generate mesh vertices
            const int total_vertices = vertex_density*vertex_density;
            const GLfloat offset = 2.0f / (vertex_density-1);

            vertices.clear();
            vertices.reserve(total_vertices);
            for (int y = 0; y < vertex_density - 1; y++) {
                for (int x = 0; x < vertex_density - 1; x++)
                    vertices.push_back(glm::vec2(-1.0f + x*offset, -1.0f + y*offset));
                vertices.push_back(glm::vec2(1.0f, -1.0f + y*offset));
            }
            for (int x = 0; x < vertex_density - 1; x++)
                vertices.push_back(glm::vec2(-1.0f + x*offset, 1.0f));
            vertices.push_back(glm::vec2(1.0f, 1.0f));

        // Generate mesh indices
            indices.clear();
            if (index_density == 0) {
                const int total_indices = (mode == GL_TRIANGLES) ? 6*(vertex_density - 1)*(vertex_density - 1) : 2*(vertex_density*vertex_density - 2);
                indices.reserve(total_indices);

                GLuint index  = 0;
                if (mode == GL_TRIANGLES) {
                    for (int y = 0; y < vertex_density - 1; y++) {
                        for (int x = 0; x < vertex_density - 1; x++) {
                            indices.push_back(index);
                            indices.push_back(index + 1);
                            indices.push_back(index + vertex_density);
                            indices.push_back(index + vertex_density);
                            indices.push_back(index + 1);
                            indices.push_back(index + 1 + vertex_density);
                            index++;
                        }
                        index++;
                    }
                }
                else {
                    for (int y = 0; y < vertex_density - 1; y++) {
                        for (int x = 0; x < vertex_density; x++) {
                            indices.push_back(index);
                            indices.push_back(index + vertex_density);
                            index++;
                        }
                        if ((vertex_density > 2) && (y != vertex_density - 2)) {
                            indices.push_back(index + vertex_density - 1);
                            indices.push_back(index);
                        }
                    }
                }
            } else { // (index_density == 0)
                const int total_indices = (mode == GL_TRIANGLES) ? 6*(index_density - 1)*(index_density - 1) : 
                                          ((mode == GL_TRIANGLE_STRIP) ? 2*(index_density*index_density - 2) : 4*(index_density - 1)*(index_density - 1));
                const float h_index_offset = static_cast<float>(vertex_density - 1) / (index_density - 1);
                indices.reserve(total_indices);

                GLuint index  = 0;
                switch (mode) {
                case GL_TRIANGLES: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            for (int x = 0; x < index_density - 1; x++) {
                                const GLuint hindex1 = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                const GLuint hindex2 = glm::clamp(static_cast<int>((x + 1) * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(vindex1 + hindex1); // 0
                                indices.push_back(vindex1 + hindex2); // 1
                                indices.push_back(vindex2 + hindex1); // 2
                                indices.push_back(vindex2 + hindex1); // 3
                                indices.push_back(vindex1 + hindex2); // 4
                                indices.push_back(vindex2 + hindex2); // 5
                            }
                        }
                    }
                    break;
                case GL_TRIANGLE_STRIP: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            const int startIdx = indices.size() + 1;
                            const int endIdx = indices.size() + 2*index_density - 1;
                            for (int x = 0; x < index_density; x++) {
                                const GLuint hindex = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(hindex + vindex1);
                                indices.push_back(hindex + vindex2);
                            }
                            if (y < index_density - 2) {
                                // add 2 degenerated triangles
                                indices.push_back(indices[endIdx]);
                                indices.push_back(indices[startIdx]);
                            }
                        }
                    }
                    break;
                case GL_QUADS: {
                        for (int y = 0; y < index_density - 1; y++) {
                            const GLuint vindex1 = glm::clamp(static_cast<int>(glm::round(y * h_index_offset)*vertex_density), 0, vertex_density*(vertex_density - 1));
                            const GLuint vindex2 = glm::clamp(static_cast<int>(glm::round((y + 1)*h_index_offset) * vertex_density), 0, vertex_density*(vertex_density - 1));
                            for (int x = 0; x < index_density - 1; x++) {
                                const GLuint hindex1 = glm::clamp(static_cast<int>(x * h_index_offset + 0.5f), 0, vertex_density - 1);
                                const GLuint hindex2 = glm::clamp(static_cast<int>((x + 1) * h_index_offset + 0.5f), 0, vertex_density - 1);
                                indices.push_back(hindex1 + vindex1);
                                indices.push_back(hindex1 + vindex2);
                                indices.push_back(hindex2 + vindex2);
                                indices.push_back(hindex2 + vindex1);
                            }
                        }
                    }
                    break;
                default:
                    assert(0);
                    break;
                }
            }
    
            return true;
        }

        //-----------------------------------------------------------------------------
        // Name: CreateCircle()
        // Desc: 
        //-----------------------------------------------------------------------------
        template <class INDEX_TYPE>
        void CreateCircle(GLenum primitive_type, unsigned int num_triangles, float z,
                                  std::vector<glm::vec3>& vertices, std::vector<INDEX_TYPE>* indices) {
            if ((primitive_type != GL_TRIANGLES) && (primitive_type != GL_TRIANGLE_FAN))
                return;

            const GLfloat TWO_PI = 2.0f * glm::pi<float>();
            const GLfloat alpha_step = TWO_PI / num_triangles;
            GLfloat alpha = 0.0f;

            if (indices != NULL) {
                // Generate mesh vertices
                vertices.reserve(num_triangles + 1);
                vertices.push_back(glm::vec3());
                for (unsigned int iTriangle = 0; iTriangle < num_triangles; iTriangle++) {
                    vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                    alpha = glm::min(TWO_PI, alpha + alpha_step);
                }

                // Generate IBO
                if (primitive_type == GL_TRIANGLES) {
                    indices->reserve(num_triangles * 3);
                    for (unsigned int iTriangle = 0; iTriangle < num_triangles - 1; iTriangle++) {
                        indices->push_back(static_cast<INDEX_TYPE>(0));
                        indices->push_back(static_cast<INDEX_TYPE>(iTriangle + 1));
                        indices->push_back(static_cast<INDEX_TYPE>(iTriangle + 2));
                    }
                    indices->push_back(static_cast<INDEX_TYPE>(0));
                    indices->push_back(static_cast<INDEX_TYPE>(num_triangles));
                    indices->push_back(static_cast<INDEX_TYPE>(1));
                } else {
                    indices->reserve(vertices.size() + 1);
                    for (unsigned int iVertex = 0; iVertex < vertices.size(); iVertex++) {
                        indices->push_back(static_cast<INDEX_TYPE>(iVertex));
                    }
                    indices->push_back(static_cast<INDEX_TYPE>(1));
                }
            }
            else {
            // Generate sequence geometry mesh
                vertices.reserve((primitive_type == GL_TRIANGLES) ? (num_triangles * 3) : (num_triangles + 2));
                for (unsigned int iTriangle = 0; iTriangle < num_triangles; iTriangle++) {
                    // 1st vertex of triangle
                    if ((primitive_type == GL_TRIANGLES) || (iTriangle == 0))
                        vertices.push_back(glm::vec3());

                    // 2nd vertex of triangle
                    vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                    // 3rd vertex of triangle
                    alpha = glm::min(TWO_PI, alpha + alpha_step);
                    if (primitive_type == GL_TRIANGLES)
                        vertices.push_back(glm::vec3(cos(alpha), sin(alpha), z));
                }
                if (primitive_type != GL_TRIANGLES)
                    vertices.push_back(glm::vec3(1.0f, 0.0f, z));
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateSphere()
        // Desc: based on OpenGL SuperBible example by Richard S. Wright Jr.
        //-----------------------------------------------------------------------------
        glm::vec3* CreateSphereMesh(float radius, int slices, int stacks, int* num_vertices) {
            assert(num_vertices != NULL);

            const int NUM_VERTICES = stacks*(2 * slices + 2);
            if (NUM_VERTICES < 1)
                return NULL;

            const float DRHO = glm::pi<float>() / stacks;
            const float DTHETA = 2.0f * glm::pi<float>() / slices;
            glm::vec3* mesh = new glm::vec3[NUM_VERTICES];
            glm::vec3* ptr  = mesh;
            for (int iStack = 0; iStack < stacks; iStack++)  {
                const GLfloat RHO = iStack * DRHO;
                for (int iSlice = 0; iSlice <= slices; iSlice++) {
                    const GLfloat THETA  = (iSlice == slices) ? 0.0f : iSlice * DTHETA;
                    const glm::vec3 theta = glm::vec3((GLfloat)(-sin(THETA)), (GLfloat)(cos(THETA)), 1.0f) * radius;
                    *ptr++ = theta * glm::vec3((GLfloat)(sin(RHO + DRHO)), (GLfloat)(sin(RHO + DRHO)), (GLfloat)(cos(RHO + DRHO)));
                    *ptr++ = theta * glm::vec3((GLfloat)(sin(RHO)), (GLfloat)(sin(RHO)), (GLfloat)(cos(RHO)));
                }
            }

            *num_vertices = NUM_VERTICES;
            return mesh;
        }
    } // end of namespace Mesh



    namespace Shader {
        //-----------------------------------------------------------------------------
        // Name: CheckShaderInfoLog()
        // Desc: 
        //-----------------------------------------------------------------------------
        void CheckShaderInfoLog(GLuint shader_id) {
            if (shader_id == 0)
                return;
    
            int log_length = 0;
            glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

            if (log_length > 0) {
                char* buffer  = new char[log_length];
                int   written = 0;
                glGetShaderInfoLog(shader_id, log_length, &written, buffer);
                printf("%s\n", buffer);
                delete [] buffer;
            }
        }

        //-----------------------------------------------------------------------------
        // Name: CheckProgramInfoLog()
        // Desc: 
        //-----------------------------------------------------------------------------
        void CheckProgramInfoLog(GLuint program_id) {
            if (program_id == 0)
                return;
 
            int log_length = 0;
            glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

            if (log_length > 0) {
                char* buffer  = new char[log_length];
                int   written = 0;
                glGetProgramInfoLog(program_id, log_length, &written, buffer);
                printf("%s\n", buffer);
                delete [] buffer;
            }
        }

        //-----------------------------------------------------------------------------
        // Name: CheckShaderCompileStatus()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLint CheckShaderCompileStatus(GLuint shader_id) {
            GLint status = GL_FALSE;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
            return status;
        }


        //-----------------------------------------------------------------------------
        // Name: CheckProgramLinkStatus()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLint CheckProgramLinkStatus(GLuint program_id) {
            GLint status = GL_FALSE;
            glGetProgramiv(program_id, GL_LINK_STATUS, &status);
            return status;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderFromSource()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateShaderFromSource(GLenum shader_type, const char* source, const char* name) {
            if (source == NULL)
                return 0;

            switch (shader_type) {
                case GL_VERTEX_SHADER         : printf("vertex shader %s creation ... ", name); break;
                case GL_FRAGMENT_SHADER       : printf("fragment shader %s creation ... ", name); break;
                case GL_GEOMETRY_SHADER       : printf("geometry shader %s creation ... ", name); break;
                case GL_TESS_CONTROL_SHADER   : printf("tesselation control %s shader creation ... ", name); break;
                case GL_TESS_EVALUATION_SHADER: printf("tesselation evaluation %s shader creation ... ", name); break;
                default                       : return 0;
            }

            GLuint shader_id = glCreateShader(shader_type);
            if (shader_id == 0)
                return 0;

            glShaderSource(shader_id, 1, &source, NULL);
            glCompileShader(shader_id);
            if (CheckShaderCompileStatus(shader_id) != GL_TRUE) {
                printf("failed.\n");
                CheckShaderInfoLog(shader_id);
                glDeleteShader(shader_id);
                return 0;
            } else {
                printf("successfull.\n");
                return shader_id;
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderFromFile()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateShaderFromFile(GLenum shader_type, const char* file_name, const char* preprocessor = NULL) {
            char* buffer = Tools::ReadFile(file_name);
            if (buffer == NULL) {
                printf("Shader creation failed, input file is empty or missing!\n");
                return 0;
            }

            GLuint shader_id = 0;
            if (preprocessor) {
                std::string temp = buffer;
                std::size_t insertIdx = temp.find("\n", temp.find("#version"));
                temp.insert((insertIdx != std::string::npos) ? insertIdx : 0, std::string("\n") + preprocessor + "\n\n");
                shader_id = CreateShaderFromSource(shader_type, temp.c_str(), file_name);
            } else
                shader_id = CreateShaderFromSource(shader_type, buffer, file_name);
 
            delete [] buffer;
            return shader_id;
        }


        void _updateProgramList(GLuint oldProgram, GLuint newProgram) {
            // Remove program from OpenGL and internal list
            for (std::vector<OpenGL::Program>::iterator it = OpenGL::programs.begin(); it != OpenGL::programs.end(); ++it) {
                if (it->id == oldProgram) {
                    OpenGL::programs.erase(it);
                     break;
                }
            }
   
            // Save program ID if it contains user variables
            if ((glGetUniformLocation(newProgram, "u_UserVariableInt") > -1) || 
                (glGetUniformLocation(newProgram, "u_UserVariableFloat") > -1) ||
                (glGetUniformLocation(newProgram, "u_FrameCounter") > -1) ||
                (glGetUniformLocation(newProgram, "u_MVPMatrix") > -1) || 
                (glGetUniformLocation(newProgram, "u_ModelViewMatrix") > -1) || 
                (glGetUniformLocation(newProgram, "ModelViewMatrixInv") > -1) || 
                (glGetUniformLocation(newProgram, "u_ProjectionMatrix") > -1) ||
                (glGetUniformLocation(newProgram, "u_ZOffset") > -1)) {
                OpenGL::programs.push_back(newProgram);
            }
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderProgramFromSource()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreateShaderProgramFromSource(GLuint& programId, GLint count, const GLenum* shader_types, const char** source) {
            if ((shader_types == NULL) || (source == NULL))
                return false;

            // Create shader program object
            GLuint pr_id = glCreateProgram();
            for (int i = 0; i < count; i++) {
                GLuint shader_id = CreateShaderFromSource(shader_types[i], source[i], "");
                if (shader_id == 0) {
                    glDeleteProgram(pr_id);
                    return false;
                }
                glAttachShader(pr_id, shader_id);
                glDeleteShader(shader_id);
            }
            glLinkProgram(pr_id);
            if (!CheckProgramLinkStatus(pr_id)) {
                CheckProgramInfoLog(pr_id);
                printf("program linking failed.\n");
                glDeleteProgram(pr_id);
                return false;
            }

            // Remove program from OpenGL and update internal list
            glDeleteProgram(programId);
            _updateProgramList(programId, pr_id);
            programId = pr_id;
   
            return true;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateShaderProgramFromFile()
        // Desc: 
        //-----------------------------------------------------------------------------
        bool CreateShaderProgramFromFile(GLuint& programId, const char* vs, const char* tc,
                                         const char* te, const char* gs, const char* fs, const char* preprocessor = NULL) {
            GLenum shader_types[5] = {
                vs ? GL_VERTEX_SHADER : GL_NONE,
                tc ? GL_TESS_CONTROL_SHADER : GL_NONE,
                te ? GL_TESS_EVALUATION_SHADER : GL_NONE,
                gs ? GL_GEOMETRY_SHADER : GL_NONE,
                fs ? GL_FRAGMENT_SHADER : GL_NONE,
            };
            const char* source_file_names[5] = {
                vs, tc, te, gs, fs
            };

            // Create shader program object
            GLuint pr_id = glCreateProgram();
            for (int i = 0; i < 5; i++) {
                if (source_file_names[i]) {
                    GLuint shader_id = CreateShaderFromFile(shader_types[i], source_file_names[i], preprocessor);
                    if (shader_id == 0) {
                        glDeleteProgram(pr_id);
                        return false;
                    }
                    glAttachShader(pr_id, shader_id);
                    glDeleteShader(shader_id);
                }
            }
            glLinkProgram(pr_id);
            if (!CheckProgramLinkStatus(pr_id)) {
                CheckProgramInfoLog(pr_id);
                printf("Program linking failed!\n");
                glDeleteProgram(pr_id);
                return false;
            }

            // Remove program from OpenGL and update internal list
            glDeleteProgram(programId);
            _updateProgramList(programId, pr_id);
            programId = pr_id;

            return true;
        }
    } // end of namespace Shader


    namespace Texture {
        //-----------------------------------------------------------------------------
        // Name: Show2DTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        inline void Show2DTexture(GLuint tex_id, GLint x, GLint y, GLsizei width, GLsizei height) {
            static const GLenum SHADER_TYPES[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
            // Vertex shader
            static const char* vertex_shader = 
                "#version 330 core\n\
                layout (location = 0) in vec4 a_Vertex;\n\
                out vec2 v_TexCoord;\n\
                void main(void) {\n\
                  v_TexCoord  = a_Vertex.zw;\n\
                  gl_Position = vec4(a_Vertex.xy, 0.0, 1.0f);\n\
                }";
            // Fragment shader GL_RGBA
            static const char* fragment_shader_rgba8 = 
                "#version 330 core\n\
                layout (location = 0) out vec4 FragColor;\n\
                in vec2 v_TexCoord;\n\
                uniform sampler2D u_Texture;\n\
                void main(void) {\n\
                  FragColor = vec4(texture(u_Texture, v_TexCoord).rgb, 1.0);\n\
                }";
           // Fragment shader GL_R32UI
           static const char* fragment_shader_r32ui =
               "#version 330 core\n\
                layout (location = 0) out vec4 FragColor;\n\
                in vec2 v_TexCoord;\n\
                uniform usampler2D u_Texture;\n\
                void main(void) {\n\
                   uint color = texture(u_Texture, v_TexCoord).r;\n\
                   FragColor = vec4(float(color) * 0.0625, 0.0, 0.0, 1.0);\n\
                }";
            static GLuint s_program_ids[2] = {0};

            glm::vec4 vp;
            glGetFloatv(GL_VIEWPORT, &vp.x);
            const GLfloat normalized_coords_with_tex_coords[] = {
                        (x - vp.x)/(vp.z - vp.x)*2.0f - 1.0f,          (y - vp.y)/(vp.w - vp.y)*2.0f - 1.0f, 0.0f, 0.0f,
                (x + width - vp.x)/(vp.z - vp.x)*2.0f - 1.0f,          (y - vp.y)/(vp.w - vp.y)*2.0f - 1.0f, 1.0f, 0.0f,
                (x + width - vp.x)/(vp.z - vp.x)*2.0f - 1.0f, (y + height - vp.y)/(vp.w - vp.y)*2.0f - 1.0f, 1.0f, 1.0f,
                        (x - vp.x)/(vp.z - vp.x)*2.0f - 1.0f, (y + height - vp.y)/(vp.w - vp.y)*2.0f - 1.0f, 0.0f, 1.0f,
            };

            // Setup texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            GLint tex_format = GL_RGBA8;
            GLint tex_comp_mode = GL_NONE;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &tex_format);
            glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, &tex_comp_mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);   // disable compare mode

            // Compile shaders
            const char* sources[2] = {
                vertex_shader, (tex_format == GL_R32UI) ? fragment_shader_r32ui : fragment_shader_rgba8
            };
            int index = (tex_format == GL_R32UI) ? 1 : 0;
            if (s_program_ids[index] == 0) {
                if (!Shader::CreateShaderProgramFromSource(s_program_ids[index], 2, SHADER_TYPES, sources)) {
                    printf("Show2DTexture: Unable to compile shader program.");
                    return;
                }
            }
            GLint current_program_id = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &current_program_id);
            GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);

            // Render textured screen quad
            glDisable(GL_DEPTH_TEST);
            glUseProgram(s_program_ids[index]);
            glUniform1i(glGetUniformLocation(s_program_ids[index], "u_Texture"), 0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, normalized_coords_with_tex_coords);
            glEnableVertexAttribArray(0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            // glDisableVertexAttribArray(0);
            glUseProgram(current_program_id);
            if (depth_test_enabled)
                glEnable(GL_DEPTH_TEST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, tex_comp_mode);   // set original compare mode
        }


        //-----------------------------------------------------------------------------
        // Name: ShowCubeTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        inline void ShowCubeTexture(GLuint tex_id, GLint x, GLint y, GLsizei width, GLsizei height) {
            static const GLenum SHADER_TYPES[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
            static const char* sources[2] = {
                // Vertex shader
                "#version 330 core\n\
                layout (location = 0) in vec3 a_Vertex;\n\
                layout (location = 1) in vec3 a_TexCoord;\n\
                uniform vec2 u_Offset[7];\n\
                uniform vec4 u_Scale;\n\
                out vec3 v_TexCoord;\n\
                void main(void) {\n\
                    v_TexCoord  = a_TexCoord;\n\
                    gl_Position = vec4((a_Vertex.xy*u_Scale.xy + u_Offset[int(a_Vertex.z)]) * u_Scale.zw - 1.0, 0.0, 1.0f);\n\
                }",
                // Fragment shader
                "#version 330 core\n\
                layout (location = 0) out vec4 FragColor;    \n\
                uniform samplerCube u_Texture;\n\
                uniform vec4 u_Color;\n\
                in vec3 v_TexCoord; \n\
                void main(void) {\n\
                    FragColor = vec4(texture(u_Texture, v_TexCoord).rgb, 1.0) * u_Color;\n\
                }"
            };
            static GLuint s_program_id = 0;
            if (s_program_id == 0) {
                if (!Shader::CreateShaderProgramFromSource(s_program_id, 2, SHADER_TYPES, sources)) {
                    printf("ShowCubeTexture: Unable to compile shader program.");
                    return;
                }
            }
            static GLuint s_vao = 0;
            if (s_vao == 0) {
                const GLfloat vertices[] = {
                    // full rect
                    0.0f, 0.0f, 0.0f, -1.0f,-1.0f,-1.0f,        1.0f, 0.0f, 0.0f, -1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, 0.0f, -1.0f,-1.0f,-1.0f,        1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 1.0f,        0.0f, 1.0f, 0.0f, -1.0f, 1.0f,-1.0f,
                    // positive x
                    0.0f, 0.0f, 1.0f,  1.0f,-1.0f, 1.0f,        1.0f, 0.0f, 1.0f,  1.0f,-1.0f,-1.0f,        1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,
                    0.0f, 0.0f, 1.0f,  1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f,        0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
                    // negative x
                    0.0f, 0.0f, 2.0f, -1.0f,-1.0f,-1.0f,        1.0f, 0.0f, 2.0f, -1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 2.0f, -1.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, 2.0f, -1.0f,-1.0f,-1.0f,        1.0f, 1.0f, 2.0f, -1.0f, 1.0f, 1.0f,        0.0f, 1.0f, 2.0f, -1.0f, 1.0f,-1.0f,
                    // positive y
                    0.0f, 0.0f, 3.0f,  1.0f, 1.0f,-1.0f,        1.0f, 0.0f, 3.0f, -1.0f, 1.0f,-1.0f,        1.0f, 1.0f, 3.0f, -1.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, 3.0f,  1.0f, 1.0f,-1.0f,        1.0f, 1.0f, 3.0f, -1.0f, 1.0f, 1.0f,        0.0f, 1.0f, 3.0f,  1.0f, 1.0f, 1.0f,
                    // negative y
                    0.0f, 0.0f, 4.0f,  1.0f,-1.0f, 1.0f,        1.0f, 0.0f, 4.0f, -1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 4.0f, -1.0f,-1.0f,-1.0f,
                    0.0f, 0.0f, 4.0f,  1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 4.0f, -1.0f,-1.0f,-1.0f,        0.0f, 1.0f, 4.0f,  1.0f,-1.0f,-1.0f,
                    // positive z
                    0.0f, 0.0f, 5.0f, -1.0f,-1.0f, 1.0f,        1.0f, 0.0f, 5.0f,  1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 5.0f,  1.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, 5.0f, -1.0f,-1.0f, 1.0f,        1.0f, 1.0f, 5.0f,  1.0f, 1.0f, 1.0f,        0.0f, 1.0f, 5.0f, -1.0f, 1.0f, 1.0f,
                    // negative z
                    0.0f, 0.0f, 6.0f,  1.0f,-1.0f,-1.0f,        1.0f, 0.0f, 6.0f, -1.0f,-1.0f,-1.0f,        1.0f, 1.0f, 6.0f, -1.0f, 1.0f,-1.0f,
                    0.0f, 0.0f, 6.0f,  1.0f,-1.0f,-1.0f,        1.0f, 1.0f, 6.0f, -1.0f, 1.0f,-1.0f,        0.0f, 1.0f, 6.0f,  1.0f, 1.0f,-1.0f,
                };
                GLuint vbo = 0;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glGenVertexArrays(1, &s_vao);
                glBindVertexArray(s_vao);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (const void*) 0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (const void*) (3*sizeof(GLfloat)));
                glBindVertexArray(0);
                glDeleteBuffers(1, &vbo);
            }

            glm::vec4 vp;
            glGetFloatv(GL_VIEWPORT, &vp.x);
            const GLfloat offset[] = {
                (GLfloat)x, (GLfloat)y, 
                (GLfloat)x, y+height/3.0f,          // positive_x
                x+width/2.0f, y+height/3.0f,        // negative_x
                x+width/4.0f, y+height*2.0f/3.0f,   // positive_y
                x+width/4.0f, (GLfloat)y,           // negative_y
                x+width*3.0f/4.0f, y+height/3.0f,   // positive_z
                x+width/4.0f, y+height/3.0f,        // negative_z
            };

            GLint current_program_id = 0;
            glGetIntegerv(GL_CURRENT_PROGRAM, &current_program_id);
            GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
            GLboolean blending_enabled = glIsEnabled(GL_BLEND);

            // Render textured screen quad
            glDisable(GL_DEPTH_TEST);
            glUseProgram(s_program_id);
            glBindVertexArray(s_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
            glUniform1i(glGetUniformLocation(s_program_id, "u_Texture"), 0);
            glUniform2fv(glGetUniformLocation(s_program_id, "u_Offset"), 7, offset);
            glUniform4f(glGetUniformLocation(s_program_id, "u_Color"), 0.0f, 0.0f, 0.0f, 0.5f);
            glUniform4f(glGetUniformLocation(s_program_id, "u_Scale"), (GLfloat)width, (GLfloat)height, 2.0f/(vp.z - vp.x), 2.0f/(vp.w - vp.y));
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisable(GL_BLEND);
            glUniform4f(glGetUniformLocation(s_program_id, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform4f(glGetUniformLocation(s_program_id, "u_Scale"), width/4.0f, height/3.0f, 2.0f/(vp.z - vp.x), 2.0f/(vp.w - vp.y));
            glDrawArrays(GL_TRIANGLES, 6, 36);
            glBindVertexArray(0);

            glUseProgram(current_program_id);
            if (depth_test_enabled)
                glEnable(GL_DEPTH_TEST);
            if (blending_enabled)
                glEnable(GL_BLEND);
        }


        //-----------------------------------------------------------------------------
        // Name: LoadRGBA8()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint LoadR8(const char* filename, GLsizei* num_texels = NULL) {
            // Create mipmapped texture with image
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
            if (rgb_data == NULL)
                return 0;

            const GLsizei width = static_cast<GLsizei>(sqrtf((float)bytes_read));
            assert(width*width == bytes_read);

            GLuint texId = 0;
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, width, GL_RED, GL_UNSIGNED_BYTE, rgb_data);

            delete [] rgb_data;

            if (num_texels)
                *num_texels = width*width;

            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: LoadRGB8()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint LoadRGB8(const char* filename, GLsizei* num_texels = NULL) {
            // Create mipmapped texture from raw file
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
            if (rgb_data == NULL)
                return 0;

            const GLsizei width = static_cast<GLsizei>(sqrtf(bytes_read / 3.0f));
            assert(width*width*3 == bytes_read);

            GLuint texId = 0;
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, width, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);

            delete [] rgb_data;

            if (num_texels)
                *num_texels = width*width;

            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: LoadRGBA8()
        // Desc: resolution = (width, height, mipmap levels)
        //-----------------------------------------------------------------------------
        inline GLuint LoadRGB8(const char* filename, glm::ivec3 resolution, bool immutable = false) {
            // Create mipmapped texture from raw file
            size_t bytes_read = 0;
            const char* rgb_data = Tools::ReadFile(filename, &bytes_read);
 
            if (rgb_data == NULL)
                return 0;

            GLuint texId = 0;
            glGenTextures(1, &texId);
            glBindTexture(GL_TEXTURE_2D, texId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if (immutable)
                glTexStorage2D(GL_TEXTURE_2D, resolution.z, GL_RGBA8, resolution.x, resolution.y); 
            
            if (0 && !immutable && (resolution.z == 1) && ((resolution.x > 1) || (resolution.y > 1)))
                gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, resolution.x, resolution.y, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);
            else {
                GLint level = 0;
                const char* ptr = rgb_data; 
                while ((bytes_read > 0) && (level < resolution.z)) {
                    if (immutable)
                        glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, resolution.x, resolution.y, GL_RGB, GL_UNSIGNED_BYTE, ptr);
                    else
                        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr);

                    const size_t bytes_to_move = 3 * resolution.x * resolution.y * sizeof(GLubyte);
                    ptr += bytes_to_move;
                    bytes_read -= bytes_to_move;

                    if (resolution.x > 1)
                        resolution.x >>= 1;
                    if (resolution.y > 1)
                        resolution.y >>= 1;
                    level++;
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - 1);
            }

            delete [] rgb_data;
			assert(glGetError() == GL_NO_ERROR);
            return texId;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateColoredTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateColoredTexture(GLuint texture_id) {
            if (texture_id == 0) 
                return 0;

            // Clone texture with colorized mipmap layers
            GLint width = 0, height = 0;
            GLint level = 0;
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); 
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            if (width*height == 0)
                return 0;

            GLuint colored_texture_id = 0;
            glGenTextures(1, &colored_texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            GLubyte* texels = new GLubyte[width * height * 4];
            const GLubyte color_shift[8][3] = {{0, 6, 6}, {0, 0, 6}, {6, 0, 6}, {6, 0, 0}, {6, 6, 0}, {0, 6, 0}, {3, 3, 3}, {6, 6, 6}};
            const int MAX_COLOR_LEVELS = sizeof(color_shift) / sizeof(color_shift[0]);

            do {
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, texels);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width); 
                glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &height);

                for (int i = 0; i <width*height; i++) {
                    const int shift_level = glm::min(level, MAX_COLOR_LEVELS);
                    texels[4*i+0] >>= color_shift[shift_level][0];
                    texels[4*i+1] >>= color_shift[shift_level][1];
                    texels[4*i+2] >>= color_shift[shift_level][2];
                }
                glBindTexture(GL_TEXTURE_2D, colored_texture_id);
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);
                level++;
                assert(glGetError() == GL_NO_ERROR);
            } while ((width > 1) || (height > 1));
            glBindTexture(GL_TEXTURE_2D, 0);

            delete [] texels;
            return colored_texture_id;
        }


        //-----------------------------------------------------------------------------
        // Name: CreateSimpleTexture()
        // Desc: 
        //-----------------------------------------------------------------------------
        GLuint CreateSimpleTexture(GLint width, GLint height)
        {
            if (width * height <= 0) {
                return 0;
            }

            GLubyte* data = new GLubyte[width*height*4];
            assert(data != NULL);

            const GLubyte pattern_width = 16;
            GLubyte* ptr = data; 
            GLubyte pattern = 0xFF;

            for (int h = 0; h < height; h++) {              
                for (int w = 0; w < width; w++) {
                    *ptr++ = pattern; 
                    *ptr++ = pattern;
                    *ptr++ = pattern;
                    *ptr++ = 255;
                    if (w % pattern_width == 0)
                        pattern = ~pattern;
                }

                if (h % pattern_width == 0)
                    pattern = ~pattern;
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

            GLuint tex_id = 0;
            glGenTextures(1, &tex_id);
            glBindTexture(GL_TEXTURE_2D, tex_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            delete [] data;

            return tex_id;
        } 
    }; // end of namespace Texture

    namespace Noise {
        /* Taken from "Lode's Computer Graphics Tutorial", http://lodev.org/cgtutor/randomnoise.html */

        const int NoiseWidth  = 128;
        const int NoiseHeight = 128;
        float Noise[NoiseHeight][NoiseWidth] = {0}; // The noise array


        //-----------------------------------------------------------------------------
        // Name: GenerateNoise()
        // Desc: Generates array of random values
        //-----------------------------------------------------------------------------
          void GenerateNoise() {
            if (Noise[0][0] != 0.0f)
                return;

            for (int y = 0; y < NoiseHeight; y++) {
                for (int x = 0; x < NoiseWidth; x++) {
                    Noise[y][x] = glm::compRand1<float>();
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Name: SmoothNoise()
        // Desc: 
        //-----------------------------------------------------------------------------
        float SmoothNoise(float x, float y) {
           // Get fractional part of x and y
           const float fractX = x - int(x);
           const float fractY = y - int(y);

           // Wrap around
           const int x1 = (int(x) + NoiseWidth) % NoiseWidth;
           const int y1 = (int(y) + NoiseHeight) % NoiseHeight;

           // Neighbor values
           const int x2 = (x1 + NoiseWidth - 1) % NoiseWidth;
           const int y2 = (y1 + NoiseHeight - 1) % NoiseHeight;

           // Smooth the noise with bilinear interpolation
           return fractY * (fractX * Noise[y1][x1] + (1 - fractX) * Noise[y1][x2]) +
                  (1 - fractY) * (fractX * Noise[y2][x1] + (1 - fractX) * Noise[y2][x2]);
        }


        //-----------------------------------------------------------------------------
        // Name: Turbulence()
        // Desc: 
        //-----------------------------------------------------------------------------
        float Turbulence(int x, int y, int size) {
            const int initialSize = size;
            float value = 0.0;

            while(size >= 1) {
                const float sizeInv = 1.0f / size;
                value += SmoothNoise(x * sizeInv, y * sizeInv) * size;
                size >>= 1;
            }

            return(128.0f * value / initialSize);
        }


        //-----------------------------------------------------------------------------
        // Name: GenerateMarblePatter()
        // Desc: 
        //-----------------------------------------------------------------------------
        glm::u8vec3* GenerateMarblePatter(int width, int height, int turbSize = 32, float turbPower = 5.0f,
                                          float xPeriod = 5.0f, float yPeriod = 10.0f) {
            GenerateNoise();

            // xPeriod and yPeriod together define the angle of the lines
            // xPeriod and yPeriod both 0 ==> it becomes a normal clouds or turbulence pattern
            xPeriod /= NoiseWidth;  // defines repetition of marble lines in x direction
            yPeriod /= NoiseHeight; // defines repetition of marble lines in y direction

            glm::u8vec3* pixels = new glm::u8vec3[width * height];

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    const float xyValue = x * xPeriod + y * yPeriod + turbPower * Turbulence(x, y, turbSize) * 0.01227184f;
                    const float sineValue = 256 * fabs(sin(xyValue));
                    pixels[x + y*width] = glm::u8vec3(sineValue);
                }
                printf("\rGenerateMarblePatter() - row %d generated\t\t", y);
            }

            return pixels;
        }


    }; // end of namespace Noise
}; // end of namespace Tools

#endif // !__TOOLS_H__