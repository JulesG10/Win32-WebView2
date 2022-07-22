#pragma once
#include "AppCore.h"

class DeskApps : public AppCore
{
public:
	DeskApps(AppCoreOptions options);
	~DeskApps();

	static DeskApps *MainFrame();

protected:
	VOID OnJsonMessage(Json::Value) override;
	VOID OnExit() override;
	VOID OnInit() override;
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};
