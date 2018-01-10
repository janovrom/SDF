#pragma once

#ifndef __SCENE_H__
#define __SCENE_H__

#include "mesh.h"
#include <iostream>
#include "../../common/glm/gtc/matrix_transform.hpp"
#include "lights.h"

#define NUM_FILES 7


Mesh Objects[NUM_FILES];
GLuint PLights[NUM_POINT_LIGHTS];
GLuint DLights[NUM_DIRECTIONAL_LIGHTS];
GLuint m_SphereVAO;
float PLightsRadii[NUM_POINT_LIGHTS];


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
	// Load meshes
	std::ifstream tin;
	tin.open("scene.transform");
	for (unsigned int i = 0; i < NUM_FILES; ++i)
	{
		Objects[i].LoadMesh(files[i]);
		unsigned int count;
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

	// Load lights
	tin.open("scene.lights"); // First are all point lights, then directional lights
	// Generate buffers for lights
	glGenBuffers(NUM_POINT_LIGHTS, &PLights[0]);
	glGenBuffers(NUM_DIRECTIONAL_LIGHTS, &DLights[0]);
	// Load point lights
	for (unsigned int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		int type;
		tin >> type;
		PointLight p;
		tin >> p.light.color.x >> p.light.color.y >> p.light.color.z;
		tin >> p.light.ambientIntensity >> p.light.diffuseIntensity;
		tin >> p.attenuation.constant >> p.attenuation.linear >> p.attenuation.exp;
		tin >> p.pos.x >> p.pos.y >> p.pos.z;
		p.radius = CalcPLightBSphere(p);
		glBindBuffer(GL_UNIFORM_BUFFER, PLights[i]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PointLight), (void*)&p, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	// Load directional lights
	for (unsigned int i = 0; i < NUM_DIRECTIONAL_LIGHTS; ++i)
	{
		int type;
		tin >> type;
		DirectionalLight p;
		tin >> p.light.color.x >> p.light.color.y >> p.light.color.z;
		tin >> p.light.ambientIntensity >> p.light.diffuseIntensity;
		tin >> p.dir.x >> p.dir.y >> p.dir.z;
		glBindBuffer(GL_UNIFORM_BUFFER, DLights[i]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), (void*)&p, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

#endif // !__SCENE_H__