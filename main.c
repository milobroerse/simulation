#include "main.h"

unsigned int worldHeight;
unsigned int worldWidth;
unsigned int worldBorder;

int main(int argc, char **argv){

	if (argc < 5 || argc > 5) {
		printf("USAGE:\nmain.exe <HEIGHT> <WIDTH> <BORDER> <GRIDSIZE>\n");
		return 1;
	}

	worldHeight = atoi(argv[1]);
	worldWidth = atoi(argv[2]);
	worldBorder = atoi(argv[3]);
	unsigned int gridsize = atoi(argv[4]);

	// allocate world
	Cell **world = malloc(sizeof(Cell*) * worldHeight);
	for (unsigned int p = 0; p < worldHeight; p++) {
		world[p] = malloc(sizeof(Cell) * worldWidth);
	}

	// allocate grid
	unsigned int **grid = malloc(sizeof(unsigned int*) * gridsize);
	for (unsigned int p = 0; p < gridsize; p++) {
		grid[p] = malloc(sizeof(unsigned int) * gridsize);
	}

	fillGrid(grid, gridsize);
	fillWorld(world, 23, grid, gridsize);
	showWorld(world, 0);
	update(world);
	showWorld(world, 0);


	// free grid
	for (unsigned int p = 0; p < gridsize; p++) {
		free(grid[p]);
		grid[p] = NULL;
	}
	free(grid);
	grid = NULL;

	// free world
	for (int p = 0; p < worldHeight; p++) {
		free(world[p]);
		world[p] = NULL;
	}
	free(world);
	world = NULL;
	return 0;
}

// TBO
void flow(Cell **world, unsigned int j, unsigned int i){
	unsigned int y = j + 1;

	Cell middle = world[y][i];
	Cell left;
	if ((i - 1) < 0){
		left = middle;
	} else {
		left = world[y][i - 1];
	}
	Cell right;
	if ((i + 1) == worldWidth){
		right = middle;
	} else {
		right = world[y][i + 1];
	}

	Point p = flowDirection(left, middle, right);
	world[j][i].waterOccupied -= 1;
	world[j + p.y][i + p.x].waterOccupied += 1;
}

Point choosePoint(unsigned int l, unsigned int m, unsigned int r) {
	Point p = { .x = 0, .y = 0 };
	if (l + m + r) {
		p.y = 1;
		unsigned int max = m;
		if (l > max) {
			max = l;
			p.x = -1;
		}
		if (r > max) {
			p.x = 1;
		}
	}
	return p;
}

Point flowDirection(Cell left, Cell middle, Cell right) {
	unsigned int leftAccessibility = capacityAccessibility(left.waterOccupied, left.waterCapacity, middle.waterOccupied,  right.waterOccupied);
	unsigned int middleAccessibility = capacityAccessibility(middle.waterOccupied, middle.waterCapacity, left.waterOccupied, right.waterOccupied);
	unsigned int rightAccessibility = capacityAccessibility(right.waterOccupied, right.waterCapacity, left.waterOccupied, middle.waterOccupied);
	return choosePoint(leftAccessibility, middleAccessibility, rightAccessibility);
}

unsigned int capacityAccessibility(unsigned int occupied, unsigned int capacity, unsigned int o1, unsigned int o2){
	unsigned int diff = capacity - occupied;
	return (diff) + (diff * (occupied < o1)) + (diff * (occupied < o2));
}

void update(Cell **world){
	for (int j = worldHeight - 1; j >= 0; j--) {
		unsigned int y = j + 1;
		for (int i = worldWidth - 1; i >= 0; i--) {
			if (y < worldHeight) {
				Cell currentCell = world[j][i];
				if (currentCell.waterCapacity && currentCell.waterOccupied) {
					flow(world, j, i);
				}
			}
		}
	}
}

// TBO
void fillGrid(unsigned int **grid, unsigned int n) {
	unsigned int stop = n;
	for (unsigned int j = 0; j < ((n - 1) / 2) + 1; j++) {
		if (j != 0){
			for (unsigned int p = 0; p < n; p++){
				grid[j][p] = grid[j-1][p];
			}
		}
		for (unsigned int i = j; i < stop; i++) {
			grid[j][i] += 1;
		}
		stop -= 1;
		for (unsigned int p = 0; p < n; p++) {
			grid[stop][p] = grid[j][p];
		}
	}
}

// TBO
void fillWorld(Cell **world, unsigned int seed, unsigned int **grid, unsigned int n){
	srand(seed);

	unsigned int points = rand() % 5 + 3;
	unsigned int radius = (n - 1) / 2;

	for (int j = 0; j < worldHeight; j++) {
		for (int i = 0; i < worldWidth; i++) {
			Cell c;
			c.type = 'G';
			c.waterOccupied = 0;
			c.waterCapacity = rand() % 2; // add noise
			c.nutrition = 0;
			c.sun = 0;
			if(j < worldBorder){
				c.type = 'A';
				c.waterCapacity = 0;
			}
			world[j][i] = c;
		}
	}

	for (unsigned int p = 0; p < points; p++) {
		unsigned int randX = rand() % worldWidth;
		unsigned int randY = (rand() % (worldHeight - worldBorder)) + worldBorder;
		for (unsigned int j = 0; j < n; j++) {
			int l = j - radius;
			for (unsigned int i = 0; i < n; i++) {
				int k = i - radius;
				int x = randX - k;
				int y = randY - l;
				if ((x >= 0 && x < worldWidth) && (y >= worldBorder && y < worldHeight)){
					Cell c = world[y][x];
					c.nutrition += grid[j][i];
					c.waterOccupied += 1;
					c.waterCapacity += grid[j][i];
					c.type = 'T';
					world[y][x] = c;
				}
			}
		}
	}
}

// TBO
void showWorld(Cell **world, unsigned int toCheck){
	for (unsigned int x = 0; x < worldWidth + 6; x++){
		printf("_");
	}
	printf("\n");
	for (unsigned int y = 0; y < worldHeight; y++) {
		for (unsigned int x = 0; x < worldWidth; x++) {
			if (toCheck == 0){
				printf("%c", typeToDisplay(world[y][x].type));
			} else if (toCheck == 1) {
				printf("%d", world[y][x].waterOccupied);
			} else if (toCheck == 2) {
				printf("%d", world[y][x].nutrition);
			} else if (toCheck == 3) {
				printf("%d", world[y][x].waterCapacity);
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
