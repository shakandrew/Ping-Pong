#pragma once

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <cmath>
#include <windowsx.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 30
#define BALL_RADIUS 15
#define TIMER_PERIOD 30
#define MAGIC_NUMBER 5
#define REFLECTION 3
#define PI 3.14159265
class Player
{
private:
	int X;
	int Y;
	int mX;
	int mY;
public:
	Player();
	void setPlayerParams(int _X, int _Y, int _mX, int _mY);
	void setY(int _Y);
	const RECT getRect();
};

class Ball
{
private:
	bool state;
	double X;
	double Y;
	double dX;
	double dY;
public:
	Ball();
	void setBallParams(bool _state, double _X, double _Y, double _dX, double _dY);
	void moveBall();
	int getMoveSide();
	void collision(RECT playerRect);
	Ball transfer(bool toR);
	bool getState();
	const RECT getRect();
};

void checkBallCrossesTheBorder(HWND hWnd);
void drawBall(HDC hdc);
void drawPlayer(HDC hdc);
void drawFinalStage(HDC hdc);
void postHWND(HWND hWnd);
const RECT prepareWindowSize();
bool sendBallState(Ball& ball, HWND hParent);

int onConnectLeftWindow(HWND hWnd, WPARAM wParam, LPARAM lParam);
int onCopyData(HWND hWnd, HWND hSourceWnd, PCOPYDATASTRUCT cds);
int onEnd(HWND hWnd);
int onMouseClick(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
int onMouseMove(HWND hWnd, int x, int y, UINT keyFlags);
int onPaint(HWND hWnd);
int onTimerTick(HWND hWnd, UINT uTimerId);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);