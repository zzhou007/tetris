struct block {
	//pivot
	char pivot[3];
	//chunk
	char chunk1[3];
	char chunk2[3];
	char chunk3[3];
};
//{
//{'0','1','2','3'}
//{'0','1','2','3'}
//{'0','1','2','3'}
//{'0','1','2','2'}
//}

extern char board[9][8];
//rotates the piece clockwise
//sets check to -1 if not possible
struct block rotate (struct block b) {
	
}

//drops a block down one row
//returns sets check to -1 if it cant
struct block drop (struct block b) {
	
}

//combines the board and a block
void combine (struct block b) {
	
}

//checks the board and the piece
int checkboard (struct block b) {
	
}