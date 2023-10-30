// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021

#pragma once
#include "Constants.h"
#include "GLTexture.h"
#include "Math.h"

class Shader
{
public:
	// constructor / destructor
	Shader(const char* vfile, const char* pfile, bool fromString);
	~Shader();
	// methods
	void Init(const char* vfile, const char* pfile);
	void Compile(const char* vtext, const char* ftext);
	void Bind();
	void SetInputTexture(uint slot, const char* name, GLTexture* texture);
	void SetInputTexture(uint slot, const char* name);
	void SetInputMatrix(const char* name, const Mat4& matrix);
	void SetFloat(const char* name, const float v);
	void SetInt(const char* name, const int v);
	void SetUInt(const char* name, const uint v);
	void Unbind();
private:
	// data members
	uint vertex = 0;	// vertex shader identifier
	uint pixel = 0;		// fragment shader identifier
	uint ID = 0;		// shader program identifier
};

void CheckProgram(uint id, const char* vshader, const char* fshader);
void CheckShader(uint shader, const char* vshader, const char* fshader);