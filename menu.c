#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
/******************************************************************************
* N.Kozak // Lviv'2019 // Console menu with actions (draft, previous version) *
*    file: menu.c                                                             *
*******************************************************************************/

//#include "stdafx.h"

// from HKEY_USERS\.DEFAULT\Keyboard Layout\Preload
#define ENG_KEYBOARD_LAYOUT "00000409"
#define UKR_KEYBOARD_LAYOUT "00000422"
#define RUS_KEYBOARD_LAYOUT "00000419"

#include "stdio.h"
//#include <stdlib.h>
#include <string.h>
#if _WIN32
#include "conio.h"
//#define getch _getch
#else // #elif __linux__
//#include <curses.h>
#include <termios.h>
#include <unistd.h> // //
#endif
#include <stdlib.h>
#include <wchar.h>
#if _WIN32
#include <windows.h>
#endif

//#define DISABLE_DEFAULT_ACTION

#define MAX_MENU_SPACE 20
#define MAX_LENGTH_MENU_NAME 10
#define MAX_MENU_ITEMS_COUNT 5

#if _WIN32

#define TYPER_FULL_RAW_MODE

#define IS_KEY_UP(CH0, CH1, CH2) (CH0 == 224 && CH1 == 72)
#define IS_KEY_DOWN(CH0, CH1, CH2) (CH0 == 224 && CH1 == 80)
#define IS_KEY_LEFT(CH0, CH1, CH2) (CH0 == 224 && CH2 == 75)
#define IS_KEY_RIGHT(CH0, CH1, CH2) (CH0 == 224 && CH2 == 77)
#define ESCAPE_SEQUENSE_INIT(CH0, CH1) { if (CH0 == 224) CH1 = _getch(); }
#define IS_ESCAPE_SEQUENSE_PREPARE(CH0, CH1, CH2)
#define IS_ESCAPE_KEY(CH0, CH1) (CH0 == 224)
#define IS_KEY_DELETE_PREPARE(CH0, CH1, CH2, CH3) 
#define IS_KEY_DELETE(CH0, CH1, CH2, CH3) (CH0 == 224 && CH1 == 83)
#define IS_KEY_BACKSPACE(CH0) (CH0 == 8)
#define IS_KEY_ENTER(CH0) (CH0 == 13)
#define IS_KEY_CTRLC(CH0) (CH0 == 3)

#else // #elif __linux__

#define TYPER_FULL_RAW_MODE

#define IS_KEY_UP(CH0, CH1, CH2) (CH0 == 0x1b && CH1 == '[' && CH2 == 'A')
#define IS_KEY_DOWN(CH0, CH1, CH2) (CH0 == 0x1b && CH1 == '[' && CH2 == 'B')
#define IS_KEY_LEFT(CH0, CH1, CH2) (CH0 == 0x1b && CH1 == '[' && CH2 == 'D')
#define IS_KEY_RIGHT(CH0, CH1, CH2) (CH0 == 0x1b && CH1 == '[' && CH2 == 'C')
#define ESCAPE_SEQUENSE_INIT(CH0, CH1) { if (!kbhit()) CH1 = 0x1b; else if (CH0 == 0x1b) CH1 = getch(); }
#define IS_ESCAPE_SEQUENSE_PREPARE(CH0, CH1, CH2) {if(CH0 == 0x1b && CH1 == '[') CH2 = getch();}
#define IS_ESCAPE_KEY(CH0, CH1) (CH0 == 0x1b && CH1 == 0x1b)
#define IS_KEY_DELETE_PREPARE(CH0, CH1, CH2, CH3) {if(CH0 == 0x1b && CH1 == '[' && CH2 == '3') CH3 = getch();}
#define IS_KEY_DELETE(CH0, CH1, CH2, CH3) (CH0 == 0x1b && CH1 == '[' && CH2 == '3') // && CH3 == '^')
#define IS_KEY_BACKSPACE(CH0) (CH0 == 127)
#ifdef TYPER_FULL_RAW_MODE
#define IS_KEY_ENTER(CH0) (CH0 == 13)
#else
#define IS_KEY_ENTER(CH0) (CH0 == 10)
#endif
#define IS_KEY_CTRLC(CH0) (CH0 == 3)

#endif

#if _WIN32
#else // #elif __linux__

static struct termios term, oterm;

static int getch(void){
	int c = 0;

	tcgetattr(0, &oterm);
	memcpy(&term, &oterm, sizeof(term));
#ifdef TYPER_FULL_RAW_MODE
	term.c_iflag |= IGNBRK;
	term.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
	term.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN);
#else
	term.c_lflag &= ~(ICANON | ECHO);
#endif
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &term);
	c = getchar();
	tcsetattr(0, TCSANOW, &oterm);
	return c;
}

static int kbhit(void){
	int c = 0;

	tcgetattr(0, &oterm);
	memcpy(&term, &oterm, sizeof(term));
#ifndef TYPER_FULL_RAW_MODE
	term.c_iflag |= IGNBRK;
	term.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF);
	term.c_lflag &= ~(ICANON | ECHO | ECHOK | ECHOE | ECHONL | ISIG | IEXTEN);
#else
	term.c_lflag &= ~(ICANON | ECHO);
#endif
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;
	tcsetattr(0, TCSANOW, &term);
	c = getchar();
	tcsetattr(0, TCSANOW, &oterm);
	if (c != -1) ungetc(c, stdin);
	return ((c != -1) ? 1 : 0);
}
#endif

void goToXY(int x, int y){
#if _WIN32
	COORD p = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), p);
#else // #elif __linux__
	printf("\033[%d;%dH", y + 1, x + 1);
    //move(y, x);//mvgetch(y, x);
#endif	
}

void clearGoToXY(int x, int y) {
#if _WIN32
	COORD topLeft = { x, y };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
	SetConsoleCursorPosition(console, topLeft);
#else // #elif __linux__
#define ESC "\033"
	printf(ESC "c");
	printf("\033[%d;%dH", y + 1, x + 1);
    //move(y, x);//mvgetch(y, x);
#endif		
}

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_ENTER 13
#define KEY_BACKSPACE 8
#define KEY_ESCAPE 27

unsigned int runSpaceOp(int x, int y, int kolommen, char spaceOp[MAX_MENU_SPACE][MAX_MENU_SPACE], int rijen/* = MAX_MENU_SPACE*/){
    int ch0, ch1, ch2 = 0, ch3;
	int index = 0, indey = 0, lol = MAX_MENU_SPACE;
    do{
	    goToXY(index + x, indey + y);
#if _WIN32
    	ch0 = _getch();
#else // #elif __linux__
    	ch0 = getch();
#endif
    	ch1 = 0; //
    	ESCAPE_SEQUENSE_INIT(ch0, ch1);
    	IS_ESCAPE_SEQUENSE_PREPARE(ch0, ch1, ch2);
    	IS_KEY_DELETE_PREPARE(ch0, ch1, ch2, ch3);
    	if (IS_KEY_CTRLC(ch0)) {
    		// no action
    	}
    	else if (IS_KEY_ENTER(ch0)) {
			return indey;
    	}
    	else if (IS_KEY_BACKSPACE(ch0)) {
    		// no action
    	}
    	else if (IS_KEY_DELETE(ch0, ch1, ch2, ch3)) {
    		// no action
    	}
    	else if (IS_KEY_UP(ch0, ch1, ch2)) {
    		--indey;    
			indey = (indey <= 0) ? 0 : (indey > kolommen) ? kolommen : indey;		
    	}		
    	else if (IS_KEY_LEFT(ch0, ch1, ch2)) {
    		--index;    
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;			
    	}
    	else if (IS_KEY_DOWN(ch0, ch1, ch2)) {
    		++indey;
			indey = (indey <= 0) ? 0 : (indey > kolommen) ? kolommen : indey;		
    	}		
    	else if (IS_KEY_RIGHT(ch0, ch1, ch2)) {
    		++index;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;			
    	}
    	else if (IS_ESCAPE_KEY(ch0, ch1)){
#if _WIN32
    	    _getch();
#else // #elif __linux__
    		while (kbhit()) {
    			getch();
    		}
#endif
    	}
    	else if (ch0 == 32 || (ch0 >= 46 && ch0 <= 57) || (ch0 <= 64 && ch0 >= 126)){
			spaceOp[index][indey] = (char)ch0;
			printf("%c", spaceOp[index][indey]);
			++index;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}		
	} while (IS_ESCAPE_KEY(ch0, ch0));
	
	return -1;
}

/*
unsigned int runSpaceOp__OLD(int x, int y, int kolommen, char spaceOp[MAX_MENU_SPACE][MAX_MENU_SPACE], int rijen = MAX_MENU_SPACE){
    int ch0, ch1, ch2, ch3;
	int index = 0, indey = 0, keuze, lol = MAX_MENU_SPACE;	
	do{
		goToXY(index + x, indey + y);
		keuze = 0;
#if _WIN32
		keuze = _getch();
#else // #elif __linux__
		keuze = getch();
#endif		
		if (keuze == 0 || keuze == 0xE0 || keuze == 224)
		{
			keuze = _getch();
			goToXY(index + x, indey + y);
			switch (keuze)
			{
			case KEY_UP:indey--;
				break;
			case KEY_LEFT: index--;
				break;
			case KEY_DOWN:indey++;
				break;
			case KEY_RIGHT: index++;
				break;
			}
			indey = (indey <= 0) ? 0 : (indey > kolommen) ? kolommen : indey;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}
		if (keuze == 32 || (keuze >= 46 && keuze <= 57) || (keuze <= 64 && keuze >= 126))
		{
			spaceOp[index][indey] = (char)keuze;
			printf("%c", spaceOp[index][indey]);
			index++;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}
		if (keuze == KEY_ENTER)
		{
			return indey;
		}
	} while (keuze != KEY_ESCAPE);

	return -1; //
}
*/

/*
void runTextOp(int x, int y, int kolommen, char spaceOp[MAX_MENU_SPACE][MAX_MENU_SPACE], int rijen = MAX_MENU_SPACE)
{
	int index = 0, indey = 0, keuze, lol = MAX_MENU_SPACE;
	do{
		goToXY(index + x, indey + y);
		keuze = 0;
		keuze = _getch();
		if (keuze == 0 || keuze == 0xE0 || keuze == 224)
		{
			keuze = _getch();
			goToXY(index + x, indey + y);
			switch (keuze)
			{
			case KEY_UP:indey--;
				break;
			case KEY_LEFT: index--;
				break;
			case KEY_DOWN:indey++;
				break;
			case KEY_RIGHT: index++;
				break;
			}
			indey = (indey <= 0) ? 0 : (indey > kolommen) ? kolommen : indey;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}
		if (keuze == 32 || (keuze >= 46 && keuze <= 57) || (keuze <= 64 && keuze >= 126))
		{
			spaceOp[index][indey] = (char)keuze;
			printf("%c", spaceOp[index][indey]);
			index++;
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}
		if (keuze == KEY_BACKSPACE)
		{
			index = index--;
			goToXY(index + x, indey + y);
			printf(" ");
			index = (index <= 0) ? 0 : (index > rijen) ? rijen : index;
		}
		if (keuze == KEY_ENTER)
		{
			index = 0;
			indey++;
			indey = (indey >= kolommen) ? kolommen : indey++;
		}
	} while (keuze != KEY_ESCAPE);
}
*/

void defaultAction(){
#ifndef DISABLE_DEFAULT_ACTION
	clearGoToXY(0, 0);
	printf("default action");
	getchar();
#endif
}

struct MenuElement{
	char menuName[MAX_LENGTH_MENU_NAME];
	struct MenuElement * SubMenuSet;
	void (* action)();
};

extern struct MenuElement menuA[MAX_MENU_ITEMS_COUNT];
extern struct MenuElement menuB[MAX_MENU_ITEMS_COUNT];
extern struct MenuElement menuC[MAX_MENU_ITEMS_COUNT];

struct MenuElement
baseMenu[MAX_MENU_ITEMS_COUNT] = {
	{ "PREV MENU", (struct MenuElement *)NULL, defaultAction },
	{ "MENU A", menuA, (void(*)())NULL },
	{ "MENU B", menuB, (void(*)())NULL },
	{ "MENU C", menuC, (void(*)())NULL },
	{ "", (struct MenuElement *)NULL, (void(*)())NULL }
}
, menuA[MAX_MENU_ITEMS_COUNT] = {
	{ "PREV MENU", baseMenu, (void(*)())NULL },
	{ "MENU A1", (struct MenuElement *)NULL, defaultAction },
	{ "MENU A2", (struct MenuElement *)NULL, defaultAction },
	{ "", (struct MenuElement *)NULL, (void(*)())NULL }
}
, menuB[MAX_MENU_ITEMS_COUNT] = {
	{ "PREV MENU", baseMenu, (void(*)())NULL },
	{ "MENU B1", (struct MenuElement *)NULL, defaultAction },
	{ "MENU B2", (struct MenuElement *)NULL, defaultAction },
	{ "", (struct MenuElement *)NULL, (void(*)())NULL }
}
, menuC[MAX_MENU_ITEMS_COUNT] = {
	{ "PREV MENU", baseMenu, (void(*)())NULL },
	{ "MENU C1", (struct MenuElement *)NULL, defaultAction },
	{ "MENU C2", (struct MenuElement *)NULL, defaultAction },
	{ "", (struct MenuElement *)NULL, (void(*)())NULL }
};

unsigned int render(struct MenuElement * menu){
	unsigned int index = 0;

	clearGoToXY(0, 0);

	if (menu){
		for (; menu[index].menuName[0]; ++index){
			printf(menu[index].menuName);
			goToXY(11, index);
			printf("[ ]\r\n");
		}
	}

	return --index;
}

int main(){	
	struct MenuElement * menu = baseMenu;
	char spaceOp[MAX_MENU_SPACE][MAX_MENU_SPACE];

#ifdef _WIN32	
	PostMessage(
		GetForegroundWindow(),
		WM_INPUTLANGCHANGEREQUEST,
		2,
		(UINT)LoadKeyboardLayoutA(ENG_KEYBOARD_LAYOUT, KLF_ACTIVATE)
		);
#else // #elif __linux__
#endif		

	while (1/*TRUE*/){

		unsigned int lastIndex = render(menu);
		if ((int)lastIndex == -1){
			return 0;
		}
	
		unsigned int selectedIndex = runSpaceOp(12, 0, lastIndex, spaceOp, 0);
		if ((int)selectedIndex == -1){
			return 0;
		}

		if (menu[selectedIndex].action != NULL){
			menu[selectedIndex].action();
		}
		
		if (menu[selectedIndex].SubMenuSet != NULL){
			menu = menu[selectedIndex].SubMenuSet;
		}
	
	}

	return 0;
}