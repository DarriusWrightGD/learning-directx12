#include <Windows.h>

//identifies the window that was created
HWND mainWindowHandle = 0;

//initializes the window
bool InitializeWindowsApp(HINSTANCE instanceHandle, int show);

//wraps the message loop
int Run();

//the event handler
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
	return 0;
}