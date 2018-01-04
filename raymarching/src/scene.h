#pragma once
#include "mesh.h"
#include <iostream>
#include "../../common/glm/gtc/matrix_transform.hpp"

#define NUM_FILES 7

Mesh Objects[NUM_FILES];

std::string files[NUM_FILES] =
{
	DIR + "3.obj",
	DIR + "columns2F.obj",
	DIR + "h3.obj",
	DIR + "wall.obj",
	DIR + "wall2.obj",
	DIR + "wall3.obj",
	DIR + "7.obj"
};

void LoadScene()
{
	std::ifstream tin;
	tin.open("scene.transform");
	std::string empty;
	for (unsigned int i = 0; i < NUM_FILES; ++i)
	{
		Objects[i].LoadMesh(files[i]);
		int count;
		tin >> count;
		for (unsigned int j = 0; j < count; ++j)
		{
			glm::mat4x4 t(1.0);
			glm::mat4x4 s(1.0);
			glm::mat4x4 r(1.0);
			float angle;
			glm::vec3 rot;
			glm::vec3 scale;
			glm::vec3 trans;
			tin >> rot.x >> rot.y >> rot.z >> angle;
			tin >> scale.x >> scale.y >> scale.z;
			tin >> trans.x >> trans.y >> trans.z;
			r = glm::rotate(r, angle, rot);
			s = glm::scale(s, scale);
			t = glm::translate(t, trans);
			Objects[i].AddTransformation(t * r * s);
		}
	}
	tin.close();
}

void RenderSceneGeometry(GLuint program)
{
	for (unsigned int i = 0; i < NUM_FILES; ++i)
	{
		Objects[i].Render(program);
	}
}
