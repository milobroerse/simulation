#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef	struct Cell {
	char type;

	uint8_t waterOccupied;
	uint8_t waterCapacity;
	uint8_t nutrition;
	uint8_t sun;
} Cell;

typedef struct Point {
	int16_t x;
	int16_t y;
} Point;

void 					rain();
void 					flow(Cell **world, Point currentPoint, uint16_t worldWidth);
Point 				choosePoint(uint8_t l, uint8_t m, uint8_t r);
Point 				flowDirection(Cell left, Cell middle, Cell right);
uint8_t		 		capacityAccessibility(uint8_t occupied, uint8_t capacity, uint8_t o1, uint8_t o2);
void 					update(Cell **world, uint16_t worldHeight, uint16_t worldWidth);
void 					fillGrid(uint16_t **grid, uint16_t gridSize);
void 					fillWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t worldBorder, uint16_t seed, uint16_t **grid, uint16_t gridSize);
void 					showWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t toCheck);
char 					typeToDisplay(char type);
