// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021

#include "stdafx.h"
#include "Shader.h"
#include "Error.h"

// Shader class implementation
Shader::Shader(const char* vfile, const char* pfile, bool fromString)
{
	if (fromString)
	{
		Compile(vfile, pfile);
	}
	else
	{
		Init(vfile, pfile);
	}
}

Shader::~Shader()
{
	glDetachShader(ID, pixel);
	glDetachShader(ID, vertex);
	glDeleteShader(pixel);
	glDeleteShader(vertex);
	glDeleteProgram(ID);
	CheckGL();
}

void Shader::Init(const char* vfile, const char* pfile)
{
	std::string vsText = TextFileRead(vfile);
	std::string fsText = TextFileRead(pfile);
	FATALERROR_IF(vsText.size() == 0, "File %s not found", vfile);
	FATALERROR_IF(fsText.size() == 0, "File %s not found", pfile);
	const char* vertexText = vsText.c_str();
	const char* fragmentText = fsText.c_str();
	Compile(vertexText, fragmentText);
}

void Shader::Compile(const char* vtext, const char* ftext)
{
	vertex = glCreateShader(GL_VERTEX_SHADER);
	pixel = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertex, 1, &vtext, 0);
	glCompileShader(vertex);
	CheckShader(vertex, vtext, ftext);
	glShaderSource(pixel, 1, &ftext, 0);
	glCompileShader(pixel);
	CheckShader(pixel, vtext, ftext);
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, pixel);
	glBindAttribLocation(ID, 0, "pos");
	glBindAttribLocation(ID, 1, "tuv");
	glLinkProgram(ID);
	CheckProgram(ID, vtext, ftext);
	CheckGL();
}

void Shader::Bind()
{
	glUseProgram(ID);
	CheckGL();
}

void Shader::Unbind()
{
	glUseProgram(0);
	CheckGL();
}

void Shader::SetInputTexture(uint slot, const char* name, GLTexture* texture)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture->ID);
	glUniform1i(glGetUniformLocation(ID, name), slot);
	CheckGL();
}

void Shader::SetInputTexture(uint slot, const char* name)
{
	glUniform1i(glGetUniformLocation(ID, name), slot);
	CheckGL();
}

void Shader::SetInputMatrix(const char* name, const Mat4& matrix)
{
	const GLfloat* data = (const GLfloat*)&matrix;
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, data);
	CheckGL();
}

void Shader::SetFloat(const char* name, const float v)
{
	glUniform1f(glGetUniformLocation(ID, name), v);
	CheckGL();
}

void Shader::SetInt(const char* name, const int v)
{
	glUniform1i(glGetUniformLocation(ID, name), v);
	CheckGL();
}

void Shader::SetUInt(const char* name, const uint v)
{
	glUniform1ui(glGetUniformLocation(ID, name), v);
	CheckGL();
}

