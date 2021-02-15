#include "Includes.h"
// S_SU == DIR_U, S_SR == DIR_R, etc...
typedef enum{S_SU, S_SR, S_SD, S_SL, S_SF, S_A, S_E}Square;

typedef struct{
	uint scale;
	Length len;
	Square **grid;
}Board;

typedef struct{
	Coord head;
	Direction dir;
}Snake;

void drawBoard(const Board board)
{
	for(uint y = 0; y < board.len.y; y++){
		for(uint x = 0; x < board.len.x; x++){
			switch(board.grid[x][y]){
			case S_SU ... S_SF:
				setColor(GREEN);
				break;
			case S_A:
				setColor(RED);
				break;
			default:
				setColor(GREY);
				drawSquareCoord(
					coordMul((Coord){x,y},board.scale),
					board.scale
				);
				continue;
			}
			fillSquareCoord(
				coordMul((Coord){x,y},board.scale),
				board.scale
			);
		}
	}
}

Direction getDir(const Direction currentDir, bool *dirKey)
{
	Direction ret = currentDir;
	for(uint i = 0; i < 4; i++){
		if(dirKey[i] &&
		!dirKey[dirINV(i)] &&
		i != dirINV(currentDir))
			ret = i;
	}
	return ret;
}

bool isSnake(const Square square)
{
	return square >= S_SU && square <= S_SF;
}

void placeApple(Board board)
{
	Coord pos;
	do{
		pos = (Coord){rand()%board.len.x, rand()%board.len.y};
	}while(isSnake(board.grid[pos.x][pos.y]));
	board.grid[pos.x][pos.y] = S_A;
}

Coord nextSnake(const Coord pos, const Board board)
{
	if(board.grid[pos.x][pos.y] < S_SU || board.grid[pos.x][pos.y] > S_SL)
		return pos;
	return coordShift(pos, board.grid[pos.x][pos.y], 1);
}

void loose(Board board)
{
	for(uint x = 0; x < board.len.x; x++)
		free(board.grid[x]);
	free(board.grid);
	printf("You lost... gtfo\n");
	exit(0);
}

Coord moveSnake(const Snake snake, Board board)
{
	const Coord ret = coordShift(snake.head, snake.dir, 1);
	if(!inBound(ret.x, 0, board.len.x) ||
	!inBound(ret.y, 0, board.len.y) ||
	isSnake(board.grid[ret.x][ret.y]))
		loose(board);
	const bool isApple = board.grid[ret.x][ret.y] == S_A;
	if(!isApple){
		Coord pos;
		Coord next = snake.head;
		do{
			pos = next;
			next = nextSnake(pos, board);
		}while(board.grid[next.x][next.y] != S_SF);
		board.grid[next.x][next.y] = S_E;
		board.grid[pos.x][pos.y] = S_SF;
	}
	board.grid[ret.x][ret.y] = dirINV(snake.dir);
	if(isApple)
		placeApple(board);
	return ret;
}

void readDirKeys(bool *dirKey)
{
	dirKey[0] |= keyPressed(SDL_SCANCODE_W)||keyPressed(SDL_SCANCODE_UP);
	dirKey[1] |= keyPressed(SDL_SCANCODE_D)||keyPressed(SDL_SCANCODE_RIGHT);
	dirKey[2] |= keyPressed(SDL_SCANCODE_S)||keyPressed(SDL_SCANCODE_DOWN);
	dirKey[3] |= keyPressed(SDL_SCANCODE_A)||keyPressed(SDL_SCANCODE_LEFT);
}

int main(int argc, char const *argv[])
{
	const Length window = {800, 600};
	init(window);

	Board board = {
		.scale = 10,
		.len = coordDiv(window, board.scale),
		.grid = malloc(sizeof(Square*)*board.len.x)
	};

	for(uint x = 0; x < board.len.x; x++){
		board.grid[x] = malloc(sizeof(Square)*board.len.y);
		memset(board.grid[x], S_E, sizeof(Square)*board.len.y);
	}

	Snake snake = {
		.head = coordDiv(board.len, 2),
		.dir = DIR_L
	};

	board.grid[snake.head.x][snake.head.y] = S_SR;
	board.grid[snake.head.x][snake.head.y] = S_SF;
	placeApple(board);

	uint frameCount = 0;
	bool dirKey[4] = {0};
	while(1){
		Ticks frameStart = getTicks();
		clear();

		readDirKeys(dirKey);
		if(frameCount++ > FPS/8){
			frameCount = 0;
			snake.dir = getDir(snake.dir, dirKey);
			snake.head = moveSnake(snake, board);
			memset(dirKey, 0, sizeof(bool)*4);
		}
		drawBoard(board);

		draw();
		events(frameStart + TPF);
	}
	return 0;
}
