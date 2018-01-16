#include "sdf.h"

void ComputeShader::ComputeSDF(GLuint posBuffer, GLuint uvBuffer, GLuint idxBuffer, GLuint tex, glm::vec3 origin, glm::vec3 size, glm::ivec2 texSize)
{
	//glGenTextures(3, &m_Textures[0]);
	//glGenTextures(1, &m_buffer3d);
	//glBindTexture(GL_TEXTURE_3D, m_buffer3d);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 64, 64, 64, 0, GL_RED, GL_FLOAT, NULL);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//glm::ivec2 grid[64][64][64]; // for now let's have regular grid
	//size = size / 64.0f;

	//// Bind positions
	//glBindImageTexture(0, posBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_BUFFER, m_Textures[0]);
	////glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, posBuffer);
	//printOpenGLError();
	//// Bind uvs
	//glBindImageTexture(1, uvBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	////glActiveTexture(GL_TEXTURE1);
	////glBindTexture(GL_TEXTURE_BUFFER, m_Textures[1]);
	////glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, uvBuffer);
	//printOpenGLError();
	//// Bind indices
	//glBindImageTexture(2, idxBuffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
	////glActiveTexture(GL_TEXTURE2);
	////glBindTexture(GL_TEXTURE_BUFFER, m_Textures[2]);
	////glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, idxBuffer);
	//printOpenGLError();
	////// Bind texture
	//glBindImageTexture(3, m_buffer3d, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32F);
	////glBindImageTexture(3, tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);


	//// For each triangle, do rasterization
}

void CheckProgramInfoLog(GLuint program_id)
{
	if (program_id == 0)
		return;

	int log_length = 0;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 0)
	{
		char* buffer = new char[log_length];
		int   written = 0;
		glGetProgramInfoLog(program_id, log_length, &written, buffer);
		printf("%s\n", buffer);
		delete[] buffer;
	}
}

void CheckShaderInfoLog(GLuint shader_id)
{
	if (shader_id == 0)
		return;

	int log_length = 0;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 0)
	{
		char* buffer = new char[log_length];
		int   written = 0;
		glGetShaderInfoLog(shader_id, log_length, &written, buffer);
		printf("%s\n", buffer);
		delete[] buffer;
	}
}

GLint CheckShaderCompileStatus(GLuint shader_id)
{
	GLint status = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
	return status;
}

GLint CheckProgramLinkStatus(GLuint program_id)
{
	GLint status = GL_FALSE;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	return status;
}

GLuint CreateShaderFromSource(GLenum shader_type, const char* source)
{
	if (source == NULL)
		return 0;

	switch (shader_type)
	{
	case GL_COMPUTE_SHADER:	printf("compute shader creation ..."); break;
	default: return 0;
	}

	GLuint shader_id = glCreateShader(shader_type);
	if (shader_id == 0)
		return 0;

	glShaderSource(shader_id, 1, &source, NULL);
	glCompileShader(shader_id);
	if (CheckShaderCompileStatus(shader_id) != GL_TRUE)
	{
		printf("failed.\n");
		CheckShaderInfoLog(shader_id);
		glDeleteShader(shader_id);
		return 0;
	}
	else
	{
		printf("successfull.\n");
		return shader_id;
	}
}

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

GLuint CreateShaderFromFile(GLenum shader_type, const char* file_name, const char* preprocessor = NULL)
{
	char* buffer = ReadFile(file_name);
	if (buffer == NULL)
	{
		printf("Shader creation failed, input file is empty or missing!\n");
		return 0;
	}

	GLuint shader_id = 0;
	if (preprocessor)
	{
		std::string temp = buffer;
		std::size_t insertIdx = temp.find("\n", temp.find("#version"));
		temp.insert((insertIdx != std::string::npos) ? insertIdx : 0, std::string("\n") + preprocessor + "\n\n");
		shader_id = CreateShaderFromSource(shader_type, temp.c_str());
	}
	else
		shader_id = CreateShaderFromSource(shader_type, buffer);

	delete[] buffer;
	return shader_id;
}

bool CreateComputeShaderProgramFromFile(GLuint& programId, const char* cs, const char* preprocessor = NULL)
{
	// Create shader program object
	GLuint shader_id = CreateShaderFromFile(GL_COMPUTE_SHADER, cs, NULL);
	GLuint pr_id = glCreateProgram();
	if (shader_id == 0)
	{
		glDeleteProgram(pr_id);
		return false;
	}
	glAttachShader(pr_id, shader_id);
	glDeleteShader(shader_id);
	glLinkProgram(pr_id);
	if (!CheckProgramLinkStatus(pr_id))
	{
		CheckProgramInfoLog(pr_id);
		printf("Program linking failed!\n");
		glDeleteProgram(pr_id);
		return false;
	}

	// Remove program from OpenGL and update internal list
	glDeleteProgram(programId);
	programId = pr_id;

	return true;
}

void ComputeShader::InitShader(const char* file, glm::ivec3 sizes)
{
	bool ret = CreateComputeShaderProgramFromFile(m_program, file, NULL);
	m_sizes = sizes;
}

void ComputeShader::LaunchComputeShader(GLuint tex, GLuint format)
{
	glUseProgram(m_program);
	glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, format);
	printOpenGLError();
	glDispatchCompute(m_sizes.x, m_sizes.y, m_sizes.z);
	printOpenGLError();

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	printOpenGLError();
	glUseProgram(0);
}