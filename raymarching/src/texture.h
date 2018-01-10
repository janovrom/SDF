#pragma once


#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <string>
#include "../../common/glm/glm.hpp"
#include "../../common/GLEW/glew.h"
#include "../../common/GLFW/glfw3.h"
#include "../../common/assimp/scene.h"
#include "tga.h"
#include "glerror.h"

const std::string DIR = "../obj/";

class Texture
{
public:
	Texture();
	~Texture();

	bool InitTexture(const aiMaterial* pMaterial);
	void BindForGeometryPass(GLuint program);
	void BindForLightPass(GLuint program);
	bool HasAlpha = false;
	glm::ivec2 size;

private:
	static GLuint m_DefaultHeight;

	enum TextureType
	{
		TEXTURE_TYPE_DIFFUSE,
		TEXTURE_TYPE_HEIGHT,
		TEXTURE_TYPE_SPECULAR,
		NUM_TEXTURE_TYPES
	};

	GLuint m_Textures[NUM_TEXTURE_TYPES];

	GLuint LoadTGATexture(const aiMaterial* pMaterial, aiTextureType aiType, Texture::TextureType type);
};

#endif // !__TEXTURE_H__