#include <glm/glm.hpp>

class Tile {
public:
	int id, idPiece;
	bool isSelected, canPlay;
	//left point
	float Ax, Ay;
	//top point
	float Bx, By;
	//bottom point
	float Cx, Cy;
	//right point
	float Dx, Dy;
	glm::vec3 colorsRGB;

	Tile();
	Tile(int id, float x0, float y0, float th, float tw);
	void generateColor(int row, int col);
	void setIdPiece(int value);
};