#include "texture.h"
#include "../../common/assimp/scene.h"

GLuint Texture::m_DefaultHeight = 0;

GLuint Texture::LoadTGATexture(const aiMaterial* pMaterial, aiTextureType aiType, Texture::TextureType type)
{
	if (pMaterial->GetTextureCount(aiType) > 0)
	{
		aiString path;

		if (pMaterial->GetTexture(aiType, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			std::string fullPath = DIR + path.C_Str();
			Tga info = Tga(fullPath.c_str());
			glGenTextures(1, &m_Textures[type]);
			glBindTexture(GL_TEXTURE_2D, m_Textures[type]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			HasAlpha = info.HasAlphaChannel();
			size.x = info.GetWidth();
			size.y = info.GetHeight();
			gluBuild2DMipmaps(GL_TEXTURE_2D, info.HasAlphaChannel() ? GL_RGBA : GL_RGB, info.GetWidth(), info.GetHeight(), info.HasAlphaChannel() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, info.GetPixels().data());
			printf("Texture %s\n", fullPath.c_str());
		}

	}
	return m_Textures[type];
}

bool Texture::InitTexture(const aiMaterial* pMaterial)
{
	if (m_DefaultHeight == 0)
	{
		glGenBuffers(1, &m_DefaultHeight);
		glBindTexture(GL_TEXTURE_2D, m_DefaultHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		char blackBytes[] = { 0, 0, 0 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &blackBytes);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Set default values for textures
	for (unsigned int i = 0; i < NUM_TEXTURE_TYPES; ++i)
	{
		m_Textures[i] = 0;
	}

	LoadTGATexture(pMaterial, aiTextureType_DIFFUSE, TextureType::TEXTURE_TYPE_DIFFUSE);
	// Copy diffuse as default texture for specular
	m_Textures[TEXTURE_TYPE_SPECULAR] = m_Textures[TEXTURE_TYPE_DIFFUSE];
	LoadTGATexture(pMaterial, aiTextureType_SPECULAR, TextureType::TEXTURE_TYPE_SPECULAR);
	// Copy black texture as default texture
	if (LoadTGATexture(pMaterial, aiTextureType_HEIGHT, TextureType::TEXTURE_TYPE_HEIGHT) < 1)
	{
		m_Textures[TEXTURE_TYPE_HEIGHT] = m_DefaultHeight;
	}

	return true;
}

void Texture::BindForGeometryPass(GLuint program)
{
	// Bind color map for rasterization fs
	if (m_Textures[TEXTURE_TYPE_DIFFUSE] > 0)
	{
		glActiveTexture(GL_TEXTURE0 + TEXTURE_TYPE_DIFFUSE);
		printOpenGLError();
		glBindTexture(GL_TEXTURE_2D, m_Textures[TEXTURE_TYPE_DIFFUSE]);
		printOpenGLError();
		glUniform1i(glGetUniformLocation(program, "u_DiffuseTex"), TEXTURE_TYPE_DIFFUSE);
		printOpenGLError();
	}
	// Bind height map for rasterization fs
	if (m_Textures[TEXTURE_TYPE_HEIGHT] > 0)
	{
		glActiveTexture(GL_TEXTURE0 + TEXTURE_TYPE_HEIGHT);
		glBindTexture(GL_TEXTURE_2D, m_Textures[TEXTURE_TYPE_HEIGHT]);
		glUniform1i(glGetUniformLocation(program, "u_HeightTex"), TEXTURE_TYPE_HEIGHT);
		printOpenGLError();
	}
	// Bind color map for rasterization fs
	if (m_Textures[TEXTURE_TYPE_SPECULAR] > 0)
	{
		glActiveTexture(GL_TEXTURE0 + TEXTURE_TYPE_SPECULAR);
		glBindTexture(GL_TEXTURE_2D, m_Textures[TEXTURE_TYPE_SPECULAR]);
		glUniform1i(glGetUniformLocation(program, "u_SpecularTex"), TEXTURE_TYPE_SPECULAR);
		printOpenGLError();
	}
}

void Texture::BindForLightPass(GLuint program)
{
	
}

Texture::Texture()
{
}

Texture::~Texture()
{
	glDeleteBuffers(NUM_TEXTURE_TYPES, m_Textures);
}