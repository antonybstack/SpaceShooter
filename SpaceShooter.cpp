#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <Windows.h>
#include <vector>
#include <sstream>
#include <debugapi.h>
#include <chrono>


//visual studio output macro
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

unsigned short nScreenWidth = 120;
unsigned short nScreenHeight = 30;
bool leftCollision = false;
bool rightCollision = false;
bool topCollision = false;
bool bottomCollision = false;
unsigned short delayTimer = 0;
unsigned short gameLevel = 3;
bool gameStart = false;
bool gameComplete = false;
bool gamePause = false;
bool levelComplete = true;
unsigned int nScore = 0;
wchar_t* screen;
bool zKeyDownOld = false;
DWORD dwBytesWritten;
DWORD cWritten;
COORD coordinate;

struct Coordinate {
	short x;
	short y;
};

struct GameObject {
	short health;
	Coordinate pos;
	short size;
};

struct Player : public GameObject {
	short level = 1;
	Player() {
		pos = { 60, 25 };
		health = 3;
		size = 1;
	}
	void draw(HANDLE &hConsole) {
		// set color
		coordinate.X = this->pos.x - 2;
		coordinate.Y = this->pos.y;
		WORD wColor =  FOREGROUND_GREEN  | FOREGROUND_BLUE | FOREGROUND_INTENSITY | 0;
		FillConsoleOutputAttribute(
			hConsole,          // screen buffer handle 
			wColor,           // color to fill with 
			5,            // number of cells to fill 
			coordinate,            // first cell to write to 
			&cWritten);       // actual number written 

		// draw character
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

// initializing vectors and player object
std::vector<std::shared_ptr<LowEnemy>> lowEnemies;
std::vector<std::shared_ptr<MedEnemy>> medEnemies;
std::vector<std::shared_ptr<HighEnemy>> highEnemies;
std::vector<Coordinate> bullets;
Player player;

// creates enemies for different levels (1-3)
void updateLevel()
{
	switch (gameLevel)
	{
	case 1: {
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(20,5)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(30, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(40, 5)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(50, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(60, 5)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(70, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(80, 5)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(90, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(100, 5)));
		break;
	}
	case 2:
		
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(20,8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(30, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(40, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(50, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(60, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(70, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(80, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(90, 8)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(15, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(30, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(80, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(95, 5)));
		break;

	case 3:
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(17, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(27, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(37, 8)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(47, 12)));
		highEnemies.push_back(std::unique_ptr<HighEnemy>(new HighEnemy(57, 8)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(67, 12)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(77, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(87, 8)));
		lowEnemies.push_back(std::unique_ptr<LowEnemy>(new LowEnemy(97, 8)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(12, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(27, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(87, 5)));
		medEnemies.push_back(std::unique_ptr<MedEnemy>(new MedEnemy(102, 5)));
		break;
	}
}



int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		delayTimer += 1;
		bool bKeyLeft = false, bKeyRight = false, zKeyDown = false, escDown = false, enterDown = false;
		auto t1 = std::chrono::system_clock::now();

		// Get Input
		bKeyRight = (0x8000 & GetAsyncKeyState((unsigned char)('\x27'))) != 0;
		bKeyLeft = (0x8000 & GetAsyncKeyState((unsigned char)('\x25'))) != 0;
		zKeyDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x5A'))) != 0;
		escDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x1B'))) != 0;
		enterDown = (0x8000 & GetAsyncKeyState((unsigned char)('\x0D'))) != 0;

		// Start screen
		if (!gameStart) {
			// ==== > LOGIC < ======================================================
			if (enterDown == true) {
				gameStart = true;
			}
			// ==== > DISPLAY < ====================================================

			// First we clear the screen
			for (short i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			// Then draw Title and Border at the top
			for (short i = 0; i < nScreenWidth; i++)
			{
				// condition solves Warning C6386 Buffer overrun
				if ((2 * nScreenWidth + i) < (nScreenWidth * nScreenHeight * 2)) {
					screen[2 * nScreenWidth + i] = L'_'; // draw border
				}
			}

			// draw title
			wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER           SCORE: %d                                   CONTROLS: left, right, z        'esc' to exit", nScore);
			
			//blinking start screen
			if (delayTimer % 50 <= 25) {
				wsprintf(&screen[nScreenWidth * 15 + 5], L"                                        >>  PRESS 'ENTER' to start  <<                                            ");
			}
			else {
				wsprintf(&screen[nScreenWidth * 15 + 5], L"                                            PRESS 'ENTER' to start                                                ");
			}

			//output console
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
			if (delayTimer == 100) {
				delayTimer = 0;
			}

		}
		else if(gameComplete){
			if (enterDown == true) {
				// reset all objects and variables before reset
				gameComplete = false;
				gameLevel = 0;
				delayTimer = 0;
				levelComplete = true;
				nScore = 0;
				player.level = 1;
				player.pos.x = 60;
				player.pos.y = 25;
				player.health = 3;
				bullets.erase(
					std::remove_if(
						bullets.begin(),
						bullets.end(),
						[](auto& bullet) { return bullet.y >= -1; }
					),
					bullets.end()
				);
			}
			// ==== > DISPLAY < ====================================================

			// First we clear the screen
			for (short i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

			// Then draw Title and Border at the top
			for (short i = 0; i < nScreenWidth; i++)
			{
				// condition solves Warning C6386 Buffer overrun
				if ((2 * nScreenWidth + i) < (nScreenWidth * nScreenHeight * 2)) {
					screen[2 * nScreenWidth + i] = L'_'; // draw border
				}
			}

			// draw title
			wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER           SCORE: %d                                   CONTROLS: left, right, z        'esc' to exit", nScore);

			coordinate.X = 0;
			coordinate.Y = 14;
			WORD wColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
			FillConsoleOutputAttribute(
				hConsole,          // screen buffer handle 
				wColor,           // color to fill with 
				120,            // number of cells to fill 
				coordinate,            // first cell to write to 
				&cWritten);       // actual number written 

			//blinking reset screen
			if (delayTimer % 50 <= 25) {
				wsprintf(&screen[nScreenWidth * 14 + 5], L"                                       ***         YOU WON!         ***                                           ");
			}
			else {
				wsprintf(&screen[nScreenWidth * 14 + 5], L"                                                   YOU WON!                                                       ");
			}


				wsprintf(&screen[nScreenWidth * 16 + 5], L"                                            PRESS 'ENTER' to reset                                                ");



			//output console
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
			if (delayTimer == 100) {
				delayTimer = 0;
			}
		}
		// Game started
		else {
			// ==== > LOGIC < ======================================================
			
			//updates current level
			if (levelComplete == true) {
				gameLevel++;
				updateLevel();
				levelComplete = false;
			}

			// player left and right movement
			if (bKeyRight && (rightCollision == false))
			{
				player.pos.x += 1;
			}

			if (bKeyLeft && (leftCollision == false))
			{
				player.pos.x -= 1;
			}

			// player shoots
			if (zKeyDown && !zKeyDownOld)
			{
				Coordinate Bullet = { player.pos.x,player.pos.y };
				bullets.push_back(Bullet);
			}
			
			// limits player shooting speed
			if (player.level == 3) {
				if (delayTimer % 5 != 0) {
					zKeyDownOld = zKeyDown;
				}
				else {
					zKeyDownOld = false;
				}
			}
			else {
				if (delayTimer % 10 != 0) {
					zKeyDownOld = zKeyDown;
				}
				else {
					zKeyDownOld = false;
				}
			}

			// player collisions
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

			// bullet damage to enemy
			if (player.level == 1) {
				for (auto& bullet : bullets) // access by reference to avoid copying
				{
					if (bullet.x != -1 && bullet.y != -1) {
						bullet.y -= 1; // continue to shift bullet up one pixel to the top of screen
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
			}
			else if (player.level >= 2) {
					for (auto& bullet : bullets)
					{
						if (bullet.x != -1 && bullet.y != -1) {
							bullet.y -= 1; // continue to shift bullet up one pixel to the top of screen

							//level two bullets are twice the size
							for (auto& enemy : lowEnemies)
							{
								if ((bullet.x + 2 >= enemy->pos.x - enemy->size && bullet.x - 2 <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
									enemy->health -= 1;
							}
							for (auto& enemy : medEnemies)
							{
								if ((bullet.x + 2 >= enemy->pos.x - enemy->size && bullet.x - 2 <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
									enemy->health -= 1;
							}
							for (auto& enemy : highEnemies)
							{
								if ((bullet.x + 2 >= enemy->pos.x - enemy->size && bullet.x - 2 <= enemy->pos.x + enemy->size) && enemy->pos.y == bullet.y && enemy->health != 0)
									enemy->health -= 1;
							}
						}
					}
			}

			// enemy movement
			if (delayTimer % 10 == 0) {
				if (delayTimer <= 50) {
					for (auto& enemy : lowEnemies)
					{
						enemy->pos.x += 1;
					}
					for (auto& enemy : medEnemies)
					{
						enemy->pos.x += 1;
					}
					for (auto& enemy : highEnemies)
					{
						enemy->pos.x += 1;
					}
				}
				else {
					for (auto& enemy : lowEnemies)
					{
						enemy->pos.x -= 1;
					}
					for (auto& enemy : medEnemies)
					{
						enemy->pos.x -= 1;
					}
					for (auto& enemy : highEnemies)
					{
						enemy->pos.x -= 1;
					}
				}
			}

			//bullet objects deleted if reaches end of screen
			bullets.erase(
				std::remove_if(
					bullets.begin(),
					bullets.end(),
					[](auto& bullet) { return bullet.y == -1; }
				),
				bullets.end()
			);

			// enemy objects deleted if health becomes 0
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

			// if player's score is 1000, player levels up to 2
			if (nScore >= 1000 && nScore <= 3000) {
				player.level = 2;
			}
			// if player's score is 3000, player levels up to 3
			else if(nScore > 3000){
				player.level = 3;
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
			// draw title
			wsprintf(&screen[nScreenWidth + 5], L"STARSHOOTER           SCORE: %d                                   CONTROLS: left, right, z        'esc' to exit", nScore);

			//draw player
			player.draw(hConsole);

			// draw enemies
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

			// draw bullets
			for (auto& bullet : bullets) // access by reference to avoid copying
			{
				coordinate.X = bullet.x;
				coordinate.Y = bullet.y;
				WORD wColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0;
				
				if (bullet.y != -1 && bullet.y > 2) {
					if (player.level == 1) {
						FillConsoleOutputAttribute(
							hConsole,          // screen buffer handle 
							wColor,           // color to fill with 
							1,            // number of cells to fill 
							coordinate,            // first cell to write to 
							&cWritten);       // actual number written 
						screen[bullet.y * nScreenWidth + bullet.x] = L'|';
					}
					if (player.level >= 2) {
						coordinate.X = bullet.x-1;
						FillConsoleOutputAttribute(
							hConsole,          // screen buffer handle 
							wColor,           // color to fill with 
							3,            // number of cells to fill 
							coordinate,            // first cell to write to 
							&cWritten);       // actual number written 
						screen[bullet.y * nScreenWidth + bullet.x - 1] = L'|';
						screen[bullet.y * nScreenWidth + bullet.x + 1] = L'|';
					}
				}
			}

			// if all enemies of level are destroyed, level is complete
			if (lowEnemies.size() == 0 && medEnemies.size() == 0 && highEnemies.size() == 0) {
				levelComplete = true;
			}

			if (lowEnemies.size() == 0 && medEnemies.size() == 0 && highEnemies.size() == 0 && gameLevel == 4) {
				gameComplete = true;
			}

			//player can exit application with 'esc'
			if (escDown)
			{
				exit(1);
			}

			//output console
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); // we give it the HANDLE, the buffer, the bytes, the coordinate where text starts writing
			if (delayTimer == 100) {
				delayTimer = 0;
			}
		}
	}
}