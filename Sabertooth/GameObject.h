#include <GL\glew.h>
#include "Piece.cpp"
#include <iostream>
#include <vector>
#include "Movement.cpp"

using namespace std;

class GameObject
{
public:
	GameObject(int id, int col, int row, GLuint tid, Piece piece, vector<Movement>movements);
	void setVao(GLuint value);
	void setId(int value);
	void setTid(GLuint value);
	void setPiece(Piece value);
	void setCurrentRow(int value);
	void setCurrentCol(int value);
	void setMovements(vector<Movement>movements);
	int vao, id, tid, currentRow, currentCol, quantidadeMov;
	bool allowMove;
	Piece piece;
	vector<Movement>movements;
};