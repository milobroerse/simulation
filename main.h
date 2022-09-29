#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

typedef	struct Cell {
	char type;

	uint8_t waterOccupied;
	uint8_t waterCapacity;
	// uint8_t nutrition;
	// uint8_t sun;
} Cell;

typedef struct Point {
	int16_t x;
	int16_t y;
} Point;

LRESULT 			CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void 					setPixelColor(int x, int y, uint32_t color, void *bitmapMemory, int bitmapWidth);
void 					flow(Cell **world, Point currentPoint, uint16_t worldWidth);
Point 				choosePoint(uint8_t l, uint8_t m, uint8_t r);
Point 				flowDirection(Cell left, Cell middle, Cell right);
uint8_t		 		capacityAccessibility(uint8_t occupied, uint8_t capacity, uint8_t o1, uint8_t o2);
void 					update(Cell **world, uint16_t worldHeight, uint16_t worldWidth);
void 					fillGrid(uint16_t **grid, uint16_t gridSize);
void 					fillWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t worldBorder, uint64_t *seed, uint16_t **grid, uint16_t gridSize);
COLORREF 			cellToColor(Cell currentCell);
uint64_t 			rol64(uint64_t x, int k);
uint64_t 			xoshiro256ss(uint64_t *s);
void 					DebugLog(char* format, ...);
