#include <GL/glew.h> /* include GLEW and new version of GL on Windows */
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "Tile.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "GameObject.h"

using namespace std;

#pragma region Properties

// Altura e largura dos tiles
constexpr auto TILE_WIDTH = 80;
constexpr auto TILE_HEIGHT = 40;
constexpr auto NUM_COLS = 8;
constexpr auto NUM_ROWS = 8;

// Constantes para tamanho de tela
const int WIDTH = NUM_ROWS * TILE_WIDTH;
const int HEIGHT = NUM_COLS * TILE_HEIGHT;

// Guada o VAO das rainhas para ser usado quando um pe�o chegar ao outro lado do tabuleiro
int BlackQueenVAO;
int WhiteQueenVAO;

bool canPlayWhite = true;
bool canPlayBlack = false;

// Tiles
Tile matrixColors[NUM_ROWS][NUM_COLS] = {};

// Pe�as do jogo
vector<GameObject> whiteSprites;
vector<GameObject> blackSprites;

const int sumTilesHeigth = NUM_ROWS * TILE_HEIGHT;

// Array com as posi��es que poder�o ser jogadas, serve para controlar o movimento da pe�a.
// Exemplo: se o jogador selecionar a pe�a bispo, todos os prov�veis tiles para onde o bispode pode se mover estar�o aqui.
vector<pair<int, int>> selectedPositions;


glm::mat4 matrix = glm::mat4(1);

int textureShader_programme;

int x = 0;

int lastSelectedColumn = -1;
int lastSelectedRow = -1;

#pragma endregion

int ConnectVertex(const char* v_shader, const char* f_shader)
{
	// identifica vs e o associa com vertex_shader
	int vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &v_shader, NULL);
	glCompileShader(vs);

	// identifica fs e o associa com fragment_shader
	int fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &f_shader, NULL);
	glCompileShader(fs);

	// identifica do programa, adiciona partes e faz "linkagem"
	int shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	return shader_programme;
}

#pragma region Sprite

// Carrega as sprites
void LoadImage(int id, bool isBlack, int row, int col, Piece piece)
{
	const char* img = "";
	float taxaIncremento = 0;
	float valueZ = 0;
	vector<Movement> pieceMovement;

	switch (piece)
	{
	case Piece::Bishop:
		img = isBlack ? "..\\Images\\BlackBishop.png" : "..\\Images\\WhiteBishop.png";
		pieceMovement = { Movement::Northwest, Movement::Northeast, Movement::Southeast, Movement::Southwest };
		break;

	case Piece::King:
		img = isBlack ? "..\\Images\\BlackKing.png" : "..\\Images\\WhiteKing.png";
		pieceMovement = { Movement::Northwest, Movement::Northeast, Movement::Southeast, Movement::Southwest, Movement::East, Movement::North, Movement::South, Movement::West };
		break;

	case Piece::Knight:
		img = isBlack ? "..\\Images\\BlackKnight.png" : "..\\Images\\WhiteKnight.png";
		pieceMovement = { Movement::InL };
		break;

	case Piece::Pawn:
		img = isBlack ? "..\\Images\\BlackPawn.png" : "..\\Images\\WhitePawn.png";
		pieceMovement = { isBlack ? Movement::North : Movement::South };
		break;

	case Piece::Queen:
		img = isBlack ? "..\\Images\\BlackQueen.png" : "..\\Images\\WhiteQueen.png";
		pieceMovement = { Movement::Northwest, Movement::Northeast, Movement::Southeast, Movement::Southwest, Movement::East, Movement::North, Movement::South, Movement::West };
		break;

	case Piece::Rook:
		img = isBlack ? "..\\Images\\BlackRook.png" : "..\\Images\\WhiteRook.png";
		pieceMovement = { Movement::North, Movement::South, Movement::East, Movement::West };
		break;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(img, &width, &height, &nrChannels, 0);

	if (data)
	{
		nrChannels == 3
			? glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data)
			: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);

		GameObject gameObj = GameObject::GameObject(id, isBlack, texture, piece, pieceMovement);

		isBlack ? blackSprites.push_back(gameObj) : whiteSprites.push_back(gameObj);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
}

void DiamondDrawCalculation(float& x, float& y, int row, int col)
{
	x = row * (TILE_WIDTH / 2.0f) + col * (TILE_WIDTH / 2.0f);
	y = row * (TILE_HEIGHT / 2.0f) - col * (TILE_HEIGHT / 2.0f) + sumTilesHeigth / 2.0f - (TILE_HEIGHT / 2.0f);
}

// faz o bind das textura e desenha a geometria
void Render(GLuint vao, GLuint texture, int sp)
{
	//Inicio do c�digo para carregar textura
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(sp, "sprite"), 0);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Transform(glm::mat4& mt, unsigned int& tl, int sp, float x, float y, float z)
{
	mt = glm::translate(mt, glm::vec3(x, y, z));
	glUniformMatrix4fv(tl, 1, GL_FALSE, glm::value_ptr(mt));
}

// Atribuis o offsetx e o offsetY por layer e gameobject
void DefineOffsetAndRender(int sp, float offsetx, float offsety, float z, GLuint vao, GLuint tid, glm::mat4 mt, GameObject& go, unsigned int& tl)
{
	float x, y;
	DiamondDrawCalculation(x, y, 0, 0);

	Transform(mt, tl, sp, x + 25 , y - 30, 0.0f);

	glUniform1f(glGetUniformLocation(sp, "offsetx"), offsetx);
	glUniform1f(glGetUniformLocation(sp, "offsety"), offsety);
	glUniform1f(glGetUniformLocation(sp, "layer_z"), z);

	Render(vao, tid, sp);
}

// Define os vertices das sprites. E faz a associa��o dos VAO e os VBO
void DefineGeometry(int id, GameObject& go)
{
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	go.setVao(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

#pragma region vertices
#pragma region White pieces
	// White rook
	GLfloat vertices1[] = {
		// positions			  // texture coords
		0.0f, 60.0f, 	  1.0, 1.0f,
		0.0f, 0.0f, 	  1.0f, 0.0f,
		30.0f, 60.0f, 	  0.0f, 1.0f,

		30.0f, 60.0f, 	  0.0, 1.0f,
		0.0f, 0.0f, 	  1.0f, 0.0f,
		30.0f, 0.0f, 	  0.0f, 0.0f,
	};

	// White knight
	GLfloat vertices2[] = {
		// positions			  // texture coords
		350.0f, 285.0f, 	  1.0, 1.0f,
		350.0f, 255.0f, 	  1.0f, 0.0f,
		370.0f, 285.0f, 	  0.0f, 1.0f,

		370.0f, 285.0f, 	  0.0, 1.0f,
		350.0f, 255.0f, 	  1.0f, 0.0f,
		370.0f, 255.0f, 	  0.0f, 0.0f,
	};

	// White bishop
	GLfloat vertices3[] = {
		// positions			  // texture coords
		390.0f, 265.0f, 	  1.0, 1.0f,
		390.0f, 235.0f, 	  1.0f, 0.0f,
		410.0f, 265.0f, 	  0.0f, 1.0f,

		410.0f, 265.0f, 	  0.0, 1.0f,
		390.0f, 235.0f, 	  1.0f, 0.0f,
		410.0f, 235.0f, 	  0.0f, 0.0f,
	};

	// White king
	GLfloat vertices4[] = {
		// positions			  // texture coords
		430.0f, 245.0f, 	  1.0, 1.0f,
		430.0f, 215.0f, 	  1.0f, 0.0f,
		450.0f, 245.0f, 	  0.0f, 1.0f,

		450.0f, 245.0f, 	  0.0, 1.0f,
		430.0f, 215.0f, 	  1.0f, 0.0f,
		450.0f, 215.0f, 	  0.0f, 0.0f,
	};

	// White queen
	GLfloat vertices5[] = {
		// positions			  // texture coords
		470.0f, 225.0f, 	  1.0, 1.0f,
		470.0f, 195.0f, 	  1.0f, 0.0f,
		490.0f, 225.0f, 	  0.0f, 1.0f,

		490.0f, 225.0f, 	  0.0, 1.0f,
		470.0f, 195.0f, 	  1.0f, 0.0f,
		490.0f, 195.0f, 	  0.0f, 0.0f,
	};

	// White bishop
	GLfloat vertices6[] = {
		// positions			  // texture coords
		510.0f, 205.0f, 	  1.0, 1.0f,
		510.0f, 175.0f, 	  1.0f, 0.0f,
		530.0f, 205.0f, 	  0.0f, 1.0f,

		530.0f, 205.0f, 	  0.0, 1.0f,
		510.0f, 175.0f, 	  1.0f, 0.0f,
		530.0f, 175.0f, 	  0.0f, 0.0f,
	};

	// White knight
	GLfloat vertices7[] = {
		// positions			  // texture coords
		550.0f, 185.0f, 	  1.0, 1.0f,
		550.0f, 155.0f, 	  1.0f, 0.0f,
		570.0f, 185.0f, 	  0.0f, 1.0f,

		570.0f, 185.0f, 	  0.0, 1.0f,
		550.0f, 155.0f, 	  1.0f, 0.0f,
		570.0f, 155.0f, 	  0.0f, 0.0f,
	};

	// White rook
	GLfloat vertices8[] = {
		// positions			  // texture coords
		590.0f, 165.0f, 	  1.0, 1.0f,
		590.0f, 135.0f, 	  1.0f, 0.0f,
		610.0f, 165.0f, 	  0.0f, 1.0f,

		610.0f, 165.0f, 	  0.0, 1.0f,
		590.0f, 135.0f, 	  1.0f, 0.0f,
		610.0f, 135.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices9[] = {
		// positions			  // texture coords
		270.0f, 285.0f, 	  1.0, 1.0f,
		270.0f, 255.0f, 	  1.0f, 0.0f,
		290.0f, 285.0f, 	  0.0f, 1.0f,

		290.0f, 285.0f, 	  0.0, 1.0f,
		270.0f, 255.0f, 	  1.0f, 0.0f,
		290.0f, 255.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices10[] = {
		// positions			  // texture coords
		310.0f, 265.0f, 	  1.0, 1.0f,
		310.0f, 235.0f, 	  1.0f, 0.0f,
		330.0f, 265.0f, 	  0.0f, 1.0f,

		330.0f, 265.0f, 	  0.0, 1.0f,
		310.0f, 235.0f, 	  1.0f, 0.0f,
		330.0f, 235.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices11[] = {
		// positions			  // texture coords
		350.0f, 245.0f, 	  1.0, 1.0f,
		350.0f, 215.0f, 	  1.0f, 0.0f,
		370.0f, 245.0f, 	  0.0f, 1.0f,

		370.0f, 245.0f, 	  0.0, 1.0f,
		350.0f, 215.0f, 	  1.0f, 0.0f,
		370.0f, 215.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices12[] = {
		// positions			  // texture coords
		390.0f, 225.0f, 	  1.0, 1.0f,
		390.0f, 195.0f, 	  1.0f, 0.0f,
		410.0f, 225.0f, 	  0.0f, 1.0f,

		410.0f, 225.0f, 	  0.0, 1.0f,
		390.0f, 195.0f, 	  1.0f, 0.0f,
		410.0f, 195.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices13[] = {
		// positions			  // texture coords
		430.0f, 205.0f,	  1.0, 1.0f,
		430.0f, 175.0f,	  1.0f, 0.0f,
		450.0f, 205.0f,	  0.0f, 1.0f,

		450.0f, 205.0f,	  0.0, 1.0f,
		430.0f, 175.0f,	  1.0f, 0.0f,
		450.0f, 175.0f,	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices14[] = {
		// positions			  // texture coords
		470.0f, 185.0f,	  1.0, 1.0f,
		470.0f, 155.0f,	  1.0f, 0.0f,
		490.0f, 185.0f,	  0.0f, 1.0f,

		490.0f, 185.0f,	  0.0, 1.0f,
		470.0f, 155.0f,	  1.0f, 0.0f,
		490.0f, 155.0f,	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices15[] = {
		// positions			  // texture coords
		510.0f, 165.0f, 	  1.0, 1.0f,
		510.0f, 135.0f, 	  1.0f, 0.0f,
		530.0f, 165.0f, 	  0.0f, 1.0f,

		530.0f, 165.0f, 	  0.0, 1.0f,
		510.0f, 135.0f, 	  1.0f, 0.0f,
		530.0f, 135.0f, 	  0.0f, 0.0f,
	};

	// White pawn
	GLfloat vertices16[] = {
		// positions			  // texture coords
		550.0f, 145.0f,	  1.0, 1.0f,
		550.0f, 115.0f,	  1.0f, 0.0f,
		570.0f, 145.0f,	  0.0f, 1.0f,

		570.0f, 145.0f,	  0.0, 1.0f,
		550.0f, 115.0f,	  1.0f, 0.0f,
		570.0f, 115.0f,	  0.0f, 0.0f,
	};
#pragma endregion
#pragma region Black pieces
	// Black rook
	GLfloat vertices17[] = {
		// positions			  // texture coords
		30.0f, 165.0f, +0.0f,	  1.0, 1.0f,
		30.0f, 135.0f, +0.0f,	  1.0f, 0.0f,
		50.0f, 165.0f, +0.0f,	  0.0f, 1.0f,

		50.0f, 165.0f, +0.0f,	  0.0, 1.0f,
		30.0f, 135.0f, +0.0f,	  1.0f, 0.0f,
		50.0f, 135.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black knight
	GLfloat vertices18[] = {
		// positions			  // texture coords
		70.0f, 145.0f, +0.0f,	  1.0, 1.0f,
		70.0f, 115.0f, +0.0f,	  1.0f, 0.0f,
		90.0f, 145.0f, +0.0f,	  0.0f, 1.0f,

		90.0f, 145.0f, +0.0f,	  0.0, 1.0f,
		70.0f, 115.0f, +0.0f,	  1.0f, 0.0f,
		90.0f, 115.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black bishop
	GLfloat vertices19[] = {
		// positions			  // texture coords
		110.0f, 125.0f, +0.0f,	  1.0, 1.0f,
		110.0f, 95.0f, +0.0f,	  1.0f, 0.0f,
		130.0f, 125.0f, +0.0f,	  0.0f, 1.0f,

		130.0f, 125.0f, +0.0f,	  0.0, 1.0f,
		110.0f, 95.0f, +0.0f,	  1.0f, 0.0f,
		130.0f, 95.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black king
	GLfloat vertices20[] = {
		// positions			  // texture coords
		150.0f, 105.0f, +0.0f,	  1.0, 1.0f,
		150.0f, 75.0f, +0.0f,	  1.0f, 0.0f,
		170.0f, 105.0f, +0.0f,	  0.0f, 1.0f,

		170.0f, 105.0f, +0.0f,	  0.0, 1.0f,
		150.0f, 75.0f, +0.0f,	  1.0f, 0.0f,
		170.0f, 75.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black queen
	GLfloat vertices21[] = {
		// positions			  // texture coords
		190.0f, 85.0f, +0.0f,	  1.0, 1.0f,
		190.0f, 55.0f, +0.0f,	  1.0f, 0.0f,
		210.0f, 85.0f, +0.0f,	  0.0f, 1.0f,

		210.0f, 85.0f, +0.0f,	  0.0, 1.0f,
		190.0f, 55.0f, +0.0f,	  1.0f, 0.0f,
		210.0f, 55.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black bishop
	GLfloat vertices22[] = {
		// positions			  // texture coords
		230.0f, 65.0f, +0.0f,	  1.0, 1.0f,
		230.0f, 35.0f, +0.0f,	  1.0f, 0.0f,
		250.0f, 65.0f, +0.0f,	  0.0f, 1.0f,

		250.0f, 65.0f, +0.0f,	  0.0, 1.0f,
		230.0f, 35.0f, +0.0f,	  1.0f, 0.0f,
		250.0f, 35.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black knight
	GLfloat vertices23[] = {
		// positions			  // texture coords
		270.0f, 45.0f, +0.0f,	  1.0, 1.0f,
		270.0f, 15.0f, +0.0f,	  1.0f, 0.0f,
		290.0f, 45.0f, +0.0f,	  0.0f, 1.0f,

		290.0f, 45.0f, +0.0f,	  0.0, 1.0f,
		270.0f, 15.0f, +0.0f,	  1.0f, 0.0f,
		290.0f, 15.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black rook
	GLfloat vertices24[] = {
		// positions			  // texture coords
		310.0f, 25.0f, +0.0f,	  1.0, 1.0f,
		310.0f, -5.0f, +0.0f,	  1.0f, 0.0f,
		330.0f, 25.0f, +0.0f,	  0.0f, 1.0f,

		330.0f, 25.0f, +0.0f,	  0.0, 1.0f,
		310.0f, -5.0f, +0.0f,	  1.0f, 0.0f,
		330.0f, -5.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices25[] = {
		// positions			  // texture coords
		70.0f, 185.0f, +0.0f,	  1.0, 1.0f,
		70.0f, 155.0f, +0.0f,	  1.0f, 0.0f,
		90.0f, 185.0f, +0.0f,	  0.0f, 1.0f,

		90.0f, 185.0f, +0.0f,	  0.0, 1.0f,
		70.0f, 155.0f, +0.0f,	  1.0f, 0.0f,
		90.0f, 155.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices26[] = {
		// positions			  // texture coords
		110.0f, 165.0f, +0.0f,	  1.0, 1.0f,
		110.0f, 135.0f, +0.0f,	  1.0f, 0.0f,
		130.0f, 165.0f, +0.0f,	  0.0f, 1.0f,

		130.0f, 165.0f, +0.0f,	  0.0, 1.0f,
		110.0f, 135.0f, +0.0f,	  1.0f, 0.0f,
		130.0f, 135.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices27[] = {
		// positions			  // texture coords
		150.0f, 145.0f, +0.0f,	  1.0, 1.0f,
		150.0f, 115.0f, +0.0f,	  1.0f, 0.0f,
		170.0f, 145.0f, +0.0f,	  0.0f, 1.0f,

		170.0f, 145.0f, +0.0f,	  0.0, 1.0f,
		150.0f, 115.0f, +0.0f,	  1.0f, 0.0f,
		170.0f, 115.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices28[] = {
		// positions			  // texture coords
		190.0f, 125.0f, +0.0f,	  1.0, 1.0f,
		190.0f, 95.0f, +0.0f,	  1.0f, 0.0f,
		210.0f, 125.0f, +0.0f,	  0.0f, 1.0f,

		210.0f, 125.0f, +0.0f,	  0.0, 1.0f,
		190.0f, 95.0f, +0.0f,	  1.0f, 0.0f,
		210.0f, 95.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices29[] = {
		// positions			  // texture coords
		230.0f, 105.0f, +0.0f,	  1.0, 1.0f,
		230.0f, 75.0f, +0.0f,	  1.0f, 0.0f,
		250.0f, 105.0f, +0.0f,	  0.0f, 1.0f,

		250.0f, 105.0f, +0.0f,	  0.0, 1.0f,
		230.0f, 75.0f, +0.0f,	  1.0f, 0.0f,
		250.0f, 75.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices30[] = {
		// positions			  // texture coords
		270.0f, 85.0f, +0.0f,	  1.0, 1.0f,
		270.0f, 55.0f, +0.0f,	  1.0f, 0.0f,
		290.0f, 85.0f, +0.0f,	  0.0f, 1.0f,

		290.0f, 85.0f, +0.0f,	  0.0, 1.0f,
		270.0f, 55.0f, +0.0f,	  1.0f, 0.0f,
		290.0f, 55.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices31[] = {
		// positions			  // texture coords
		310.0f, 65.0f, +0.0f,	  1.0, 1.0f,
		310.0f, 35.0f, +0.0f,	  1.0f, 0.0f,
		330.0f, 65.0f, +0.0f,	  0.0f, 1.0f,

		330.0f, 65.0f, +0.0f,	  0.0, 1.0f,
		310.0f, 35.0f, +0.0f,	  1.0f, 0.0f,
		330.0f, 35.0f, +0.0f,	  0.0f, 0.0f,
	};

	// Black pawn
	GLfloat vertices32[] = {
		// positions			  // texture coords
		350.0f, 45.0f, +0.0f,	  1.0, 1.0f,
		350.0f, 15.0f, +0.0f,	  1.0f, 0.0f,
		370.0f, 45.0f, +0.0f,	  0.0f, 1.0f,

		370.0f, 45.0f, +0.0f,	  0.0, 1.0f,
		350.0f, 15.0f, +0.0f,	  1.0f, 0.0f,
		370.0f, 15.0f, +0.0f,	  0.0f, 0.0f,
	};
#pragma endregion
#pragma endregion

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

// Faz a leitura e define o vao das sprites e as vincula com seu tile inicial
void ConfigPiece(int id, int row, int col, bool isBlack, Piece piece)
{
	LoadImage(id, isBlack, row, col, piece);

	GameObject sprite = isBlack ? blackSprites.back() : whiteSprites.back();

	DefineGeometry(id, isBlack ? blackSprites.back() : whiteSprites.back());

	glBindVertexArray(sprite.vao);

	if (isBlack) {

		matrixColors[row][col].setIdPiece(sprite.id);
	}
	else
	{
		int rowBlack = 7 - row;
		matrixColors[rowBlack][col].setIdPiece(sprite.id);
	}
}

// Configura as sprites, fazendo a leitura e definindo a geometria
void ConfigSprites()
{
	int idSprite = 0;

	for (size_t row = 0; row < 2; row++)
	{
		for (size_t col = 0; col < NUM_ROWS; col++)
		{
			Piece p;
			int idWhite = ++idSprite;
			int idBlack = idWhite + 16;

			if (row > 0)
			{
				p = Piece::Pawn;
			}
			else
			{
				switch (idSprite)
				{
				case 1:
				case 8:
					p = Piece::Rook;
					break;
				case 2:
				case 7:
					p = Piece::Knight;
					break;
				case 3:
				case 6:
					p = Piece::Bishop;
					break;
				case 4:
					p = Piece::King;
					break;
				case 5:
					p = Piece::Queen;
					break;
				}
			}

			ConfigPiece(idWhite, row, col, false, p);
			ConfigPiece(idBlack, row, col, true, p);
		}
	}
}

#pragma endregion

#pragma region DiamondMap



void CreateMatrixColors()
{
	int idTile = 1;

	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int col = 0; col < NUM_COLS; col++)
		{
			float x0, y0;

			DiamondDrawCalculation(x0, y0, row, col);

			Tile t = Tile(idTile++, x0, y0, TILE_HEIGHT, TILE_WIDTH);

			t.generateColor(row, col);

			matrixColors[row][col] = t;
		}
	}
}

void RenderDiamondMap(glm::mat4 matrix, int sp)
{
	for (int i = 0; i < NUM_ROWS; i++)
	{
		for (int j = 0; j < NUM_COLS; j++)
		{
			float x, y;

			DiamondDrawCalculation(x, y, i, j);

			matrix = glm::mat4(1);

			matrix = glm::translate(matrix, glm::vec3(x, y, 0.0));

			//define aonde desenhar
			glUniformMatrix4fv(glGetUniformLocation(sp, "matrix_OBJ"), 1, GL_FALSE, glm::value_ptr(matrix));

			Tile tile = matrixColors[i][j];

			glUniform3fv(glGetUniformLocation(sp, "colorValues"), 1, glm::value_ptr(tile.colorsRGB));

			//Desenha ret�ngulo para o tile
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
	}
}

void DiamondClickCalculation(float xPos, float yPos, int& row, int& col) {
	double x = (double)xPos;
	double y = ((double)yPos) - (((double)sumTilesHeigth) / 2.0);
	double tw = (double)TILE_WIDTH;
	double th = (double)TILE_HEIGHT;
	double rowClick = (((2.0 * y / th) + (2.0 * x / tw))) / 2.0;
	double columnClick = (2.0 * x / tw) - rowClick;

	row = (int)rowClick;
	col = (int)columnClick;
}

bool TestPointCollision(float RefenceX, float RefenceY, float Bx, float By, float Cx, float Cy, float Px, float Py) {
	float ABx = Bx - RefenceX;
	float ABy = By - RefenceY;
	float ABmodule = sqrt(pow(ABx, 2) + pow(ABy, 2));

	float normalABx = ABx / ABmodule;
	float normalABy = ABy / ABmodule;

	float ACx = Cx - RefenceX;
	float ACy = Cy - RefenceY;
	float ACmodule = sqrt(pow(ACx, 2) + pow(ACy, 2));

	float normalACx = ACx / ACmodule;
	float normalACy = ACy / ACmodule;

	float APx = Px - RefenceX;
	float APy = Py - RefenceY;
	float APmodule = sqrt(pow(APx, 2) + pow(APy, 2));

	float normalAPx = APx / APmodule;
	float normalAPy = APy / APmodule;

	float theta = acos(normalABx * normalAPx + normalABy * normalAPy);
	float alpha = acos(normalABx * normalACx + normalABy * normalACy);
	float betha = acos(normalACx * normalAPx + normalACy * normalAPy);

	bool collide = 0.001 > abs(alpha - (theta + betha));
	return collide;
}

GameObject GetPiece(int id, bool setTranslate = false)
{
	GameObject retorno;

	for each (GameObject white in whiteSprites)
	{
		if (white.id == id)
		{
			retorno = white;

			if (setTranslate)
			{
				white.translate = true;
			}

			break;
		}
	}

	if (retorno.id == 0)
	{
		for each (GameObject black in blackSprites)
		{
			if (black.id == id)
			{
				retorno = black;

				if (setTranslate)
				{
					black.translate = true;
				}

				break;
			}
		}
	}

	return retorno;
}

void AddSelectedPosition(int r, int c)
{
	std::pair<int, int> p = make_pair(r, c);
	selectedPositions.push_back(p);
}

void markTile(int r, int c, bool canPlay, bool add)
{
	matrixColors[r][c].canPlay = canPlay;
	matrixColors[r][c].generateColor(r, c);

	if (add)
	{
		AddSelectedPosition(r, c);
	}
}

void markMovements(int rowClick, int columnClick, GameObject piece)
{
	for each (Movement movement in piece.movements)
	{
		if (movement == Movement::North)
		{
			int moveNorth = NUM_ROWS;

			if (piece.piece == Piece::Pawn)
			{
				// Define um movimento a mais, pois a condi��o do for � < e n�o <=
				moveNorth = piece.isFirstMove ? 3 : 2;

				// Regra para ataque do pi�o
				if (rowClick + 1 < NUM_ROWS
					&& columnClick + 1 < NUM_COLS
					&& matrixColors[rowClick + 1][columnClick + 1].idPiece > 0
					&& GetPiece(matrixColors[rowClick + 1][columnClick + 1].idPiece).color != piece.color)
				{
					markTile(rowClick + 1, columnClick + 1, true, true);
				}

				if (rowClick + 1 < NUM_ROWS
					&& columnClick - 1 > 0
					&& matrixColors[rowClick + 1][columnClick - 1].idPiece > 0
					&& GetPiece(matrixColors[rowClick + 1][columnClick - 1].idPiece).color != piece.color)
				{
					markTile(rowClick + 1, columnClick - 1, true, true);
				}
			}

			if (piece.piece == Piece::King)
			{
				moveNorth = 2;
			}

			for (size_t i = 1; i < moveNorth; i++)
			{
				if (rowClick + i < NUM_ROWS)
				{
					GameObject nextTile = GetPiece(matrixColors[rowClick + i][columnClick].idPiece);

					if (nextTile.color == piece.color)
					{
						break;
					}

					markTile(rowClick + i, columnClick, true, true);

					if (nextTile.color != piece.color && nextTile.color != NULL)
					{
						break;
					}
				}
			}
		}
		else if (movement == Movement::East)
		{
			int moveEast = NUM_COLS;

			if (piece.piece == Piece::King)
			{
				moveEast = 1;
			}

			for (size_t i = 1; i <= moveEast; i++)
			{
				if (columnClick - i < NUM_ROWS)
				{
					GameObject nextTile = GetPiece(matrixColors[rowClick][columnClick - i].idPiece);

					if (nextTile.color == piece.color)
					{
						break;
					}

					markTile(rowClick, columnClick - i, true, true);

					if (nextTile.color != piece.color && nextTile.color != NULL)
					{
						break;
					}
				}
			}
		}
		else if (movement == Movement::InL)
		{
			// Sobe para o oeste e para o norte
			if (rowClick + 1 < NUM_COLS
				&& columnClick + 2 < NUM_COLS
				&& GetPiece(matrixColors[rowClick + 1][columnClick + 2].idPiece).color != piece.color)
			{
				markTile(rowClick + 1, columnClick + 2, true, true);
			}

			if (rowClick + 2 < NUM_COLS
				&& columnClick + 1 < NUM_COLS
				&& GetPiece(matrixColors[rowClick + 2][columnClick + 1].idPiece).color != piece.color)
			{
				markTile(rowClick + 2, columnClick + 1, true, true);
			}

			// Desce para o leste e sobe para o norte
			if (rowClick + 1 < NUM_COLS
				&& columnClick - 2 >= 0
				&& GetPiece(matrixColors[rowClick + 1][columnClick - 2].idPiece).color != piece.color)
			{
				markTile(rowClick + 1, columnClick - 2, true, true);
			}

			if (rowClick + 2 < NUM_COLS
				&& columnClick - 1 >= 0
				&& GetPiece(matrixColors[rowClick + 2][columnClick - 1].idPiece).color != piece.color)
			{
				markTile(rowClick + 2, columnClick - 1, true, true);
			}

			// Desce para o leste e desce para o sul
			if (rowClick - 1 >= 0
				&& columnClick - 2 >= 0
				&& GetPiece(matrixColors[rowClick - 1][columnClick - 2].idPiece).color != piece.color)
			{
				markTile(rowClick - 1, columnClick - 2, true, true);
			}

			if (rowClick - 2 >= 0
				&& columnClick - 1 >= 0
				&& GetPiece(matrixColors[rowClick - 2][columnClick - 1].idPiece).color != piece.color)
			{
				markTile(rowClick - 2, columnClick - 1, true, true);
			}

			// Sobe para o oeste e desce para o sul
			if (rowClick - 1 >= 0
				&& columnClick + 2 < NUM_COLS
				&& GetPiece(matrixColors[rowClick - 1][columnClick + 2].idPiece).color != piece.color)
			{
				markTile(rowClick - 1, columnClick + 2, true, true);
			}

			if (rowClick - 2 >= 0
				&& columnClick + 1 < NUM_COLS
				&& GetPiece(matrixColors[rowClick - 2][columnClick + 1].idPiece).color != piece.color)
			{
				markTile(rowClick - 2, columnClick + 1, true, true);
			}
		}
		else if (movement == Movement::Northeast)
		{
			int moveNortheast = NUM_COLS;

			if (piece.piece == Piece::King)
			{
				moveNortheast = 2;
			}

			for (size_t i = 1; i < moveNortheast; i++)
			{
				GameObject nextTile = GetPiece(matrixColors[rowClick + i][columnClick - i].idPiece);

				if (nextTile.color == piece.color)
				{
					break;
				}

				if (rowClick + i < NUM_ROWS && columnClick - i < NUM_ROWS)
				{
					markTile(rowClick + i, columnClick - i, true, true);
				}

				if (nextTile.color != piece.color && nextTile.color != NULL)
				{
					break;
				}
			}
		}
		else if (movement == Movement::Northwest)
		{
			int moveNorthwest = NUM_ROWS;

			if (piece.piece == Piece::King)
			{
				moveNorthwest = 2;
			}

			for (size_t i = 1; i < moveNorthwest; i++)
			{
				GameObject nextTile = GetPiece(matrixColors[rowClick + i][columnClick + i].idPiece);

				if (nextTile.color == piece.color)
				{
					break;
				}

				if (rowClick + i < NUM_ROWS && columnClick + i < NUM_COLS)
				{
					markTile(rowClick + i, columnClick + i, true, true);
				}

				if (nextTile.color != piece.color && nextTile.color != NULL)
				{
					break;
				}
			}
		}
		else if (movement == Movement::South)
		{
			int moveSouth = NUM_ROWS;

			if (piece.piece == Piece::Pawn)
			{
				moveSouth = piece.isFirstMove ? 2 : 1;

				// Regra para ataque do pi�o
				if (rowClick - 1 > 0
					&& columnClick + 1 < NUM_COLS
					&& matrixColors[rowClick - 1][columnClick + 1].idPiece > 0
					&& GetPiece(matrixColors[rowClick - 1][columnClick + 1].idPiece).color != piece.color)
				{
					markTile(rowClick - 1, columnClick + 1, true, true);
				}

				if (rowClick - 1 > 0
					&& columnClick - 1 >= 0
					&& matrixColors[rowClick - 1][columnClick - 1].idPiece > 0
					&& GetPiece(matrixColors[rowClick - 1][columnClick - 1].idPiece).color != piece.color)
				{
					markTile(rowClick - 1, columnClick - 1, true, true);
				}
			}

			if (piece.piece == Piece::King)
			{
				moveSouth = 1;
			}

			for (size_t i = 1; i <= moveSouth; i++)
			{
				if (rowClick - i < NUM_ROWS)
				{
					GameObject nextTile = GetPiece(matrixColors[rowClick - i][columnClick].idPiece);

					if (nextTile.color == piece.color)
					{
						break;
					}

					markTile(rowClick - i, columnClick, true, true);

					if (nextTile.color != piece.color && nextTile.color != NULL)
					{
						break;
					}
				}
			}
		}
		else if (movement == Movement::Southeast)
		{
			int moveSoutheast = rowClick;

			if (piece.piece == Piece::King)
			{
				moveSoutheast = 2;
			}

			for (size_t i = 1; i < moveSoutheast; i++)
			{
				GameObject nextTile = GetPiece(matrixColors[rowClick - i][columnClick - i].idPiece);

				if (nextTile.color == piece.color)
				{
					break;
				}

				if (rowClick - i >= 0 && columnClick - i <= NUM_COLS)
				{
					markTile(rowClick - i, columnClick - i, true, true);
				}

				if (nextTile.color != piece.color && nextTile.color != NULL)
				{
					break;
				}
			}
		}
		else if (movement == Movement::Southwest)
		{
			int moveSouthwest = NUM_ROWS;

			if (piece.piece == Piece::King)
			{
				moveSouthwest = 2;
			}

			for (size_t i = 1; i < moveSouthwest; i++)
			{
				GameObject nextTile = GetPiece(matrixColors[rowClick - i][columnClick + i].idPiece);

				if (nextTile.color == piece.color)
				{
					break;
				}

				if (rowClick - i >= 0 && columnClick + i < NUM_COLS)
				{
					markTile(rowClick - i, columnClick + i, true, true);
				}

				if (nextTile.color != piece.color && nextTile.color != NULL)
				{
					break;
				}
			}
		}
		else if (movement == Movement::West)
		{
			int moveWest = NUM_COLS;

			if (piece.piece == Piece::King)
			{
				moveWest = 1;
			}

			for (size_t i = 1; i <= moveWest; i++)
			{
				if (columnClick + i < NUM_ROWS)
				{
					GameObject nextTile = GetPiece(matrixColors[rowClick][columnClick + i].idPiece);

					if (nextTile.color == piece.color)
					{
						break;
					}

					markTile(rowClick, columnClick + i, true, true);

					if (nextTile.color != piece.color && nextTile.color != NULL)
					{
						break;
					}
				}
			}
		}
	}
}

void SetFirst(int id)
{
	for each (GameObject ws in whiteSprites)
	{

	}

	for each (GameObject bs in whiteSprites)
	{

	}
}

void MouseMap(double xPos, double yPos) {

	int rowClick, columnClick;
	DiamondClickCalculation(xPos, yPos, rowClick, columnClick);

	if (rowClick < 0 || columnClick < 0 || columnClick >= NUM_COLS || rowClick >= NUM_ROWS)
	{
		return;
	}

	Tile tile = matrixColors[rowClick][columnClick];

	if (TestPointCollision(tile.Ax, tile.Ay, tile.Bx, tile.By, tile.Cx, tile.Cy, xPos, yPos))
	{
		// Verifica se for desmarcado, para poder desmarcar as poss�veis jogadas
		if (matrixColors[rowClick][columnClick].canPlay)
		{
			GameObject& piece = GetPiece(matrixColors[lastSelectedRow][lastSelectedColumn].idPiece, true);

			if (!tile.isSelected)
			{
				canPlayBlack = canPlayWhite;
				canPlayWhite = !canPlayBlack;
			}

			piece.translate = true;

			matrixColors[rowClick][columnClick].isSelected = false;

			for each (pair<int, int> pos in selectedPositions)
			{
				markTile(pos.first, pos.second, false, false);
			}

			selectedPositions.clear();

			if (matrixColors[rowClick][columnClick].idPiece != matrixColors[lastSelectedRow][lastSelectedColumn].idPiece)
			{
				SetFirst(piece.id);

				//piece.setIsFirstMove(false);
				matrixColors[rowClick][columnClick].idPiece = matrixColors[lastSelectedRow][lastSelectedColumn].idPiece;
				matrixColors[lastSelectedRow][lastSelectedColumn].idPiece = 0;
			}

		}
		else if (selectedPositions.empty() && tile.idPiece > 0) // J� ter� adicionado o tile selecionado, ent�o n�o estar� vazio mais, apenas ter� o tile selecionado
		{
			GameObject piece = GetPiece(matrixColors[rowClick][columnClick].idPiece, true);

			bool playBlack = piece.color == Color::Black && canPlayBlack;
			bool playWhite = piece.color == Color::White && canPlayWhite;

			if (playBlack || playWhite)
			{
				lastSelectedColumn = columnClick;
				lastSelectedRow = rowClick;

				matrixColors[rowClick][columnClick].isSelected = !matrixColors[rowClick][columnClick].isSelected;

				markTile(rowClick, columnClick, true, true);

				markMovements(rowClick, columnClick, piece);
			}
		}
	}
}

// M�todo utilizado para callback de click
void SelectPosition(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double xpos, ypos;

		glfwGetCursorPos(window, &xpos, &ypos);

		MouseMap(xpos, ypos);
	}
}
#pragma endregion

int main() {
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Chess developed by Matheus Moraes and Vitor Marco", NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();


	const char* map_vertex_shader =
		"#version 410\n"
		"layout(location = 0) in vec2 aPos;"
		"uniform mat4 matrix_OBJ;"
		"uniform mat4 proj;"
		"void main() {"
		"	gl_Position = proj * matrix_OBJ * vec4(aPos, 0.5, 1.0);"
		"}";

	const char* map_fragment_shader =
		"#version 410\n"
		"uniform vec3 colorValues;"
		"out vec4 frag_color;"
		"void main() {"
		"	frag_color = vec4(colorValues, 1.0);"
		"}";

	//Vertex da textura
	const char* textureVertex_shader =
		"#version 410\n"
		"layout (location = 0) in vec2 vertex_position;"
		"layout (location = 1) in vec2 texture_mapping;"
		"uniform mat4 matrix;"
		"uniform mat4 proj;"
		"out vec2 texture_coords;"
		"uniform float layer_z;"
		"void main() {"
		"	texture_coords = texture_mapping;"
		"   gl_Position = proj * matrix * vec4 (vertex_position, layer_z, 1.0);"
		"}";

	//Fragment da textura
	const char* textureFragment_shader =
		"#version 410\n"
		"in vec2 texture_coords;"
		"uniform sampler2D sprite;"
		"uniform float offsetx;"
		"uniform float offsety;"
		"out vec4 frag_color;"
		"void main () {"
		" vec4 texel = texture(sprite, vec2(texture_coords.x + offsetx, texture_coords.y + offsety));"
		" if (texel.a < 0.5) "
		"	discard; "
		" frag_color = texel;"
		"}";

	CreateMatrixColors();
	ConfigSprites();

	glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);


	int mapShader_programme = ConnectVertex(map_vertex_shader, map_fragment_shader);
	textureShader_programme = ConnectVertex(textureVertex_shader, textureFragment_shader);

	// V�rtice do diamondMap
	GLfloat mapVertices[] =
	{
		// positions
		TILE_WIDTH / 2.0f, 0.0f, 0.0f,			// TOP
		TILE_WIDTH, TILE_HEIGHT / 2.0f, 0.0f,   // RIGHT
		TILE_WIDTH / 2.0f, TILE_HEIGHT, 0.0f,   // BOTTOM
		0.0f, TILE_HEIGHT / 2.0f, 0.0f			// LEFT
	};

	unsigned int mapIndices[] =
	{
		0, 1, 2,   // first triangle
		0, 3, 2    // second triangle
	};

	// V�rtice do diamondMap
	GLfloat textureVertices[] =
	{
		// positions
		TILE_WIDTH / 2.0f, 0.0f, 0.0f,			0.1f, 0.0f,		// TOP
		TILE_WIDTH, TILE_HEIGHT / 2.0f, 0.0f,	0.1f, 0.1f,		// RIGHT
		TILE_WIDTH / 2.0f, TILE_HEIGHT, 0.0f,	0.0f, 0.1f,		// BOTTOM
		0.0f, TILE_HEIGHT / 2.0f, 0.0f,			0.0f, 0.0f 		// LEFT
	};

	unsigned int textureIndices[] =
	{
		0, 1, 2,   // first triangle
		0, 3, 2    // second triangle
	};

	GLuint mapVBO, mapVAO, mapEBO;
	glGenBuffers(1, &mapEBO);
	glGenBuffers(1, &mapVBO);

	glGenVertexArrays(1, &mapVAO);
	glBindVertexArray(mapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mapVBO);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), mapVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mapIndices), mapIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/////////////////////////////////////////////////////////////////

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureVertices), textureVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(textureIndices), textureIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Variaveis
	float coluna = 0.0f;
	float linha = 0.0f;

	int IDImagem = 0;	//1� numero = linha, 2� numero = coluna, come�ando do canto superior esquerdo do tileset
	float col = 0.0f;
	float row = 0.0f;



	// esta para quando clicar com o mouse
	glfwSetMouseButtonCallback(window, SelectPosition);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Encerra o game
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		// glm projecao
		glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);


		int screenWidth, screenHeight;
		glfwGetWindowSize(window, &screenWidth, &screenHeight);
		glViewport(0, 0, screenWidth, screenHeight);

		// Desenha as sprites
		glUseProgram(textureShader_programme);

		glUniformMatrix4fv(glGetUniformLocation(textureShader_programme, "proj"), 1, GL_FALSE, glm::value_ptr(proj));

		// Matrix de movimento para sprits inimigas
		glm::mat4 matrix_translaction = glm::mat4(1.0f);
		unsigned int transformloc = glGetUniformLocation(textureShader_programme, "matrix");
		glUniformMatrix4fv(transformloc, 1, GL_FALSE, glm::value_ptr(matrix));

		//for each (GameObject bs in blackSprites)
		//{
		//	DefineOffsetAndRender(textureShader_programme, 0.0f, 0.0f, 0.51f, bs.vao, bs.tid, matrix, bs, transformloc);
		//}

		for each (GameObject ws in whiteSprites)
		{
		DefineOffsetAndRender(textureShader_programme, 0.0f, 0.0f, 0.51f, whiteSprites[0].vao, whiteSprites[0].tid, matrix, whiteSprites[0], transformloc);
		}

		// Desenha o diamond
		glUseProgram(mapShader_programme);
		glUniformMatrix4fv(glGetUniformLocation(mapShader_programme, "proj"), 1, GL_FALSE, glm::value_ptr(projection));


		//Define VAO atual
		glBindVertexArray(mapVAO);

		RenderDiamondMap(matrix, mapShader_programme);

		glfwSwapBuffers(window);
	}

	// encerra contexto GL e outros recursos da GLFW
	glfwTerminate();
	return 0;
}