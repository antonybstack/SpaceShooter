#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <Windows.h>
#include <vector>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 30;
bool leftCollision = false;
bool rightCollision = false;
bool topCollision = false;
bool bottomCollision = false;
int npcTimer = 0;

struct sPixel
{
	int x;
	int y;
};

int main()

{

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; // creates 2d array of the product of the dimensions
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole); // we tell this buffer that it will be the target of our console
	DWORD dwBytesWritten = 0;

	// create game objects
	sPixel Player = { 60,25 };
	sPixel Enemy1 = { 50,5 };
	sPixel Enemy2 = { 70,5 };
	vector<sPixel> bullets;

	bool fired = false;




	while (1) {
		this_thread::sleep_for(50ms);
		npcTimer += 1;

		bool bKeyLeft = false, bKeyRight = false, bKeyUp = false, bKeyDown = false, zKeyDown = false;
		auto t1 = chrono::system_clock::now();


		// Get Input
		bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
		bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
		bKeyUp = (0x8000 & GetAsyncKeyState((unsigned char)('\x26'))) != 0;
		bKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x28'))) != 0;
		zKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x5A'))) != 0;

		//Player Collisions
		if (bKeyRight && (rightCollision == false))
		{
			Player.x += 1;
		}

		if (bKeyLeft && (leftCollision == false))
		{
			Player.x -= 1;
		}

		if (bKeyUp && (topCollision == false))
		{
			Player.y -= 1;
		}

		if (bKeyDown && (bottomCollision == false))
		{
			Player.y += 1;
		}

		// Player shoots
		if (zKeyDown)
		{
			fired = true;
			sPixel Bullet = { Player.x,Player.y };
			bullets.push_back(Bullet);
		}


		if (Player.x == 0)
			leftCollision = true;
		else {
			leftCollision = false;
		}
		if (Player.x == nScreenWidth - 1)
			rightCollision = true;
		else {
			rightCollision = false;
		}
		if (Player.y == 22)
			topCollision = true;
		else {
			topCollision = false;
		}
		if (Player.y == nScreenHeight - 1)
			bottomCollision = true;
		else {
			bottomCollision = false;
		}

		if (npcTimer % 5 == 0) {
			Enemy1.x += 1;
			Enemy2.x += 1;
		}

		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (bullet.x != -1 && bullet.y != -1) {
				bullet.y -= 1;

			}
		}

		// ==== DISPLAY

		// First we clear the screen
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

		// Then draw Title and Border at the top
		for (int i = 0; i < nScreenWidth; i++)
		{
			screen[2 * nScreenWidth + i] = L'_';
		}
		wsprintf(&screen[nScreenWidth + 5], L"MY GAME                                                                     CONTROLS: left, down, right, up, z");

		// Players, Enemies, and Bullets are drawn
		screen[Enemy1.y * nScreenWidth + Enemy1.x] = L'W';
		screen[Enemy2.y * nScreenWidth + Enemy2.x] = L'W';

		screen[Player.y * nScreenWidth + Player.x] = L'M';

		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (fired && bullet.y != -1 && bullet.y > 2) {
				screen[bullet.y * nScreenWidth + bullet.x] = L'|';
			}
			else {
				screen[nScreenWidth * nScreenHeight] = L' ';
			}
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
	}
}
