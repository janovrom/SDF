#pragma once

#ifndef __SDF_H__
#define __SDF_H__

#include <vector>
#include "../../common/glm/glm.hpp"
#include "../../common/GLEW/glew.h"
#include "glerror.h"


struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	float sizex() { return max.x - min.x; }
	float sizey() { return max.y - min.y; }
	float sizez() { return max.z - min.z; }
};

class SDF
{
public:
	void ComputeSDF(GLuint posBuffer, GLuint uvBuffer, GLuint idxBuffer, GLuint tex, glm::vec3 origin, glm::vec3 size, glm::ivec2 texSize);
	void InitShader();
	void LaunchComputeShader(GLuint tex);

private:
	GLuint m_program	= 0;
	GLuint m_Textures[3];
	GLuint m_buffer3d;

};

#endif // ! __SDF_H__
