#pragma once
#include "../../common/GLEW/glew.h"
#include <stdio.h>

#ifndef GLERROR_H
#define GLERROR_H

//#define printOpenGLError() printOglError(__FILE__, __LINE__)
#define printOpenGLError() 

int printOglError(char *file, int line);


#endif // !GLERROR_H
