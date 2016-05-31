#ifndef TETRIS_H
#define TETRIS_H
//for rand
#include <stdlib.h>

void initrand() {
	srand(0);
}

char color() {
	unsigned char r = rand() % 3;
	if (r == 0) {
		return 'r';
	} else if (r == 1) {
		return 'g';
	} else {
		return 'b';
	}
}

struct block {
	//row, col, color
	//pivot
	char pivot[3];
	//chunk
	char chunk1[3];
	char chunk2[3];
	char chunk3[3];
	char check;
};
//{
//{'0','1','2','3'}
//{'0','1','2','3'}
//{'0','1','2','3'}
//{'0','1','2','2'}
//}

struct block initcolor(char pos1[], char pos2[], char pos3[], char pos4[]) {
	struct block b;
	b.pivot[0] = pos1[0];
	b.pivot[1] = pos1[1];
	b.pivot[2] = color();
	
	b.chunk1[0] = pos2[0];
	b.chunk1[1] = pos2[1];
	b.chunk1[2] = color();

	b.chunk2[0] = pos3[0];
	b.chunk2[1] = pos3[1];
	b.chunk2[2] = color();

	b.chunk3[0] = pos4[0];
	b.chunk3[1] = pos4[1];
	b.chunk3[2] = color();

	return b;
}

struct block initblock(char n) {
	char r = rand() % n;
	char pos1[2];
	char pos2[2];
	char pos3[2];
	char pos4[2];
	if (r == 0) {
		//L shape
		pos1[0] = 0; pos1[1] = 3;
		pos2[0] = 0; pos2[1] = 4;
		pos3[0] = 1; pos3[1] = 3;
		pos4[0] = 2; pos4[1] = 3;
	} else if (r == 1) {
		// I shape
		pos1[0] = 1; pos1[1] = 3;
		pos2[0] = 0; pos2[1] = 3;
		pos3[0] = 2; pos3[1] = 3;
		pos4[0] = 3; pos4[1] = 3;
	} else if (r == 2) {
		// square
		pos1[0] = 1; pos1[1] = 3;
		pos2[0] = 0; pos2[1] = 3;
		pos3[0] = 1; pos3[1] = 4;
		pos4[0] = 0; pos4[1] = 4;
	}
	struct block b = initcolor(pos1, pos2, pos3, pos4);
	return b;
} 

extern char board[9][8];
extern char output[9][8];

//displays the block on 8x8
void display(struct block b) {
	updateout();
	output[b.pivot[0]][b.pivot[1]] = b.pivot[2];
	output[b.chunk1[0]][b.chunk1[1]] = b.chunk1[2];
	output[b.chunk2[0]][b.chunk2[1]] = b.chunk2[2];
	output[b.chunk3[0]][b.chunk3[1]] = b.chunk3[2];
}

//rotates the piece clockwise
//does nothing if does not work
struct block rotate (struct block b) {
	//make copy
	struct block temp = b;
	
	//calculate relative pos of chunks
	b.chunk1[0] = b.chunk1[0] - b.pivot[0];
	b.chunk1[1] = b.chunk1[1] - b.pivot[1];
	
	b.chunk2[0] = b.chunk2[0] - b.pivot[0];
	b.chunk2[1] = b.chunk2[1] - b.pivot[1];
	
	b.chunk3[0] = b.chunk3[0] - b.pivot[0];
	b.chunk3[1] = b.chunk3[1] - b.pivot[1];

	//calculate rotated pos
	char temp0, temp1;
	temp0 = -1 * b.chunk1[1];
	temp1 = b.chunk1[0];
	b.chunk1[0] = temp0;
	b.chunk1[1] = temp1;
	
	temp0 = -1 * b.chunk2[1];
	temp1 = b.chunk2[0];
	b.chunk2[0] = temp0;
	b.chunk2[1] = temp1;

	temp0 = -1 * b.chunk3[1];
	temp1 = b.chunk3[0];
	b.chunk3[0] = temp0;
	b.chunk3[1] = temp1;

	//calculate actual position
	b.chunk1[0] = b.pivot[0] + b.chunk1[0];
	b.chunk1[1] = b.pivot[1] + b.chunk1[1]; 
	
	b.chunk2[0] = b.pivot[0] + b.chunk2[0];
	b.chunk2[1] = b.pivot[1] + b.chunk2[1];

	b.chunk3[0] = b.pivot[0] + b.chunk3[0];
	b.chunk3[1] = b.pivot[1] + b.chunk3[1];
	
	//check if new pos works
	if (checkboard(b) == -1) 
		return temp;
	
	return b;
}

//drops a block down one row
//returns same block and check to -1 if it cant
//check is 1 if it can
struct block drop (struct block b) {
	//make a copy
	struct block temp = b;
	b.pivot[0] = b.pivot[0] + 1;
	b.chunk1[0] = b.chunk1[0] + 1;
	b.chunk2[0] = b.chunk2[0] + 1;
	b.chunk3[0] = b.chunk3[0] + 1;
	
	//check if valid
	if (checkboard(b) == -1) {
		temp.check = -1;
		return temp;
	}
	b.check = 1;
	//return piece
	return b;
}

//moves block left if it cant does nothing
struct block moveleft (struct block b) {
	//make a copy
	struct block temp = b;
	b.pivot[1] = b.pivot[1] - 1;
	b.chunk1[1] = b.chunk1[1] - 1;
	b.chunk2[1] = b.chunk2[1] - 1;
	b.chunk3[1] = b.chunk3[1] - 1;
		
	//check if valid
	if (checkboard(b) == -1) {
		return temp;
	}
		
	//return piece
	return b;
}

//moves block right if it cant does nothing
struct block moveright (struct block b) {
	//make a copy
	struct block temp = b;
	b.pivot[1] = b.pivot[1] + 1;
	b.chunk1[1] = b.chunk1[1] + 1;
	b.chunk2[1] = b.chunk2[1] + 1;
	b.chunk3[1] = b.chunk3[1] + 1;
	
	//check if valid
	if (checkboard(b) == -1) {
		return temp;
	}
	
	//return piece
	return b;
}

//combines the board and a block
void combine (struct block b) {
	board[b.pivot[0]][b.pivot[1]] = b.pivot[2];
	board[b.chunk1[0]][b.chunk1[1]] = b.chunk1[2];
	board[b.chunk2[0]][b.chunk2[1]] = b.chunk2[2];
	board[b.chunk3[0]][b.chunk3[1]] = b.chunk3[2];

	
}

//checks the board and the piece
//return -1 if anything is wrong
//return 1 if its okay
int checkboard (struct block b) {
	//check if piece clash with part of the board
	if (board[b.pivot[0]][b.pivot[1]] != 'x')
		return -1;
	else if (board[b.chunk1[0]][b.chunk1[1]] != 'x') 
		return -1;
	else if (board[b.chunk2[0]][b.chunk2[1]] != 'x') 
		return -1;
	else if (board[b.chunk3[0]][b.chunk3[1]] != 'x')
		return -1;
	
	//check if piece goes out of bounds
	if (b.pivot[0] >= 9 || b.pivot[1] >= 8)
		return -1;
	else if (b.pivot[0] < 0 || b.pivot[1] < 0)
		return -1;
	else if (b.chunk1[0] >= 9 || b.chunk1[1] >= 8)
		return -1;
	else if (b.chunk1[0] < 0 || b.chunk1[1] < 0)
		return -1;
	else if (b.chunk2[0] >= 9 || b.chunk2[1] >= 8)
		return -1;
	else if (b.chunk2[0] < 0 || b.chunk2[1] < 0)
		return -1;
	else if (b.chunk3[0] >= 9 || b.chunk3[1] >= 8)
		return -1;
	else if (b.chunk3[0] < 0 || b.chunk3[1] < 0)
		return -1;
		
	return 1;
}

#endif TETRIS_H
