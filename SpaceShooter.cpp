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
int npcDirection = 1; // 0 up, 1 left, 2 down, 3 left
int nScore = 0;


struct sPixel
{
	int x;
	int y;
};

struct Player {
	int health = 5;
	sPixel pos = { 60,25 };
};

struct Enemy {
	sPixel pos;
	int health = 1;
	Enemy(int x, int y) {
		pos = { x,y };
	}
	~Enemy() {
		nScore++;
	}
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
	Player player;
	Enemy enemy1(20,5);
	Enemy enemy2(30, 5);
	Enemy enemy3(40, 5);
	Enemy enemy4(50, 5);
	Enemy enemy5(60, 5);
	Enemy enemy6(70, 5);
	Enemy enemy7(80, 5);
	Enemy enemy8(90, 5);
	vector<sPixel> bullets;
	vector<Enemy> enemies;
	bool fired = false;
	bool zKeyDownOld = false;
	
	

	enemies.push_back(enemy1);
	enemies.push_back(enemy2);
	enemies.push_back(enemy3);
	enemies.push_back(enemy4);
	enemies.push_back(enemy5);
	enemies.push_back(enemy6);
	enemies.push_back(enemy7);
	enemies.push_back(enemy8);
	nScore = 0;
	

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
			player.pos.x += 1;
		}

		if (bKeyLeft && (leftCollision == false))
		{
			player.pos.x -= 1;
		}

		if (bKeyUp && (topCollision == false))
		{
			player.pos.y -= 1;
		}

		if (bKeyDown && (bottomCollision == false))
		{
			player.pos.y += 1;
		}
		
		


		// player shoots
		if (zKeyDown && !zKeyDownOld)
		{
			fired = true;
			sPixel Bullet = { player.pos.x,player.pos.y };
			bullets.push_back(Bullet);
		}

		zKeyDownOld = zKeyDown;

		if (player.pos.x == 0)
			leftCollision = true;
		else {
			leftCollision = false;
		}
		if (player.pos.x == nScreenWidth - 1)
			rightCollision = true;
		else {
			rightCollision = false;
		}
		if (player.pos.y == 22)
			topCollision = true;
		else {
			topCollision = false;
		}
		if (player.pos.y == nScreenHeight - 1)
			bottomCollision = true;
		else {
			bottomCollision = false;
		}

		if (npcTimer % 3 == 0) {
			if (npcTimer <= 24) {
				for (auto& enemy : enemies) // access by reference to avoid copying
				{
					enemy.pos.x += 1;
				}
				
			}
			else {
				for (auto& enemy : enemies) // access by reference to avoid copying
				{
					enemy.pos.x -= 1;
				}
			}
			if (npcTimer == 48) {
				npcTimer = 0;
			}
			
			
		}


		//vector<int> bulletLocations;

		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (bullet.x != -1 && bullet.y != -1) {
				bullet.y -= 1;
				for (auto& enemy : enemies) // access by reference to avoid copying
				{
					if (enemy.pos.x == bullet.x && enemy.pos.y == bullet.y && enemy.health != 0)
						enemy.health -= 1;
				}
				
			}
			//if (bullet.y == -1) {
			//	bullets.erase(*bullet);

			//}
		}

		bullets.erase(
			std::remove_if(
				bullets.begin(),
				bullets.end(),
				[](auto& bullet) { return bullet.y == -1; }
			),
			bullets.end()
		);
		enemies.erase(
			std::remove_if(
				enemies.begin(),
				enemies.end(),
				[](auto& enemy) { 
					return enemy.health == 0; 
				}
			),
			enemies.end()
		);

		// ==== DISPLAY ========================================================

		// First we clear the screen
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

		// Then draw Title and Border at the top
		for (int i = 0; i < nScreenWidth; i++)
		{
			screen[2 * nScreenWidth + i] = L'_';
		}
		wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER                      SCORE: %d                                    CONTROLS: left, down, right, up, z", nScore);

		// players, Enemies, and Bullets are drawn
		for (auto& enemy : enemies) // access by reference to avoid copying
		{
			screen[enemy.pos.y * nScreenWidth + enemy.pos.x] = L'W';
		}
		//screen[enemy1.pos.y * nScreenWidth + enemy1.pos.x] = L'W';
		//screen[enemy2.pos.y * nScreenWidth + enemy2.pos.x] = L'W';

		screen[player.pos.y * nScreenWidth + player.pos.x] = L'M';

		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (fired && bullet.y != -1 && bullet.y > 2) {
				screen[bullet.y * nScreenWidth + bullet.x] = L'|';
			}
			else {
				screen[nScreenWidth * nScreenHeight] = L' ';
			}
		}
		zKeyDown = false;
		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
	}
}