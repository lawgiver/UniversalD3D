
#include "stdafx.h"
#include "Draw.h"
#include "DrawBuffer.h"

void draw(DrawBuffer &drawBuffer)
{
	Color color{255, 255, 255 , 100};

	drawBuffer.drawRectangle(0, 0, drawBuffer.getWidth(), drawBuffer.getHeight(), color);
}
