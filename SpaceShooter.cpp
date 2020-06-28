#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <Windows.h>
#include <vector>
#include <sstream>
#include <debugapi.h>

using namespace std;

unsigned short int nScreenWidth = 120;
unsigned short int nScreenHeight = 30;
bool leftCollision = false;
bool rightCollision = false;
bool topCollision = false;
bool bottomCollision = false;
int npcTimer = 0;
int npcDirection = 1; // 0 up, 1 left, 2 down, 3 left
int nScore = 0;
string debug = "hello";
int debug1 = 0;

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
		nScore+= 100;
	}
};

//visual studio output macro
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}


int main()
{
	// detects memory leaks and outputs after program terminates
	

	//visual studio output for debugging purposes
	//DBOUT("The value of x is " << debug.c_str()); //if variable is string
	//DBOUT("The value of x is " << debug1); //if variable is int


	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; // creates 2d array of the product of the dimensions
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	DWORD mAttributes = 0x0F | COMMON_LVB_UNDERSCORE;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 1);
    SetConsoleActiveScreenBuffer(hConsole); // we tell this buffer that it will be the target of our console
	int color = 1;
	DWORD dwBytesWritten = 0; 

	/*  FOR FUTURE COLOR IMPLEMENTATION
	DWORD NumberOfCharsWritten;
	COORD coordinate;
	coordinate.X = 60;
	coordinate.Y = 25;
	WriteConsoleOutputAttribute(hConsole, (WORD*)&color, 1, coordinate, &NumberOfCharsWritten);
	*/

	// create game objects
	Player player;
	//Enemy * enemy1= new Enemy(20,5);
	vector<sPixel> bullets;
	vector<Enemy> enemies;
	bool zKeyDownOld = false;



		//enemies.push_back(*enemy1);
	enemies.push_back(Enemy(20,5));
	enemies.push_back(Enemy(30, 5));
	enemies.push_back(Enemy(40, 5));
	enemies.push_back(Enemy(50, 5));
	enemies.push_back(Enemy(60, 5));
	enemies.push_back(Enemy(70, 5));
	enemies.push_back(Enemy(80, 5));
	enemies.push_back(Enemy(90, 5));

	//delete enemy1;
	//enemy1 = nullptr;
	nScore = 0;
	
	while (1) {
		this_thread::sleep_for(50ms);
		npcTimer += 1;


		bool bKeyLeft = false, bKeyRight = false, bKeyUp = false, bKeyDown = false, zKeyDown = false, escDown = false;
		auto t1 = chrono::system_clock::now();

		// Get Input
		bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
		bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
		bKeyUp = (0x8000 & GetAsyncKeyState((unsigned char)('\x26'))) != 0;
		bKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x28'))) != 0;
		zKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x5A'))) != 0;
		escDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x1B'))) != 0;

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

		if (escDown)
		{
			exit(1);
		}

		// player shoots
		if (zKeyDown && !zKeyDownOld)
		{
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

		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (bullet.x != -1 && bullet.y != -1) {
				bullet.y -= 1;
				for (auto& enemy : enemies) // access by reference to avoid copying
				{
					if ((enemy.pos.x >= bullet.x - 1 && enemy.pos.x <= bullet.x + 1) && enemy.pos.y == bullet.y && enemy.health != 0)
						enemy.health -= 1;
				}
			}
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
			// condition solves Warning C6386 Buffer overrun
			if((2 * nScreenWidth + i)< (nScreenWidth * nScreenHeight*2))
				screen[2 * nScreenWidth + i] = L'_';
		}
		wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER                      SCORE: %d                                    CONTROLS: left, down, right, up, z", nScore);
		// wsprintf(&screen[nScreenWidth *2], L"debug info: %d", bullets.size());

		// players, Enemies, and Bullets are drawn
		for (auto& enemy : enemies) // access by reference to avoid copying
		{
			screen[enemy.pos.y * nScreenWidth + enemy.pos.x - 1] = L'\\';
			screen[enemy.pos.y * nScreenWidth + enemy.pos.x] = L'0';
			screen[enemy.pos.y * nScreenWidth + enemy.pos.x + 1] = L'/';
		}

		screen[player.pos.y * nScreenWidth + (player.pos.x - 2)] = L'<';
		
		screen[player.pos.y * nScreenWidth + (player.pos.x-1)] = L'/';
		screen[player.pos.y * nScreenWidth + (player.pos.x)] = L'*';
		screen[player.pos.y * nScreenWidth + (player.pos.x+1)] = L'\\';
		screen[player.pos.y * nScreenWidth + (player.pos.x+2)] = L'>';

		
		for (auto& bullet : bullets) // access by reference to avoid copying
		{
			if (bullet.y != -1 && bullet.y > 2) {
				screen[bullet.y * nScreenWidth + bullet.x] = L'|';
			}
		}
		zKeyDown = false;
		// Display Frame

		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
}
_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}