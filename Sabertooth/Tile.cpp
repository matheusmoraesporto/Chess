#include "Tile.h"

Tile::Tile()
{
}

Tile::Tile(int id, float x0, float y0, float th, float tw)
{
	this->id = id;
	this->idPiece = 0;
	isVisible = true;
	isSelected = false;

	//left point
	Ax = x0;
	Ay = y0 + th / 2.0f;

	//top point
	Bx = x0 + tw / 2.0f;
	By = y0;

	//bottom point
	Cx = x0 + tw / 2.0f;
	Cy = y0 + th;

	//right point
	Dx = x0 + tw;
	Dy = y0 + th / 2.0f;
}
//
//void Tile::setColor(int R, int G, int B)
//{
//	float r;
//	float g;
//	float b;
//
//	r = R / 255.0f;
//	g = G / 255.0f;
//	b = B / 255.0f;
//
//	colorsRGB = glm::vec3(r, g, b);
//}

void Tile::generateColor(int row, int col)
{
	bool isLight = (row % 2 == 0 && col % 2 == 0) || (row % 2 != 0 && col % 2 != 0);

	float r;
	float g;
	float b;

	if (isLight)
	{
		if (this->isSelected)
		{
			r = 249 / 255.0f;
			g = 124 / 255.0f;
			b = 56 / 255.0f;
		}
		else
		{
			r = 15 / 255.0f;
			g = 39 / 255.0f;
			b = 111 / 255.0f;
		}
	}
	else
	{
		if (this->isSelected)
		{
			r = 255 / 255.0f;
			g = 155 / 255.0f;
			b = 73 / 255.0f;
		}
		else
		{
			r = 204 / 255.0f;
			g = 238 / 255.0f;
			b = 255 / 255.0f;
		}
	}

	colorsRGB = glm::vec3(r, g, b);
}

void Tile::setIdPiece(int value)
{
	this->idPiece = value;
}