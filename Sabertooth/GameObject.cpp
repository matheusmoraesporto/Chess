#include "GameObject.h"

GameObject::GameObject()
{

}

GameObject::GameObject(int id, bool isBlack, GLuint tid, Piece piece, vector<Movement>movements)
{
	setId(id);
	setTid(tid);
	setPiece(piece);
	setMovements(movements);
	setColor(isBlack ? Color::Black : Color::White);
	this->isFirstMove = true;
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

void GameObject::setMovements(vector<Movement>value)
{
	this->movements = value;
}

void GameObject::setColor(Color color)
{
	this->color = color;
}