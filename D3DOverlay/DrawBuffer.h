#pragma once

struct Color
{
	BYTE r, g, b, a;
};

class DrawBuffer
{
public:
	DrawBuffer(BYTE *buffer, int width, int height);
	~DrawBuffer();

	void fillZero();
	void drawRectangle(int x, int y, int w, int h, Color col);

	int getHeight() { return this->height; }
	int getWidth() { return this->width; }

private:

	BYTE *buffer;
	int size;
	int width, height;
};

