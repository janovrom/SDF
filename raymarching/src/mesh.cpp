#include "mesh.h"
#include "glerror.h"


char* ReadFile(const char* file_name, size_t* bytes_read = 0)
{
	char* buffer = NULL;

	// Read input file
	if (file_name != NULL)
	{
		FILE* fin = fopen(file_name, "rb");
		if (fin != NULL)
		{
			fseek(fin, 0, SEEK_END);
			long file_size = ftell(fin);
			rewind(fin);

			if (file_size > 0)
			{
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

GLuint LoadRGB8(const char* filename, GLsizei* num_texels = NULL)
{
	// Create mipmapped texture from raw file
	size_t bytes_read = 0;
	//std::ifstream input(filename, std::ios::binary);
	//// copies all data into buffer
	//std::vector<char> buffer((
	//	std::istreambuf_iterator<char>(input)),
	//	(std::istreambuf_iterator<char>()));
	//const char* rgb_data = buffer.data();//ReadFile(filename, &bytes_read);
	const char* rgb_data = ReadFile(filename, &bytes_read);
	if (rgb_data == NULL)
		return 0;

	const GLsizei width = static_cast<GLsizei>(sqrtf(bytes_read / 3.0f));
	assert(width*width * 3 == bytes_read);

	GLuint texId = 0;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, width, GL_RGB, GL_UNSIGNED_BYTE, rgb_data);

	delete[] rgb_data;

	if (num_texels)
		*num_texels = width*width;

	return texId;
}

bool Mesh::LoadMesh(const std::string& Filename)
{
	// Clear existing data
	Clear();

	// Create VAO and buffers
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(MESH_NUM_BUFFERS, m_Buffers);

	// Create importer
	Assimp::Importer Importer;
	bool ret = false;
	const aiScene* pScene = Importer.ReadFile(Filename,
		(unsigned int) (aiProcess_GenSmoothNormals
		| aiProcess_JoinIdenticalVertices
		| aiProcess_Triangulate)

		// TODO Subject to test
		// additional optimization
		//| aiProcess_ImproveCacheLocality
		//| aiProcess_OptimizeMeshes
		//| aiProcess_OptimizeGraph
		//| aiProcess_PreTransformVertices
		//| aiProcess_RemoveRedundantMaterials
	);

	if (pScene)
	{
		ret = InitFromScene(pScene, Filename);
	}
	else
	{
		printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
	}

	// Unload VAO
	glBindVertexArray(0);
	return ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& filename)
{
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);

	unsigned int numVertices	= 0;
	unsigned int numIndices		= 0;
	const aiMesh* pMesh;
	// Count the number of vertices and indices
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		pMesh = pScene->mMeshes[i];
		m_Entries[i].MatIndex	= pMesh->mMaterialIndex;
		m_Entries[i].NumIndices	= pMesh->mNumFaces * 3; // Triangulated!
		m_Entries[i].BaseVertex = numVertices;
		m_Entries[i].BaseIndex	= numIndices;
	
		numVertices += pMesh->mNumVertices;
		numIndices	+= m_Entries[i].NumIndices;
	}
	
	// Prepare buffers
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<unsigned int> indices;

	positions.reserve(numVertices);
	normals.reserve(numVertices);
	texcoords.reserve(numVertices);
	indices.reserve(numIndices);


	// Now init meshes
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		pMesh = pScene->mMeshes[i];
		InitMesh(pMesh, positions, normals, texcoords, indices);
	}

	if (!InitMaterials(pScene, filename))
	{
		return false;
	}

	// Generate and populate the buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[MESH_BUFFER_TYPE_POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[MESH_BUFFER_TYPE_UV]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords[0]) * texcoords.size(), &texcoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[MESH_BUFFER_TYPE_NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[MESH_BUFFER_TYPE_INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
	
	printf("Loaded mesh %s with %d meshes and %d materials.\n", filename.c_str(), pScene->mNumMeshes, pScene->mNumMaterials);
	return true;
}

void Mesh::InitMesh(const aiMesh* pMesh,
	std::vector<glm::vec3>& positions,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec2>& texcoords,
	std::vector<unsigned int>& indices)
{
	const aiVector3D zero(0.0f, 0.0f, 0.0f);

	// Init geometry information
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		const aiVector3D* pPos = &(pMesh->mVertices[i]);
		const aiVector3D* pNorm = &(pMesh->mNormals[i]);
		// mesh can have up to 8 texture coordinates sets
		const aiVector3D* pTex = pMesh->HasTextureCoords(0) ? 
			&(pMesh->mTextureCoords[0][i]) : &zero;

		positions.push_back(gvec33(pPos));
		normals.push_back(gvec33(pNorm));
		texcoords.push_back(gvec32(pTex));
	}

	// Init incidence information
	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
	{
		const aiFace& face = pMesh->mFaces[i];
		assert(face.mNumIndices == 3); // we called for triangulation
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& filename)
{
	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		
		m_Textures[i] = 0;
		// Just use the texture as is
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string fullPath = DIR + path.data;
				m_Textures[i] = LoadRGB8(fullPath.c_str());
			}
		}
	}

	return true;
}

void Mesh::AddTransformation(glm::mat4x4 transform)
{
	m_ModelMatrices.push_back(transform);
	m_NormalMatrices.push_back(glm::transpose(glm::inverse(transform)));
}

void Mesh::Render(GLuint program)
{
	glBindVertexArray(m_VAO);
	printOpenGLError();
	for (unsigned int t = 0; t < m_ModelMatrices.size(); ++t)
	{
		for (unsigned int i = 0; i < m_Entries.size(); ++i)
		{
			const unsigned int MatIdx = m_Entries[i].MatIndex;

			// Did I get myself enough textures?
			assert(MatIdx < m_Textures.size());

			if (m_Textures[MatIdx])
			{
				glActiveTexture(GL_TEXTURE0);
				printOpenGLError();
				glUniform1i(glGetUniformLocation(program, "u_Tex"), 0);
				printOpenGLError();
				glBindTexture(GL_TEXTURE_2D, m_Textures[MatIdx]);
				printOpenGLError();

			}
			glUniformMatrix4fv(glGetUniformLocation(program, "u_Model"), 1, GL_FALSE, &m_ModelMatrices[t][0][0]);
			glUniformMatrix4fv(glGetUniformLocation(program, "u_Normal"), 1, GL_FALSE, &m_NormalMatrices[t][0][0]);
			glDrawElementsBaseVertex(GL_TRIANGLES,
				m_Entries[i].NumIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex), // skip some indices for another mesh
				m_Entries[i].BaseVertex);
			printOpenGLError();


			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	// Unbind
	glBindVertexArray(0);
}

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
	Clear();
}

void Mesh::Clear()
{
	glDeleteBuffers(MESH_NUM_BUFFERS, m_Buffers);
	glDeleteVertexArrays(1, &m_VAO);
	m_VAO = 0;
}