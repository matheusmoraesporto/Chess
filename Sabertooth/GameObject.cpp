#include "GameObject.h"

GameObject::GameObject(int id, int col, int row, GLuint tid, Piece piece, vector<Movement>movements)
{
	setId(id);
	setCurrentRow(row);
	setCurrentCol(col);
	setTid(tid);
	setPiece(piece);
	setMovements(movements);
}

void GameObject::setVao(GLuint value)
{
	this->vao = value;
}

void GameObject::setId(int value)
{
	this->id = value;
}

void GameObject::setTid(GLuint value)
{
	this->tid = value;
}

void GameObject::setPiece(Piece value)
{
	this->piece = value;
}

void GameObject::setCurrentRow(int value)
{
	this->currentRow = value;
}

void GameObject::setCurrentCol(int value)
{
	this->currentCol = value;
}

void GameObject::setMovements(vector<Movement>value)
{
	this->movements = value;
}