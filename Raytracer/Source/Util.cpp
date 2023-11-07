//#include "stdafx.h"
#include "Util.h"
#include <stdlib.h>
#include <fstream>
#include <string>

//GLuint CreateVBO(const GLfloat* data, const uint size)
//{
//	GLuint id;
//	glGenBuffers(1, &id);
//	glBindBuffer(GL_ARRAY_BUFFER, id);
//	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
//	CheckGL();
//	return id;
//}

//void BindVBO(const uint idx, const uint N, const GLuint id)
//{
//	glEnableVertexAttribArray(idx);
//	glBindBuffer(GL_ARRAY_BUFFER, id);
//	glVertexAttribPointer(idx, N, GL_FLOAT, GL_FALSE, 0, (void*)0);
//	CheckGL();
//}


// Perlin noise implementation - https://stackoverflow.com/questions/29711668/perlin-noise-generation
static int numX = 512, numY = 512, numOctaves = 7, primeIndex = 0;
static float persistence = 0.5f;
static int primes[10][3] = {
	{ 995615039, 600173719, 701464987 }, { 831731269, 162318869, 136250887 }, { 174329291, 946737083, 245679977 },
	{ 362489573, 795918041, 350777237 }, { 457025711, 880830799, 909678923 }, { 787070341, 177340217, 593320781 },
	{ 405493717, 291031019, 391950901 }, { 458904767, 676625681, 424452397 }, { 531736441, 939683957, 810651871 },
	{ 997169939, 842027887, 423882827 }
};
static float Noise(const int i, const int x, const int y)
{
	int n = x + y * 57;
	n = (n << 13) ^ n;
	const int a = primes[i][0], b = primes[i][1], c = primes[i][2];
	const int t = (n * (n * n * a + b) + c) & 0x7fffffff;
	return 1.0f - (float)t / 1073741824.0f;
}
static float SmoothedNoise(const int i, const int x, const int y)
{
	const float corners = (Noise(i, x - 1, y - 1) + Noise(i, x + 1, y - 1) + Noise(i, x - 1, y + 1) + Noise(i, x + 1, y + 1)) / 16;
	const float sides = (Noise(i, x - 1, y) + Noise(i, x + 1, y) + Noise(i, x, y - 1) + Noise(i, x, y + 1)) / 8;
	const float center = Noise(i, x, y) / 4;
	return corners + sides + center;
}
static float Interpolate(const float a, const float b, const float x)
{
	const float ft = x * 3.1415927f, f = (1 - cosf(ft)) * 0.5f;
	return a * (1 - f) + b * f;
}
static float InterpolatedNoise(const int i, const float x, const float y)
{
	const int integer_X = (int)x, integer_Y = (int)y;
	const float fractional_X = x - integer_X, fractional_Y = y - integer_Y;
	const float v1 = SmoothedNoise(i, integer_X, integer_Y);
	const float v2 = SmoothedNoise(i, integer_X + 1, integer_Y);
	const float v3 = SmoothedNoise(i, integer_X, integer_Y + 1);
	const float v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1);
	const float i1 = Interpolate(v1, v2, fractional_X);
	const float i2 = Interpolate(v3, v4, fractional_X);
	return Interpolate(i1, i2, fractional_Y);
}
float Noise2D(const float x, const float y)
{
	float total = 0, frequency = (float)(2 << numOctaves), amplitude = 1;
	for (int i = 0; i < numOctaves; ++i)
	{
		frequency /= 2, amplitude *= persistence;
		total += InterpolatedNoise((primeIndex + i) % 10, x / frequency, y / frequency) * amplitude;
	}
	return total / frequency;
}

std::string TextFileRead(const char* _File)
{	
	std::ifstream s(_File);
	std::string str((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
	s.close();
	return str;
}