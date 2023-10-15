#pragma once
#include "Constants.h"
#include <gl/glew.h>

class RenderQuad {
public:
	RenderQuad();
	~RenderQuad();
	void Draw(const void* data);
private:
	GLuint VAO, VBO, EBO;
	GLuint vertexShader, fragmentShader, shaderProgram;
	GLuint* texture;

	// Vertex data for a fullscreen quad
	const float vertices[8] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};

	const unsigned int indices[6] = {
		0, 1, 2,
		2, 3, 0
	};

	// Vertex shader source code
	const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 position;
    out vec2 TexCoords;

    void main()
    {
        gl_Position = vec4(position.x, position.y, 0.0, 1.0);
        TexCoords = vec2((position.x / 2.0) - 0.5, (position.y / 2.0) - 0.5);
    }
)";

	// Fragment shader source code
	const char* fragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;
	uniform sampler2D myTexture;

    void main()
    {
        FragColor = texture(myTexture, TexCoords);
		//FragColor = vec4(TexCoords.x, TexCoords.y, 0.0, 1.0); // Example color output
    }
)";
};