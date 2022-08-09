#include "DeskApps.h"


DeskApps::DeskApps(AppCoreOptions options) : AppCore(options)
{

}

DeskApps::~DeskApps()
{
}

DeskApps *DeskApps::MainFrame()
{
	AppCoreOptions _options;
	_options.centerWindow = TRUE;
	_options.height = 600;
	_options.width = 800;
	_options.title = (LPWSTR)L"DeskApss";
	_options.windowClass = (LPWSTR)L"deskappclass";
	_options.style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	_options.showError = TRUE;

	return new DeskApps(_options);
}


VOID DeskApps::OnJsonMessage(Json::Value message)
{
	
}

VOID DeskApps::OnExit()
{
	DeleteFileW(this->webZip);
}

VOID DeskApps::OnInit()
{
	this->webZip = this->PathCombineModuleFileName("data.zip");
	DeleteFileW(this->webZip);

	if (this->ExtractRessource(IDR_WEB_DATA1, (LPWSTR)(L"WEB_DATA"), webZip) != S_OK)
	{
		this->ErrorMessage("ERROR - Load DeskApps", "Fail to load DeskApps Starting page !");
		this->ExitApp();
		return;
	}

	std::wstring tmp(this->webDir);
	if (this->UnZip(this->webZip, std::string(tmp.begin(),tmp.end())) != S_OK)
	{
		this->ErrorMessage("ERROR - Load DeskApps", "Fail to load DeskApps Starting page !");
		this->ExitApp();
		return;
	}

	ICoreWebView2Settings *Settings;
	wbWindow->get_Settings(&Settings);

	Settings->put_IsScriptEnabled(TRUE);				 // javascript
	Settings->put_IsWebMessageEnabled(TRUE);			 // message web
	Settings->put_IsStatusBarEnabled(FALSE);			 // link bottom left
	Settings->put_AreDefaultScriptDialogsEnabled(FALSE); // no alert
	Settings->put_IsZoomControlEnabled(FALSE);			 // no zoom
	Settings->put_AreDefaultContextMenusEnabled(FALSE);	 // no right click
	Settings->put_AreDevToolsEnabled(FALSE);			 // no dev tools
	Settings->put_IsBuiltInErrorPageEnabled(FALSE);		 // disable default error page

	Settings->Release();

	LPWSTR webPagePath = PathCombineModuleFileName(appId+ "\\" + "index.html");
	wbWindow->Navigate(webPagePath);

	EventRegistrationToken navToken;
	wbWindow->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([&](ICoreWebView2 *webview, ICoreWebView2NavigationCompletedEventArgs *args)
																							 {
		BOOL success;
		args->get_IsSuccess(&success);
		if (!success)
		{
			COREWEBVIEW2_WEB_ERROR_STATUS error;
			args->get_WebErrorStatus(&error);
		}

		return S_OK; })
										  .Get(),
									  &navToken);

	EventRegistrationToken winToken;
	wbWindow->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>([&](ICoreWebView2 *webview, ICoreWebView2NewWindowRequestedEventArgs *args)
																						   {
		/*
		LPWSTR uri;
		args->get_Uri(&uri);
		webview->Navigate(uri);
		*/
		args->put_Handled(TRUE);

		return S_OK; })
										 .Get(),
									 &winToken);
}

LRESULT DeskApps::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		if (wbController != nullptr)
		{
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			wbController->put_Bounds(bounds);
		};
		break;
	case WM_DESTROY:
		this->ExitApp();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

VOID DeskApps::ExitApp()
{
	this->wbController->Close();
	this->wbEnv->Release();
	this->wbWindow->Stop();

	UnregisterClassW(options.windowClass, hInstance);
	PostQuitMessage(0);
}
