#pragma once

#ifndef __MESH_H__
#define __MESH_H__

#include "../../common/assimp/Importer.hpp"
#include "../../common/assimp/postprocess.h"
#include "../../common/assimp/scene.h"
#include "../../common/glm/glm.hpp"
#include "../../common/GLEW/glew.h"
#include "../../common/GLFW/glfw3.h"
#include <vector>
#include <fstream>
#include "texture.h"


#define gvec33(aivec3) glm::vec3(aivec3->x, aivec3->y, aivec3->z)
#define gvec32(aivec3) glm::vec2(aivec3->x, aivec3->y)

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool LoadMesh(const std::string& filename);
	void AddTransformation(glm::mat4x4 transform);
	void Render(GLuint program);

private:
	bool InitFromScene(const aiScene* pScene, const std::string& filename);
	void InitMesh(const aiMesh* pMesh, 
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec2>& texcoords,
		std::vector<unsigned int>& indices
		);
	bool InitMaterials(const aiScene* pScene, const std::string& filename);
	void Clear();

#define INVALID_MATERIAL	0xFFFFFFFF

	enum MESH_BUFFER_TYPE
	{
		MESH_BUFFER_TYPE_INDEX = 0,
		MESH_BUFFER_TYPE_POSITION,
		MESH_BUFFER_TYPE_NORMAL,
		MESH_BUFFER_TYPE_UV,
		MESH_NUM_BUFFERS
	};

	struct MeshEntry
	{
		unsigned int BaseVertex	= 0;
		unsigned int BaseIndex	= 0;
		unsigned int NumIndices = 0;
		unsigned int MatIndex	= INVALID_MATERIAL;
	};

	GLuint m_VAO;
	GLuint m_Buffers[MESH_NUM_BUFFERS];

	std::vector<MeshEntry> m_Entries;
	std::vector<Texture> m_Textures;
	std::vector<glm::mat4x4> m_ModelMatrices;
	std::vector<glm::mat4x4> m_NormalMatrices;
};

#endif // !__MESH_H__
