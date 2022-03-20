#include "main.h"


int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, LPSTR CommandLine, int CmdShow)
{
	int argc = __argc;
	char **argv = __argv;
	DebugLog("%d", argc);
	DebugLog("%s", argv[0]);

	uint16_t worldHeight = 1080;
	uint16_t worldWidth = 1920;
	uint16_t worldBorder = 646;
	uint16_t gridSize = 140;

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
	fillWorld(world, worldHeight, worldWidth, worldBorder, 14, grid, gridSize);
	// Source: https://www.guidgenerator.com/online-guid-generator.aspx
	const char* uniqueClassName = "99ee9a0b-9a7c-4ef7-b2f5-2775c626d119";

	WNDCLASSEXA WindowClass;
	HWND WindowHandle;

	WindowClass.cbSize 				= sizeof(WNDCLASSEXA);
	WindowClass.style 				= 0;
	WindowClass.lpfnWndProc 	= MainWindowProc;
	WindowClass.cbClsExtra 		= 0;
	WindowClass.cbWndExtra 		= 0;
	WindowClass.hInstance 		= Instance;
	WindowClass.hIcon 				= LoadIconA(NULL, IDI_APPLICATION);
	WindowClass.hCursor 			= LoadCursorA(NULL, IDC_ARROW);
	WindowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	WindowClass.lpszMenuName 	= NULL;
	WindowClass.lpszClassName = uniqueClassName;
	WindowClass.hIconSm 			= LoadIconA(NULL, IDI_APPLICATION);

	if (!RegisterClassExA(&WindowClass)) {
		MessageBox(NULL, "Window Registration Unsuccesful", "Error Message", MB_ICONEXCLAMATION | MB_OK);
	 	return 1;
	}

	WindowHandle = CreateWindowExA(0, WindowClass.lpszClassName, "Simulation", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, NULL, NULL, Instance, (LPVOID) world);

	if (!WindowHandle)  {
		MessageBox(NULL, "Window Creation Unsuccesful", "Error Message", MB_ICONEXCLAMATION | MB_OK);
		return 2;
	}

	MSG Message;

	while (GetMessageA(&Message, NULL, 0, 0) > 0) {
		TranslateMessage(&Message);
		DispatchMessageA(&Message);
	}

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

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){

	switch (uMsg)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			Cell** lpData = (Cell**)lpcs->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)lpData);
			return 0;
		}

		case WM_PAINT:
		{

			Cell** world = (Cell**)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			COLORREF col = RGB(135, 206, 235);
			for (unsigned int j = 0; j < 1080; j++) {
				for (unsigned int i = 0; i < 1920; i++) {
					Cell a = world[j][i];
					SetPixel(hdc, i, j, typeToColor(a.type, a.waterOccupied));
				}
			}

			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		default: break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// TBO
void flow(Cell **world, Point currentPoint, uint16_t worldWidth) {
	int16_t i = currentPoint.x;
	int16_t j = currentPoint.y;
	int16_t y = j + 1;

	Cell middle = world[y][i];
	Cell left;
	if ((i - 1) < 0) {
		left = middle;
	} else {
		left = world[y][i - 1];
	}
	Cell right;
	if ((i + 1) == worldWidth) {
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

uint8_t capacityAccessibility(uint8_t occupied, uint8_t capacity, uint8_t o1, uint8_t o2) {
	uint8_t diff = capacity - occupied;
	return (diff) + (diff * (occupied < o1)) + (diff * (occupied < o2));
}

void update(Cell **world, uint16_t worldHeight, uint16_t worldWidth) {
	for (int16_t j = worldHeight - 1; j >= 0; j--) {
		int16_t y = j + 1;
		for (int16_t i = worldWidth - 1; i >= 0; i--) {
			if (y < worldHeight) {
				Cell currentCell = world[j][i];
				Point currentPoint = { .x = i, .y = j };
				if (currentCell.waterCapacity && currentCell.waterOccupied) {
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
		if (j != 0) {
			for (uint16_t p = 0; p < gridSize; p++) {
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
void fillWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t worldBorder, uint16_t seed, uint16_t **grid, uint16_t gridSize) {
	srand(seed);

	uint16_t points = rand() % 10 + 10;
	uint16_t radius = (gridSize - 1) / 2;

	for (uint16_t j = 0; j < worldHeight; j++) {
		for (uint16_t i = 0; i < worldWidth; i++) {
			Cell c;
			c.type = 'S';
			c.waterOccupied = (rand() % 28) + 10;
			c.waterCapacity = rand() % 14;
			if (c.waterCapacity) {
				c.type = 'G';
			}
			c.nutrition = 0;
			c.sun = 0;
			if(j < worldBorder) {
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
					c.waterOccupied += 14;
					c.waterCapacity += grid[j][i];
					world[y][x] = c;
				}
			}
		}
	}
}

COLORREF typeToColor(char type, uint8_t occupied) {
	int r;
	int g;
	int b;
	switch (type) {
		case 'G':
			r = 160;
			g = 80;
			b = 30;
			break;
		case 'A':
			r = 160;
			g = 230;
			b = 255;
			break;
		case 'S':
			r = 128;
			g = 128;
			b = 128;
			break;
		default:
			r = 255;
			g = 255;
			b = 255;
			break;
	}
	return RGB(r,g,b);
}

void DebugLog(char* format, ...)
{
     va_list va;
     va_start(va, format);

     char str[128];
     vsnprintf(str, sizeof(str), format, va);

     va_end(va);

     OutputDebugStringA(str);
}
