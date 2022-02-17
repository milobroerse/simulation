#include <stdio.h>
#include <stdlib.h>

#define HEIGHT 64
#define WIDTH 32
#define BORDER 40
#define DIAMETER 9

typedef	struct Cell {
	char type;
	unsigned char water;
	unsigned char nutrition;
	unsigned char sun;
} Cell;

void fillGrid(unsigned int grid[DIAMETER][DIAMETER], unsigned int n);
void fillWorld(Cell world[HEIGHT][WIDTH], unsigned int seed, unsigned int grid[DIAMETER][DIAMETER], unsigned int n);
void showWorld(Cell world[HEIGHT][WIDTH], unsigned int visual);
char typeToDisplay(char type);

int main(){
	Cell world[HEIGHT][WIDTH];
	unsigned int nutritionGrid[DIAMETER][DIAMETER] = {{0}};
	fillGrid(nutritionGrid, DIAMETER);
	fillWorld(world, 14, nutritionGrid, DIAMETER);
	showWorld(world, 1);
	return 0;
}

void fillGrid(unsigned int grid[DIAMETER][DIAMETER], unsigned int n){
	int stop = n;
	for (int j = 0; j < ((n - 1) / 2) + 1; j++) {
		if (j != 0){
			for (int p = 0; p < n; p++){
				grid[j][p] = grid[j-1][p];
			}
		}
		for (int i = j; i < stop; i++){
			grid[j][i] += 1;
		}
		stop -= 1;
		for (int p = 0; p < n; p++){
			grid[stop][p] = grid[j][p];
		}
	}
}
void fillWorld(Cell world[HEIGHT][WIDTH], unsigned int seed, unsigned int grid[DIAMETER][DIAMETER], unsigned int n){
	srand(seed);
 	unsigned int nutritionPoints = rand() % 5 + 3;
	unsigned int waterPoints = rand() % 5 + 3;
	unsigned int radius = (n - 1) / 2;
	for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			Cell c;
			c.type = 'G';
			if(j < BORDER){
				c.type = 'A';
			}
			c.water = 0;
			c.nutrition = 0;
			c.sun = 0;
			world[j][i] = c;
		}
	}

	for (int p = 0; p < nutritionPoints; p++){
		int randX = rand() % WIDTH;
		int randY = (rand() % (HEIGHT - BORDER)) + BORDER;
		for (int j = 0; j < n; j++) {
			int l = j - radius;
			for (int i = 0; i < n; i++) {
				int k = i - radius;
				int x = randX - k;
				int y = randY - l;
				if ((x >= 0 && x < WIDTH) && (y >= BORDER && y < HEIGHT)){
					Cell c = world[y][x];
					c.nutrition += grid[j][i];
					c.water += grid[j][i];
					c.type = 'T';
					world[y][x] = c;
				}
			}
		}
	}
}
void showWorld(Cell world[HEIGHT][WIDTH], unsigned int visual){
	for (int x = 0; x < WIDTH + 6; x++){
		printf("_");
	}
	printf("\n");
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (visual){
				printf("%c", typeToDisplay(world[y][x].type));
			} else {
				printf("%d", world[y][x].nutrition);
			}
		}
		printf("| %2d |", y);
		printf("\n");
	}
}
char typeToDisplay(char type){
	char display;
	switch (type) {
		case 'G':
			display = '_';
			break;
		case 'A':
			display = ' ';
			break;
		case 'T':
			display = '?';
			break;
	}
	return display;
}
