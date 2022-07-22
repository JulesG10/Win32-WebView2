// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <Windows.h>
#include "AppCore.h"
#include "DeskApps.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	return DeskApps::MainFrame()->StartWithArgs(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}