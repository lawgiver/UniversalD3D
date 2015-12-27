#include "stdafx.h"
#include "DrawBuffer.h"

static_assert(sizeof(Color) == 4, "Sizeof Color is not 4!");

DrawBuffer::DrawBuffer(BYTE *_buffer, int _width, int _height)
	: buffer(_buffer), width(_width), height(_height), size(4*_width*_height)
{

}


DrawBuffer::~DrawBuffer()
{

}

void DrawBuffer::fillZero()
{
	memset(this->buffer, 0x0, this->size);
}

void DrawBuffer::drawRectangle(int x, int y, int w, int h, Color color)
{
	//DebugBreak();

	for (int _y = y; _y <= y + h; _y++)
	{
		for (int _x = x; _x <= x + w; _x++)
		{
			BYTE *pixel = this->buffer +
							(_y * width + _x) * 4;

			memcpy(pixel, &color, 4);
		}
	}
}
