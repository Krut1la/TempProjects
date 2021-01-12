// This is the main DLL file.

#include "stdafx.h"

#include "id15dll.net.h"


enum {
  MRight = 0,
  MUp = 1,
  MLeft = 2,
  MDown = 3
};

int const BOARD_MAXWIDTH      = 5;
int const BOARD_MAXHEIGHT     = 5;
int const BOARD_MAXCELLSCOUNT = BOARD_MAXWIDTH * BOARD_MAXHEIGHT;
int const MOVES_NUMBER        = 4;
int const MOVES_MAX           = 1024;
int const DEPTH_MAX           = 63;

int FColCount = 4;
int FRowCount = 4;
int FCellCount = 4 * 4;

int FCellsRating[BOARD_MAXCELLSCOUNT][BOARD_MAXCELLSCOUNT];
int FVectSet[BOARD_MAXCELLSCOUNT][BOARD_MAXCELLSCOUNT];
int FTurnSet[MOVES_NUMBER][BOARD_MAXCELLSCOUNT][MOVES_NUMBER + 1];

void RemoveUnnecessary(int *chain);
void InitArrays(int Top, int Bottom);

#include <iostream>

unsigned int __stdcall Solve(unsigned int count)
{
	//printf("sdasd");
	std::cout << "asd";
	return 0;
}

