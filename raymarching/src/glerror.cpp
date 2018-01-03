#include "glerror.h"

int printOglError(char *file, int line)
{

GLenum glErr;
int    retCode = 0;

glErr = glGetError();
if (glErr != GL_NO_ERROR)
{
	printf("glError %u in file %s @ line %d: %s\n", glErr,
		file, line, gluErrorString(glErr));
	retCode = 1;
}
return retCode;
}