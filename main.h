#include <stdio.h>
#include <stdlib.h>

#define DIAMETER 9

typedef	struct Cell {
	char type;

	unsigned int waterOccupied;
	unsigned int waterCapacity;
	unsigned int nutrition;
	unsigned int sun;
} Cell;

typedef struct Point {
	int x;
	int y;
} Point;

void 					flow(Cell **world, unsigned int j, unsigned int i);
Point 				choosePoint(unsigned int l, unsigned int m, unsigned int r);
Point 				flowDirection(Cell left, Cell middle, Cell right);
unsigned int 	capacityAccessibility(unsigned int occupied, unsigned int capacity, unsigned int o1, unsigned int o2);
void 					update(Cell **world);
void 					fillGrid(unsigned int grid[DIAMETER][DIAMETER], unsigned int n);
void 					fillWorld(Cell **world, unsigned int seed, unsigned int grid[DIAMETER][DIAMETER], unsigned int n);
void 					showWorld(Cell **world, unsigned int toCheck);
char 					typeToDisplay(char type);
