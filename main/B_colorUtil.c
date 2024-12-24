#include "B_colorUtil.h"

void B_ColorLerp(B_color_t* c1, B_color_t* c2, float t, B_color_t* out)
{
	if (t > 1)
		t = 1;
	out->red = c1->red + t * (c2->red - c1->red);
	out->green = c1->green + t * (c2->green - c1->green);
	out->blue = c1->blue + t * (c2->blue - c1->blue);
}
