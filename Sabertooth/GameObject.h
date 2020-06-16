#include <GL\glew.h>
#include "Piece.cpp"
#include <iostream>
#include <vector>
#include "Movement.cpp"
#include "Color.cpp"

using namespace std;

class GameObject
{
public:
	GameObject();
	GameObject(int id, bool isBlack, GLuint tid, Piece piece, vector<Movement>movements);
	void setVao(GLuint value);
	void setId(int value);
	void setTid(GLuint value);
	void setPiece(Piece value);
	void setMovements(vector<Movement>movements);
	void setColor(Color value);
	int vao, id, tid, currentRow, currentCol, quantidadeMov;
	bool isFirstMove;
	Piece piece;
	vector<Movement>movements;
	Color color;
};