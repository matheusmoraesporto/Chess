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
	GameObject(int id, bool isBlack, GLuint tid, Piece piece, vector<Movement>movements, int row, int col);
	void setVao(GLuint value);
	void setId(int value);
	void setTid(GLuint value);
	void setPiece(Piece value);
	void setMovements(vector<Movement>movements);
	void setColor(Color value);
	void setCurrentRow(int value);
	void setCurrentCol(int value);
	int vao, id, tid, currentRow, currentCol;
	bool isFirstMove;
	Piece piece;
	vector<Movement>movements;
	Color color;
};