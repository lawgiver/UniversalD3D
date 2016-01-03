
#include "stdafx.h"
#include "Draw.h"
#include "DrawBuffer.h"

void draw(DrawBuffer &drawBuffer)
{
	drawBuffer.fillZero();

	Color color{255, 255, 255 , 100};
	drawBuffer.drawRectangle(50, 50, 50, 50, color);
}
