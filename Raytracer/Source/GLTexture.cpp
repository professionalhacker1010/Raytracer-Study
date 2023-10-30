// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021
#include "stdafx.h"
//#include <gl/glew.h>
#include "GLTexture.h"

#include "Util.h"

// OpenGL texture wrapper class
GLTexture::GLTexture(unsigned int w, unsigned int h, unsigned int type)
{
	width = w;
	height = h;
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	if (type == DEFAULT)
	{
		// regular texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if (type == INTTARGET)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	else /* type == FLOAT */
	{
		// floating point texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	CheckGL();
}

GLTexture::~GLTexture()
{
	glDeleteTextures(1, &ID);
	CheckGL();
}

void GLTexture::Bind(const unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
	CheckGL();
}

void GLTexture::CopyFrom(Surface* src)
{
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, src->pixels);
	CheckGL();
}

void GLTexture::CopyTo(Surface* dst)
{
	glBindTexture(GL_TEXTURE_2D, ID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst->pixels);
	CheckGL();
}