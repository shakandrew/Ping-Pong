#include "Ping-pong_main.h"

// CONST VARIABLES
const TCHAR szWindowClass[] = _T("Pin-Pong I");//class main window
const TCHAR szTitle[] = _T("Pin-Pong I");//app's title bar
//const UINT LTOR = RegisterWindowMessage((LPCWSTR)("LTOR"));
//const UINT RTOL = RegisterWindowMessage((LPCWSTR)("RTOL"));
const UINT RNAME = RegisterWindowMessage((LPCWSTR)("RNAME"));
const UINT LNAME = RegisterWindowMessage((LPCWSTR)("LNAME"));
const UINT END = RegisterWindowMessage((LPCWSTR)("END"));
const HBRUSH white_brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
const HBRUSH black_brush = (HBRUSH)GetStockObject(BLACK_BRUSH);
// GLOBAL VARIABLES
HINSTANCE hInst;
Ball ball;
Player player;
HWND rhWnd = nullptr;
int winner = 0;
bool ballSent;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc; //A pointer to the window procedure
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance; //A handle to the instance that contains the window procedure for the class
		wcex.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), NULL, IMAGE_ICON, 16, 16, 0);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = szWindowClass;
		wcex.hIconSm = NULL;
	}

	//Registring a window class
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T(""), NULL);
		return 1;
	}

	hInst = hInstance; // Store instance handle in our global variable
	RECT rect = prepareWindowSize();

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T(""), NULL);
		return 1;
	}

	// Set window resizable - false
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	dwStyle &= ~(WS_MAXIMIZE | WS_SIZEBOX);
	SetWindowLong(hWnd, GWL_STYLE, dwStyle);

	// Timer
	SetTimer(hWnd, 1, TIMER_PERIOD, NULL);
	SetMapMode(GetDC(hWnd), MM_LOMETRIC);
	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	postHWND(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//A message loop to listen for the messages that the operating system sends
	MSG msg;
	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == RNAME)
		return onConnectLeftWindow(hWnd, wParam, lParam);

	if (message == END)
	{
		winner = (INT)lParam;
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		Sleep(5000);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
	}

	switch (message)
	{
		HANDLE_MSG(hWnd, WM_PAINT, onPaint);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, onMouseMove);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, onMouseClick);
		HANDLE_MSG(hWnd, WM_COPYDATA, onCopyData);
		HANDLE_MSG(hWnd, WM_TIMER, onTimerTick);
		HANDLE_MSG(hWnd, WM_DESTROY, onEnd);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}



// BEGIN
// REGULAR FUNCTIONS
void checkBallCrossesTheBorder(HWND hWnd)
{
	RECT rect = ball.getRect();
	if (rect.right > SCREEN_WIDTH && ball.getMoveSide() > 0)
	{
		if (rect.left < SCREEN_WIDTH)// if ball crosses the border send his coordinates
		{
			if (!ballSent)
			{
				sendBallState(ball.transfer(true), hWnd);
				ballSent = true;
			}
		}
		else
		{
			ballSent = false;
			ball.setBallParams(false, 0, 0, 0, 0);
		}
	}
	if (rect.left < 0)
	{
		PostMessage(rhWnd, END, NULL, (LPARAM)2);
		PostMessage(hWnd, END, NULL, (LPARAM)2);
	}
}

void drawBall(HDC hdc)
{
	HDC memDc = CreateCompatibleDC(hdc);// HDC for bitmap word
	int ball_d = 2 * BALL_RADIUS;
	RECT rect = ball.getRect();
	RECT bitmapRect = { 0, 0, ball_d, ball_d };
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, ball_d, ball_d);// bitmap where we will be drawing
	SelectObject(memDc, hBitmap);
	SelectObject(memDc, GetStockObject(BLACK_BRUSH));
	FillRect(memDc, &bitmapRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	if (ball.getState())
		Ellipse(memDc, 0, 0, ball_d, ball_d);
	BitBlt(hdc, max(0, rect.left), max(0, rect.top), rect.right, rect.bottom, memDc, abs(min(rect.left, 0)), abs(min(rect.top, 0)), SRCCOPY);
	DeleteObject(hBitmap);
	DeleteDC(memDc);
}

void drawPlayer(HDC hdc)
{
	HDC memDc = CreateCompatibleDC(hdc);// HDC for bitmap word
	RECT rect = player.getRect();
	RECT bitmapRect = { 0, 0, PLAYER_WIDTH * 2, SCREEN_HEIGHT };
	RECT bitmapPlayerRect = { 0, rect.top, PLAYER_WIDTH * 2, rect.bottom };
	HBITMAP hBitmap = CreateCompatibleBitmap(hdc, bitmapRect.right, bitmapRect.bottom);// bitmap where we will be drawing
	SelectObject(memDc, hBitmap);
	FillRect(memDc, &bitmapRect, (HBRUSH)GetStockObject(WHITE_BRUSH));//in the beggining out bitmap is full of 0 - black color, we should change it to
	FillRect(memDc, &bitmapPlayerRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	BitBlt(hdc, rect.left, 0, rect.right, SCREEN_HEIGHT, memDc, 0, 0, SRCCOPY);
	DeleteObject(hBitmap);
	DeleteDC(memDc);
}

void drawFinalStage(HDC hdc)
{
	RECT rect = { 0,0,SCREEN_WIDTH, SCREEN_HEIGHT };
	FillRect(hdc, &rect, white_brush);
	RECT textArea = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, SCREEN_WIDTH - SCREEN_WIDTH / 4, SCREEN_HEIGHT - SCREEN_HEIGHT / 3 };
	SetTextColor(hdc, 0x00000000);
	SetBkMode(hdc, TRANSPARENT);
	if (winner == 1)
		DrawText(hdc, (LPCWSTR)L"1st Player Won", -1, &textArea, DT_SINGLELINE | DT_NOCLIP | DT_CENTER);
	else
		DrawText(hdc, (LPCWSTR)L"2nd Player Won", -1, &textArea, DT_SINGLELINE | DT_NOCLIP | DT_CENTER);
}

void postHWND(HWND hWnd)
{
	PostMessage(HWND_BROADCAST, LNAME, NULL, (LPARAM)hWnd);
}

const RECT prepareWindowSize()
{
	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	if (AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE))
		return rect;
	else
		return rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
}

// **TODO use found window
bool sendBallState(Ball& ball, HWND hParent)
{
	if (rhWnd == nullptr)
		return false;
	COPYDATASTRUCT cds;
	cds.dwData = 0;
	cds.cbData = sizeof(ball);
	cds.lpData = &ball;
	SendMessage(rhWnd, WM_COPYDATA, (WPARAM)hParent, (LPARAM)&cds);
}
// END

// BEGIN
// MESSAGES BLOCK

int onConnectLeftWindow(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	rhWnd = (HWND)lParam;
	return 0;
}

// **TODO fix source window
int onCopyData(HWND hWnd, HWND hSourceWnd, PCOPYDATASTRUCT cds)
{
	UNREFERENCED_PARAMETER(hWnd);
	if (cds->cbData == sizeof(ball))
	{
		TCHAR sourceClassName[256]; // MSDN garanties this length
		GetClassName(hSourceWnd, sourceClassName, sizeof(sourceClassName));
		if (lstrcmp(sourceClassName, TEXT("Pin-Pong II")) == 0)
		{
			//Safe data copy
			memcpy(&ball, cds->lpData, sizeof(ball));
			ball = ball.transfer(false);
		}
	}
	return 0;
}

int onEnd(HWND hWnd)
{
	PostQuitMessage(0);
	return 0;
}

int onMouseClick(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if (!ball.getState() && rhWnd != nullptr) {
		RECT rect = player.getRect();
		ball.setBallParams(true, rect.left - 10 * BALL_RADIUS, rect.top + PLAYER_HEIGHT, 1, 0);
	}
	return 0;
}

int onMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	HDC hdc = GetDC(hWnd);
	RECT rect = player.getRect();
	RECT moveRect = { rect.left, 0, rect.right, SCREEN_HEIGHT };
	InvalidateRect(hWnd, &moveRect, TRUE);
	player.setY(y); // set new Y possition
	ReleaseDC(hWnd, hdc);
	return 0;
}

int onPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	if (winner == 0)
	{
		drawPlayer(hdc);
		drawBall(hdc);
	}
	else
		drawFinalStage(hdc);
	EndPaint(hWnd, &ps);
	return 0;
}


int onTimerTick(HWND hWnd, UINT uTimerId)
{
	UNREFERENCED_PARAMETER(uTimerId);
	if (!ball.getState() || rhWnd == nullptr)
		postHWND(hWnd);
	if (ball.getState())
	{
		InvalidateRect(hWnd, &ball.getRect(), TRUE);
		ball.moveBall();
		ball.collision(player.getRect());
		InvalidateRect(hWnd, &ball.getRect(), TRUE);
		checkBallCrossesTheBorder(hWnd);
	}
	return 0L;
}
// END

// BEGIN
// PLAYER IMPLEMENTATION
Player::Player()
{
	setPlayerParams(2 * PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_WIDTH, PLAYER_HEIGHT);
}
void Player::setPlayerParams(int _X, int _Y, int _mX, int _mY)
{
	X = _X;
	Y = _Y;
	mX = _mX;
	mY = _mY;
}
void Player::setY(int _Y)
{
	Y = max(PLAYER_HEIGHT, min(SCREEN_HEIGHT - PLAYER_HEIGHT, _Y));
}
const RECT Player::getRect()
{
	RECT rect = { X - mX, Y - mY, X + mX, Y + mY };
	return rect;
}
// END

// BEGIN
// BALL IMPLEMENTATION
Ball::Ball()
{
	setBallParams(false, 3 * PLAYER_WIDTH + BALL_RADIUS, BALL_RADIUS, 0, 0);
}
void Ball::setBallParams(bool _state, double _X, double _Y, double _dX, double _dY)
{
	state = _state;
	X = _X;
	Y = _Y;
	dX = _dX;
	dY = _dY;
}
void Ball::moveBall()
{
	if (Y < BALL_RADIUS || Y > SCREEN_HEIGHT - BALL_RADIUS)
		dY = -dY;
	X = X + MAGIC_NUMBER * dX;
	Y = Y + MAGIC_NUMBER * dY;
}
int Ball::getMoveSide()
{
	if (dX < 0)
		return -1;
	else
		return 1;
}
void Ball::collision(RECT playerRect)
{
	if (dX < 0 && Y > playerRect.top && Y < playerRect.bottom &&
		X - BALL_RADIUS - playerRect.right <= REFLECTION &&
		X - BALL_RADIUS - playerRect.right >= -REFLECTION)
	{
		double angle = ((Y - playerRect.top - PLAYER_HEIGHT) / PLAYER_HEIGHT) * 90;
		ball.dY = sin(angle*PI / 180);
		ball.dX = cos(angle*PI / 180);
	}
}
Ball Ball::transfer(bool toR)
{
	Ball result;
	if (toR)
	{
		result.state = state;
		result.X = -BALL_RADIUS;
		result.Y = Y;
		result.dX = dX;
		result.dY = dY;
	}
	else
	{
		result.state = state;
		result.X = SCREEN_WIDTH + BALL_RADIUS;
		result.Y = Y;
		result.dX = dX;
		result.dY = dY;
	}
	return result;
}
bool Ball::getState()
{
	return state;
}
const RECT Ball::getRect()
{
	RECT rect = { (LONG)(X - BALL_RADIUS), (LONG)(Y - BALL_RADIUS), (LONG)(X + BALL_RADIUS), (LONG)(Y + BALL_RADIUS) };
	return rect;
}
// END