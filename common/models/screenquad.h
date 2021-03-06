//-----------------------------------------------------------------------------
//  [PGR2] Screen quad model
//  27/02/2008
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#ifndef __SCREENQUAD_MODEL_H__
#define __SCREENQUAD_MODEL_H__

namespace Tools {
    namespace Mesh {

        const GLfloat QUAD_VERTEX_ARRAY[] = {
			-1.0f,-1.0f, 
			1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f
        };

        const int NUM_QUAD_VERTICES = sizeof(QUAD_VERTEX_ARRAY) / (2*sizeof(GLfloat));
    } // end of namespace Mesh

    //-----------------------------------------------------------------------------
    // Name: DrawScreenQuad()
    // Desc: 
    //-----------------------------------------------------------------------------
    inline void DrawScreenQuad() {
        static GLuint s_quadvao = 0;
        if (s_quadvao == 0) {
            // Lazy initialization of VBO with QUAD polygon mesh
            glGenVertexArrays(1, &s_quadvao);
            glBindVertexArray(s_quadvao);
                GLuint vbo = 0;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::QUAD_VERTEX_ARRAY), Mesh::QUAD_VERTEX_ARRAY, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*) 0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
   //         glDeleteBuffers(1, &vbo);
			//glDisableVertexAttribArray(0);

		}

        glBindVertexArray(s_quadvao);
        glDrawArrays(GL_QUADS, 0, Mesh::NUM_QUAD_VERTICES);
        glBindVertexArray(0);
    }
} // end of namespace Tools

#endif // __SCREENQUAD_MODEL_H__