#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>

int Locate[20][20];
int Turn = 1;
void GameRule(int x, int y);
void BoardDraw();
void gotoxy(int x, int y);
void Putstone(int color);
void Delstone(int x, int y);

void gotoxy(int x, int y) {
	COORD Pos;
	Pos.X = 2 * x - 2;
	Pos.Y = y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void BoardDraw() {				//판 그리기
	int i, j;

	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
	printf("┌─");
	for (i = 0; i < 17; i++)
		printf("┬─");

	printf("┐\n");
	for (i = 0; i < 17; i++) {
		printf("├─");
		for (j = 0; j < 17; j++)
			printf("┼─");
		printf("┤\n");
	}
	printf("└─");
	for (i = 0; i < 17; i++)
		printf("┴─");
	printf("┘");
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void InsertKey() {
	int x = 10, y = 10;
	int key;
	gotoxy(x, y);
	Putstone(Turn);
	while (1) {
		key = _getch();
		switch (key) {
		case 119:
			if (1 < y && y <= 19) {
				Delstone(x, y);
				y--;
				gotoxy(x, y);
				Putstone(Turn);
			}
			break;
		case 115:
			if (1 <= y && y < 19) {
				Delstone(x, y);
				y++;
				gotoxy(x, y);
				Putstone(Turn);
			}
			break;
		case 100:
			if (1 <= x && x < 19) {
				Delstone(x, y);
				x++;
				gotoxy(x, y);
				Putstone(Turn);
			}
			break;
		case 97:
			if (1 < x && x <= 19) {
				Delstone(x, y);
				x--;
				gotoxy(x, y);
				Putstone(Turn);
			}
			break;
		case 32:
			gotoxy(x, y);
			Putstone(Turn);
			Locate[x][y] = Turn;
			GameRule(x, y);
			if (Turn == 1)
				Turn++;
			else if (Turn == 2)
				Turn--;
			break;
		case 27:
			exit(0);
		}
	}
}

void Putstone(int color) {
	if (color == 1)
		printf("○");
	if (color == 2)
		printf("●");
}

void Delstone(int x, int y) {
	if (Locate[x][y] == 0) {
		if (x == 1 && y == 1) {
			gotoxy(x, y);
			printf("┌─");
		}
		else if (x == 1 && y == 19) {
			gotoxy(x, y);
			printf("└─");
		}
		else if (x == 19 && y == 1) {
			gotoxy(x, y);
			printf("┐");
		}
		else if (x == 19 && y == 19) {
			gotoxy(x, y);
			printf("┘");
		}
		else if (x == 1) {
			gotoxy(x, y);
			printf("├─");
		}
		else if (x == 19) {
			gotoxy(x, y);
			printf("┤");
		}
		else if (y == 1) {
			gotoxy(x, y);
			printf("┬─");
		}
		else if (1 < x && x < 19 && y == 19) {
			gotoxy(x, y);
			printf("┴─");
		}
		else if (1 < x && x < 19 && 1 < y && y < 19) {
			gotoxy(x, y);
			printf("┼─");
		}
	}
	else if (Locate[x][y] == 1) {
		gotoxy(x, y);
		Putstone(1);
	}
	else if (Locate[x][y] == 2) {
		gotoxy(x, y);
		Putstone(2);
	}
}

void GameRule(int x, int y) {
	int i, j = 0, count = 0;
	for (i = 0; i < 20; i++) { //가로 체크
		if (Locate[i][y] == Turn) {
			count++;
			gotoxy(30, 3);
			printf("가로 : %d", count);
			if (count == 5) {
				gotoxy(30, 8);
				printf("Game over");
			}
		}
		else
			count = 0;
	}
	for (i = 0; i < 20; i++) { //세로 체크
		if (Locate[x][i] == Turn) {
			count++;
			gotoxy(30, 4);
			printf("세로 : %d", count);
			if (count == 5) {
				gotoxy(30, 8);
				printf("Game over");
			}
		}
		else
			count = 0;
	}
	for (i = 0; i < 20; i++) { //대각선 체크
		if (Locate[i][i] == Turn) {
			count++;
			gotoxy(40, 4);
			printf("대각선↘ : %d", count);
			if (count == 5) {
				gotoxy(30, 8);
				printf("Game over");
			}
		}
		else
			count = 0;
	}
}

int main() {
	gotoxy(0, 0);
	BoardDraw();
	InsertKey();

	return 0;
}