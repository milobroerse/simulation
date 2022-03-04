#include "main.h"

int main(int argc, char **argv){

	if (argc < 5 || argc > 5) {
		printf("USAGE:\nmain.exe <HEIGHT> <WIDTH> <BORDER> <GRIDSIZE>\n");
		return 1;
	}

	uint16_t worldHeight = atoi(argv[1]);
	uint16_t worldWidth = atoi(argv[2]);
	uint16_t worldBorder = atoi(argv[3]);
	uint16_t gridSize = atoi(argv[4]);

	// allocate world
	Cell **world = malloc(sizeof(Cell*) * worldHeight);
	for (uint16_t p = 0; p < worldHeight; p++) {
		world[p] = malloc(sizeof(Cell) * worldWidth);
	}

	// allocate grid
	uint16_t **grid = malloc(sizeof(uint16_t*) * gridSize);
	for (uint16_t p = 0; p < gridSize; p++) {
		grid[p] = malloc(sizeof(uint16_t) * gridSize);
	}

	fillGrid(grid, gridSize);
	fillWorld(world, worldHeight, worldWidth, worldBorder, 23, grid, gridSize);
	update(world, worldHeight, worldWidth);
	showWorld(world, worldHeight, worldWidth, 0);


	// free grid
	for (uint16_t p = 0; p < gridSize; p++) {
		free(grid[p]);
	}
	free(grid);

	// free world
	for (uint16_t p = 0; p < worldHeight; p++) {
		free(world[p]);
	}
	free(world);
	return 0;
}

// TBO
void flow(Cell **world, Point currentPoint, uint16_t worldWidth){
	int16_t i = currentPoint.x;
	int16_t j = currentPoint.y;
	int16_t y = j + 1;

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

Point choosePoint(uint8_t l, uint8_t m, uint8_t r) {
	Point p = { .x = 0, .y = 0 };
	if (l + m + r) {
		p.y = 1;
		uint8_t max = m;
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
	uint8_t leftAccessibility = capacityAccessibility(left.waterOccupied, left.waterCapacity, middle.waterOccupied,  right.waterOccupied);
	uint8_t middleAccessibility = capacityAccessibility(middle.waterOccupied, middle.waterCapacity, left.waterOccupied, right.waterOccupied);
	uint8_t rightAccessibility = capacityAccessibility(right.waterOccupied, right.waterCapacity, left.waterOccupied, middle.waterOccupied);
	return choosePoint(leftAccessibility, middleAccessibility, rightAccessibility);
}

uint8_t capacityAccessibility(uint8_t occupied, uint8_t capacity, uint8_t o1, uint8_t o2){
	uint8_t diff = capacity - occupied;
	return (diff) + (diff * (occupied < o1)) + (diff * (occupied < o2));
}

void update(Cell **world, uint16_t worldHeight, uint16_t worldWidth){
	for (int16_t j = worldHeight - 1; j >= 0; j--) {
		int16_t y = j + 1;
		for (int16_t i = worldWidth - 1; i >= 0; i--) {
			if (y < worldHeight) {
				Cell currentCell = world[j][i];
				if (currentCell.waterCapacity && currentCell.waterOccupied) {
					Point currentPoint = { .x = i, .y = j };
					flow(world, currentPoint, worldWidth);
				}
			}
		}
	}
}

// TBO
void fillGrid(uint16_t **grid, uint16_t gridSize) {
	uint16_t stop = gridSize;
	uint16_t rad = ((gridSize - 1) / 2) + 1;
	for (uint16_t j = 0; j < rad; j++) {
		if (j != 0){
			for (uint16_t p = 0; p < gridSize; p++){
				grid[j][p] = grid[j-1][p];
			}
		}
		for (uint16_t i = j; i < stop; i++) {
			grid[j][i] += 1;
		}
		stop -= 1;
		for (uint16_t p = 0; p < gridSize; p++) {
			grid[stop][p] = grid[j][p];
		}
	}
}

// TBO
void fillWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t worldBorder, uint16_t seed, uint16_t **grid, uint16_t gridSize){
	srand(seed);

	uint16_t points = rand() % 5 + 3;
	uint16_t radius = (gridSize - 1) / 2;

	for (uint16_t j = 0; j < worldHeight; j++) {
		for (uint16_t i = 0; i < worldWidth; i++) {
			Cell c;
			c.type = 'G';
			c.waterOccupied = 0;
			c.waterCapacity = rand() % 2; // add noise
			if (c.waterCapacity) {
				c.type = 'T';
			}
			c.nutrition = 0;
			c.sun = 0;
			if(j < worldBorder){
				c.type = 'A';
				c.waterCapacity = 0;
			}
			world[j][i] = c;
		}
	}

	for (uint16_t p = 0; p < points; p++) {
		uint16_t randX = rand() % worldWidth;
		uint16_t randY = (rand() % (worldHeight - worldBorder)) + worldBorder;
		for (uint16_t j = 0; j < gridSize; j++) {
			int16_t l = j - radius;
			for (uint16_t i = 0; i < gridSize; i++) {
				int16_t k = i - radius;
				int16_t x = randX - k;
				int16_t y = randY - l;
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
void showWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t toCheck){
	for (uint16_t x = 0; x < worldWidth + 6; x++) {
		printf("_");
	}
	printf("\n");
	for (uint16_t y = 0; y < worldHeight; y++) {
		for (uint16_t x = 0; x < worldWidth; x++) {
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
			display = '#';
			break;
	}
	return display;
}
