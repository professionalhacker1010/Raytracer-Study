// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021

#pragma once
#include "Surface.h"
//#include "External/glad.h"
//#include <gl/glut.h>

// OpenGL texture wrapper
class GLTexture
{
public:
	enum { DEFAULT = 0, FLOAT = 1, INTTARGET = 2 };
	// constructor / destructor
	GLTexture(unsigned int width, unsigned int height, unsigned int type = DEFAULT);
	~GLTexture();
	// methods
	void Bind(const unsigned int slot = 0);
	void CopyFrom(Surface* src);
	void CopyTo(Surface* dst);
public:
	// public data members
	unsigned int ID = 0;
	unsigned int width = 0, height = 0;
};

// template function access
GLTexture* GetRenderTarget();