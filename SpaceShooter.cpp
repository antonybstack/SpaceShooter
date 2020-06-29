#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <Windows.h>
#include <vector>
#include <sstream>
#include <debugapi.h>

using namespace std;

//visual studio output macro
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

unsigned short int nScreenWidth = 120;
unsigned short int nScreenHeight = 30;
bool leftCollision = false;
bool rightCollision = false;
bool topCollision = false;
bool bottomCollision = false;
unsigned short delayTimer = 0;
unsigned short npcDirection = 1; // 0 up, 1 left, 2 down, 3 left
unsigned short gameLevel = 1;
bool gameStart = false;
bool gamePause = false;
bool levelComplete = true;
unsigned int nScore = 0;
wchar_t* screen;
bool zKeyDownOld = false;
DWORD dwBytesWritten;
//WORD wColor;
DWORD cWritten;
COORD coordinate;



struct Coordinate
{
	short x;
	short y;
};

struct GameObject {
	void setHealth(short &h) {
		health = h;
	}
	void setPos(short &x, short &y) {
		pos = { x,y };
	}
	short health;
	Coordinate pos;
	short size;
	// color
};

struct Player : public GameObject {
public:
	short level = 1;
	void setHealth(short h) {
		health = h;
	}
	void setLevel(short l) {
		level = l;
	}
	void setSize(short s) {
		size = s;
	}
	Player() {
		pos = { 60, 25 };
		health = 3;
		size = 1;
	}
	void draw(HANDLE &hConsole) {
		coordinate.X = this->pos.x - 2;
		coordinate.Y = this->pos.y;
		WORD wColor =  FOREGROUND_GREEN  | FOREGROUND_BLUE | FOREGROUND_INTENSITY | 0;
		FillConsoleOutputAttribute(
			hConsole,          // screen buffer handle 
			wColor,           // color to fill with 
			5,            // number of cells to fill 
			coordinate,            // first cell to write to 
			&cWritten);       // actual number written 

		screen[this->pos.y * nScreenWidth + this->pos.x - 2] = L'<';
		screen[this->pos.y * nScreenWidth + this->pos.x - 1] = L'/';
		screen[this->pos.y * nScreenWidth + this->pos.x] = L'^';
		screen[this->pos.y * nScreenWidth + this->pos.x + 1] = L'\\';
		screen[this->pos.y * nScreenWidth + this->pos.x + 2] = L'>';
	}
};

struct LowEnemy : public GameObject {
	LowEnemy(short x, short y) {
		pos = { x,y };
		health = 1;
		size = 1;
	}
	~LowEnemy() {
		nScore += 100;
	}
	void draw(HANDLE& hConsole) {
		coordinate.X = this->pos.x - 1;
		coordinate.Y = this->pos.y;
		WORD wColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
		FillConsoleOutputAttribute(
			hConsole,          // screen buffer handle 
			wColor,           // color to fill with 
			3,            // number of cells to fill 
			coordinate,            // first cell to write to 
			&cWritten);       // actual number written 
		screen[this->pos.y * nScreenWidth + this->pos.x - 1] = L'\\';
		screen[this->pos.y * nScreenWidth + this->pos.x] = L'v';
		screen[this->pos.y * nScreenWidth + this->pos.x + 1] = L'/';
	}
};

struct MedEnemy : public GameObject {
	MedEnemy(short x, short y) {
		pos = { x,y };
		health = 5;
		size = 2;
	}
	~MedEnemy() {
		nScore += 500;
	}
	void draw(HANDLE& hConsole) {
		coordinate.X = this->pos.x - 2;
		coordinate.Y = this->pos.y;
		WORD wColor;
		if (this->health > 1) {
			wColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
		}
		else{
			wColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
		}
		FillConsoleOutputAttribute(
			hConsole,          // screen buffer handle 
			wColor,           // color to fill with 
			5,            // number of cells to fill 
			coordinate,            // first cell to write to 
			&cWritten);       // actual number written 
		
		screen[this->pos.y * nScreenWidth + this->pos.x -2] = L'(';
		screen[this->pos.y * nScreenWidth + this->pos.x - 1] = L'|';
		screen[this->pos.y * nScreenWidth + this->pos.x] = L'v';
		screen[this->pos.y * nScreenWidth + this->pos.x + 1] = L'|';
		screen[this->pos.y * nScreenWidth + this->pos.x + 2] = L')';
	}
};

struct HighEnemy : public GameObject {
	HighEnemy(short x, short y) {
		pos = { x,y };
		health = 10;
		size = 3;
	}
	~HighEnemy() {
		nScore += 1000;
	}
	void draw(HANDLE& hConsole) {

		coordinate.X = this->pos.x - 3;
		coordinate.Y = this->pos.y;
		WORD wColor;
		if (this->health >= 5) {
			wColor = FOREGROUND_RED | FOREGROUND_INTENSITY | 0;
		}
		else if (this->health > 1) {
			wColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
		}
		else {
			wColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
		}
		FillConsoleOutputAttribute(
			hConsole,          // screen buffer handle 
			wColor,           // color to fill with 
			7,            // number of cells to fill 
			coordinate,            // first cell to write to 
			&cWritten);       // actual number written 

		screen[this->pos.y * nScreenWidth + this->pos.x -3] = L'{';
		screen[this->pos.y * nScreenWidth + this->pos.x - 2] = L'(';
		screen[this->pos.y * nScreenWidth + this->pos.x -1] = L'\\';
		screen[this->pos.y * nScreenWidth + this->pos.x] = L'v';
		screen[this->pos.y * nScreenWidth + this->pos.x+1] = L'/';
		screen[this->pos.y * nScreenWidth + this->pos.x + 2] = L')';
		screen[this->pos.y * nScreenWidth + this->pos.x + 3] = L'}';
	}
};

vector<shared_ptr<LowEnemy>> lowEnemies;
vector<shared_ptr<MedEnemy>> medEnemies;
vector<shared_ptr<HighEnemy>> highEnemies;
Player player;
vector<Coordinate> bullets;


void update()
{
	switch (gameLevel)
	{
	case 1: {
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(20,5)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(30, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(40, 5)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(50, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(60, 5)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(70, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(80, 5)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(90, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(100, 5)));
		break;
	}
	case 2:
		
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(20,8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(30, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(40, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(50, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(60, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(70, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(80, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(90, 8)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(15, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(30, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(80, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(95, 5)));
		
		break;

	case 3:
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(17, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(27, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(37, 8)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(47, 12)));
		highEnemies.push_back(unique_ptr<HighEnemy>(new HighEnemy(57, 8)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(67, 12)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(77, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(87, 8)));
		lowEnemies.push_back(unique_ptr<LowEnemy>(new LowEnemy(97, 8)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(12, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(27, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(87, 5)));
		medEnemies.push_back(unique_ptr<MedEnemy>(new MedEnemy(102, 5)));
		break;
	}

}


int main()
{
	// detects memory leaks and outputs after program terminates
	//visual studio output for debugging purposes
	//DBOUT("The value of x is " << debug.c_str()); //if variable is string
	//DBOUT("The value of x is " << debug1); //if variable is int

	// Create Screen Buffer
	screen = new wchar_t[nScreenWidth * nScreenHeight]; // creates 2d array of the product of the dimensions
	for (short i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole); // we tell this buffer that it will be the target of our console

	

	// game loop

	while (1) {
		this_thread::sleep_for(25ms);
		delayTimer += 1;
		bool bKeyLeft = false, bKeyRight = false, bKeyUp = false, bKeyDown = false, zKeyDown = false, escDown = false, enterDown = false;
		auto t1 = chrono::system_clock::now();

		// Get Input
		bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
		bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
		bKeyUp = (0x8000 & GetAsyncKeyState((unsigned char)('\x26'))) != 0;
		bKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x28'))) != 0;
		zKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x5A'))) != 0;
		escDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x1B'))) != 0;
		enterDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x0D'))) != 0;

		if (!gameStart) {
			if (enterDown == true) {
				gameStart = true;
			}
			// ==== DISPLAY ========================================================

			// First we clear the screen
			for (short i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			// Then draw Title and Border at the top
			for (short i = 0; i < nScreenWidth; i++)
			{
				// condition solves Warning C6386 Buffer overrun
				if ((2 * nScreenWidth + i) < (nScreenWidth * nScreenHeight * 2))
					screen[2 * nScreenWidth + i] = L'_';
			}
			wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER           SCORE: %d                        CONTROLS: left, right, down, up, z        'esc' to exit", nScore);
			if (delayTimer % 50 <= 25) {
				wsprintf(&screen[nScreenWidth * 15 + 5], L"                                        >>  PRESS 'ENTER' to start  <<                                            ");
			}
			else {
				wsprintf(&screen[nScreenWidth * 15 + 5], L"                                            PRESS 'ENTER' to start                                                ");
			}
			// wsprintf(&screen[nScreenWidth *2], L"debug info: %d", bullets.size());


			if (escDown)
			{
				exit(1);
			}

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
			if (delayTimer == 100) {
				delayTimer = 0;
			}

		}
		else {
			if (levelComplete == true) {
				gameLevel++;
				update();
				levelComplete = false;
			}

			//Player Collisions
			if (bKeyRight && (rightCollision == false))
			{
				player.pos.x += 1;
			}

			if (bKeyLeft && (leftCollision == false))
			{
				player.pos.x -= 1;
			}

			if (delayTimer % 2 == 0) {
				if (bKeyUp && (topCollision == false))
				{
					player.pos.y -= 1;
				}
			}
			if (delayTimer % 2 == 0) {
				if (bKeyDown && (bottomCollision == false))
				{
					player.pos.y += 1;
				}
			}


			// player shoots
			if (zKeyDown && !zKeyDownOld)
			{
				Coordinate Bullet = { player.pos.x,player.pos.y };
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

			for (auto& bullet : bullets) // access by reference to avoid copying
			{
				if (bullet.x != -1 && bullet.y != -1) {
					bullet.y -= 1;
					for (auto& enemy : lowEnemies) // access by reference to avoid copying
					{
						if ((bullet.x >= enemy->pos.x - enemy->size && bullet.x <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
							enemy->health -= 1;
					}
					for (auto& enemy : medEnemies) // access by reference to avoid copying
					{
						if ((bullet.x >= enemy->pos.x - enemy->size && bullet.x <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
							enemy->health -= 1;
					}
					for (auto& enemy : highEnemies) // access by reference to avoid copying
					{
						if ((bullet.x >= enemy->pos.x - enemy->size && bullet.x <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
							enemy->health -= 1;
					}
				}
			}


			if (delayTimer % 10 == 0) {
				if (delayTimer <= 50) {
					for (auto& enemy : lowEnemies) // access by reference to avoid copying
					{
						enemy->pos.x += 1;
					}
					for (auto& enemy : medEnemies) // access by reference to avoid copying
					{
						enemy->pos.x += 1;
					}
					for (auto& enemy : highEnemies) // access by reference to avoid copying
					{
						enemy->pos.x += 1;
					}
				}
				else {
					for (auto& enemy : lowEnemies) // access by reference to avoid copying
					{
						enemy->pos.x -= 1;
					}
					for (auto& enemy : medEnemies) // access by reference to avoid copying
					{
						enemy->pos.x -= 1;
					}
					for (auto& enemy : highEnemies) // access by reference to avoid copying
					{
						enemy->pos.x -= 1;
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
			lowEnemies.erase(
				std::remove_if(
					lowEnemies.begin(),
					lowEnemies.end(),
					[](auto& enemy) {
						return enemy->health == 0;
					}
				),
				lowEnemies.end()
						);
			medEnemies.erase(
				std::remove_if(
					medEnemies.begin(),
					medEnemies.end(),
					[](auto& enemy) {
						return enemy->health == 0;
					}
				),
				medEnemies.end()
						);
			highEnemies.erase(
				std::remove_if(
					highEnemies.begin(),
					highEnemies.end(),
					[](auto& enemy) {
						return enemy->health == 0;
					}
				),
				highEnemies.end()
						);

			// ==== DISPLAY ========================================================

			// First we clear the screen
			for (short i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			// Then draw Title and Border at the top
			for (short i = 0; i < nScreenWidth; i++)
			{
				// condition solves Warning C6386 Buffer overrun
				if ((2 * nScreenWidth + i) < (nScreenWidth * nScreenHeight * 2))
					screen[2 * nScreenWidth + i] = L'_';
			}
			wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER           SCORE: %d                        CONTROLS: left, right, down, up, z        'esc' to exit", nScore);
			// wsprintf(&screen[nScreenWidth *2], L"debug info: %d", bullets.size());


			/*
			WORD wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
			short playerPos = player.pos.y * nScreenWidth + player.pos.x;
			COORD playerCoordinate;
			playerCoordinate.X = player.pos.x + 3;
			playerCoordinate.Y = player.pos.y;

			coordinate.X = 0;
			coordinate.Y = 0;
			*/
			/*
			FillConsoleOutputAttribute(
				hConsole,          // screen buffer handle 
				wColor,           // color to fill with 
				playerPos - 2,            // number of cells to fill 
				coordinate,            // first cell to write to 
				&cWritten);       // actual number written 
			FillConsoleOutputAttribute(
				hConsole,          // screen buffer handle 
				wColor,           // color to fill with 
				3600 - playerPos,            // number of cells to fill 
				playerCoordinate,            // first cell to write to 
				&cWritten);       // actual number written 
			*/
			// players, Enemies, and Bullets are drawn
			for (auto& enemy : lowEnemies) // access by reference to avoid copying
			{
				enemy->draw(hConsole);
			}
			for (auto& enemy : medEnemies) // access by reference to avoid copying
			{
				enemy->draw(hConsole);
			}
			for (auto& enemy : highEnemies) // access by reference to avoid copying
			{
				enemy->draw(hConsole);
			}

			player.draw(hConsole);


			for (auto& bullet : bullets) // access by reference to avoid copying
			{
				coordinate.X = bullet.x;
				coordinate.Y = bullet.y;
				WORD wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
				
				if (bullet.y != -1 && bullet.y > 2) {
					FillConsoleOutputAttribute(
						hConsole,          // screen buffer handle 
						wColor,           // color to fill with 
						1,            // number of cells to fill 
						coordinate,            // first cell to write to 
						&cWritten);       // actual number written 
					screen[bullet.y * nScreenWidth + bullet.x] = L'|';
				}
			}

			if (lowEnemies.size() == 0 && medEnemies.size() == 0 && highEnemies.size() == 0) {
				levelComplete = true;
			}

			if (delayTimer == 100) {
				delayTimer = 0;
			}
			zKeyDown = false;
			// Display Frame
			if (escDown)
			{
				exit(1);
			}

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
		}
	}
	// main debug flags
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}