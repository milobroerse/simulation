#include "main.h"


int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, LPSTR CommandLine, int CmdShow){
	void* bitmapMemory;

	int bitmapWidth;
	int bitmapHeight;

	int clientWidth;
	int clientHeight;

	uint16_t worldHeight = 1000;
	uint16_t worldWidth = 1500;
	uint16_t worldBorder = 700;
	uint16_t gridSize = 141;
	uint64_t seed[4] = {1,2,3,4};

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
	fillWorld(world, worldHeight, worldWidth, worldBorder, seed, grid, gridSize);

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

	if (!RegisterClassEx(&WindowClass)) {
		MessageBox(NULL, "Window Registration Unsuccesful", "Error Message", MB_ICONEXCLAMATION | MB_OK);
	 	return 1;
	}

	WindowHandle = CreateWindowEx(0, WindowClass.lpszClassName, "Simulation", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, worldWidth + 16, worldHeight + 39, NULL, NULL, Instance, (LPVOID) world);

	RECT clientRect;
	GetClientRect(WindowHandle, &clientRect);
	clientWidth = clientRect.right - clientRect.left;
	clientHeight = clientRect.bottom - clientRect.top;

	bitmapWidth = clientWidth;
	bitmapHeight = clientHeight;

	int bytesPerPixel = 4;

	bitmapMemory = VirtualAlloc(0, bitmapWidth * bitmapHeight * bytesPerPixel, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	BITMAPINFO bitmapInfo;
	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	// Negative height makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom left
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	HDC DeviceContext = GetDC(WindowHandle);

	int running = 1;

	while (running) {
		MSG Message;
		while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE)) {
			if(Message.message == WM_QUIT) running = 0;
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		for (unsigned int i = 0; i < worldWidth; i++) {
			for (unsigned int j = 0; j < worldHeight; j++) {
				Cell cell = world[j][i];
				setPixelColor(i, j, cellToColor(cell), bitmapMemory, bitmapWidth);
			}
		}

		HBITMAP BitmapHandle = CreateBitmap(bitmapWidth, bitmapHeight, 1, 32, bitmapMemory);

		HDC BitmapDeviceContext = CreateCompatibleDC(NULL);
		SelectObject(BitmapDeviceContext, BitmapHandle);

		BitBlt(DeviceContext, 0, 0, bitmapWidth, bitmapHeight, BitmapDeviceContext, 0, 0, SRCCOPY);
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

void setPixelColor(int x, int y, uint32_t color, void *bitmapMemory, int bitmapWidth) {
    uint32_t *pixel = (uint32_t *)bitmapMemory;
    pixel += y * bitmapWidth + x;
    *pixel = color;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg) {
			case WM_KEYDOWN: {
					switch(wParam) { case 'O': { DestroyWindow(hwnd); }; }
			} break;
			case WM_DESTROY: { PostQuitMessage(0); } break;
			default: { return DefWindowProc(hwnd, uMsg, wParam,  lParam); }
	}
	return 0;
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
void fillWorld(Cell **world, uint16_t worldHeight, uint16_t worldWidth, uint16_t worldBorder, uint64_t *seed, uint16_t **grid, uint16_t gridSize) {
	uint16_t points = xoshiro256ss(seed) % 10 + 10;
	uint16_t radius = (gridSize - 1) / 2;

	for (uint16_t j = 0; j < worldHeight; j++) {
		for (uint16_t i = 0; i < worldWidth; i++) {
			Cell c;
			if (j < worldBorder) {
				c.type = 'A';
				c.waterOccupied =	0;
				c.waterCapacity = 0;
			} else {
				int waterCapacity = xoshiro256ss(seed) % 255;
				if (waterCapacity) {
					c.type = 'G';
					c.waterOccupied = xoshiro256ss(seed) % 255;
					c.waterCapacity = 255; // TODO cap !> occ
				} else {
					c.type = 'S';
					c.waterOccupied =	0;
					c.waterCapacity = 0;
				}
			}
			world[j][i] = c;
		}
	}

	for (uint16_t p = 0; p < points; p++) {
		uint16_t randX = xoshiro256ss(seed) % worldWidth;
		uint16_t randY = (xoshiro256ss(seed) % (worldHeight - worldBorder)) + worldBorder;
		for (uint16_t j = 0; j < gridSize; j++) {
			int16_t l = j - radius;
			for (uint16_t i = 0; i < gridSize; i++) {
				int16_t k = i - radius;
				int16_t x = randX - k;
				int16_t y = randY - l;
				if ((x >= 0 && x < worldWidth) && (y >= worldBorder && y < worldHeight)){
					Cell c = world[y][x];
					world[y][x] = c;
				}
			}
		}
	}
}

COLORREF cellToColor(Cell currentCell) {
	int hex = 0x000000;
	int r, g, b, a;
	switch (currentCell.type) {
		case 'G':
			Color lighestGround = { .a = 255, .r = 175, .g = 105, .b = 70 };
			Color darkestGround = { .a = 255, .r = 25, .g = 15, .b = 10 };

			a = lighestGround.a;
			r = lighestGround.r - (currentCell.waterOccupied * (lighestGround.r - darkestGround.r)) / currentCell.waterCapacity;
			g = lighestGround.g - (currentCell.waterOccupied * (lighestGround.g - darkestGround.g)) / currentCell.waterCapacity;
			b = lighestGround.b - (currentCell.waterOccupied * (lighestGround.b - darkestGround.b)) / currentCell.waterCapacity;

			hex = ((((((hex | a) << 8) | r) << 8) | g) << 8) | b;
			break;
		case 'A':
			r = 135;
			g = 206;
			b = 235;
			a = 255;
			hex = ((((((hex | a) << 8) | r) << 8) | g) << 8) | b;
			break;
		case 'S':
			r = 40;
			g = 40;
			b = 40;
			a = 255;
			hex = ((((((hex | a) << 8) | r) << 8) | g) << 8) | b;
			break;
		default:
			break;
	}
	return hex;
}

uint64_t rol64(uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

uint64_t xoshiro256ss(uint64_t *s)
{
	uint64_t const result = rol64(s[1] * 5, 7) * 9;
	uint64_t const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rol64(s[3], 45);

	return result;
}

void DebugLog(char* format, ...) {
     va_list va;
     va_start(va, format);

     char str[128];
     vsnprintf(str, sizeof(str), format, va);

     va_end(va);

     OutputDebugStringA(str);
}
