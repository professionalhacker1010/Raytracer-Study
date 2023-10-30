#pragma once
// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021

// 32-bit surface container
class Surface
{
	enum { OWNER = 1 };
public:
	// constructor / destructor
	Surface() = default;
	Surface(int w, int h, unsigned int* buffer);
	Surface(int w, int h);
	Surface(const char* file);
	~Surface();
	// operations
	void InitCharset();
	void SetChar(int c, const char* c1, const char* c2, const char* c3, const char* c4, const char* c5);
	void Print(const char* t, int x1, int y1, unsigned int c);
	void Clear(unsigned int c);
	void Line(float x1, float y1, float x2, float y2, unsigned int c);
	void Plot(int x, int y, unsigned int c);
	void LoadImageFile(const char* file);
	void CopyTo(Surface* dst, int x, int y);
	void Box(int x1, int y1, int x2, int y2, unsigned int color);
	void Bar(int x1, int y1, int x2, int y2, unsigned int color);
	// attributes
	unsigned int* pixels = 0;
	int width = 0, height = 0;
	bool ownBuffer = false;
};