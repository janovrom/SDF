#pragma once

#ifndef __SCENE_H__
#define __SCENE_H__

#include "mesh.h"
#include "sdf.h"
#include <iostream>
#include "../../common/glm/gtc/matrix_transform.hpp"
//#include "lights.h"
#include "shadowmap.h"

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

#define NUM_FILES 7


Mesh Objects[NUM_FILES];
GLuint PLights[NUM_POINT_LIGHTS];
GLuint DLights[NUM_DIRECTIONAL_LIGHTS];
ShadowMap DirShadowMaps[NUM_DIRECTIONAL_LIGHTS];
ShadowCube PointShadowMaps[NUM_POINT_LIGHTS];
GLuint m_SphereVAO;
GLuint m_Noise;
float PLightsRadii[NUM_POINT_LIGHTS];
ComputeShader m_NoiseShader;


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
		PointShadowMaps[i].Init(SHADOW_CUBE_MAP_SIZE, SHADOW_CUBE_MAP_SIZE, p);
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
		tin >> p.color.x >> p.color.y >> p.color.z;
		tin >> p.ambientIntensity >> p.diffuseIntensity;
		tin >> p.dir.x >> p.dir.y >> p.dir.z;
		p.dir = glm::normalize(p.dir);
		DirShadowMaps[i].Init(WINDOW_WIDTH, WINDOW_HEIGHT, p);
		glBindBuffer(GL_UNIFORM_BUFFER, DLights[i]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), (void*)&p, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	tin.close();

	// Generate noise texture
	glGenTextures(1, &m_Noise);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Noise);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1024, 1024, 0, GL_RED, GL_FLOAT, NULL);
	// Generate noise
	printOpenGLError();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSceneGeometry(GLuint program)
{
	for (unsigned int i = 0; i < NUM_FILES; ++i)
	{
		Objects[i].Render(program); 
	}
}

#endif // !__SCENE_H__