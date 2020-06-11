#include <glm/glm.hpp>

class Tile {
public:
	int id;
	glm::vec3 colorsRGB;
	bool isVisible;
	bool isSelected;
	bool isWalking;
	bool isMortal;
	int idPiece;

	//left point
	float Ax, Ay;
	//top point
	float Bx, By;
	//bottom point
	float Cx, Cy;
	//right point
	float Dx, Dy;

	Tile();
	Tile(int id, float x0, float y0, float th, float tw);
	//void setColor(int R, int G, int B);
	void generateColor(int row, int col);
	void setIdPiece(int value);
};