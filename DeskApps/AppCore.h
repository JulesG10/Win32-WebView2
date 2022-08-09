#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include <Windows.h>
#include <Winuser.h>

#include <wrl.h>
#include <wil/com.h>
#include <json/json.h>
#include "WebView2.h"

#include <stdlib.h>
#include <shlwapi.h>
#include <pathcch.h>


#include<zip.h>
#include <fcntl.h>
#include <io.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib, "rpcrt4.lib")

#define HAS_ERROR(x) x != S_OK

using namespace Microsoft::WRL;

typedef struct AppCoreOptions
{
	BOOL centerWindow;
	DWORD style;
	LPWSTR title;
	LPWSTR windowClass;
	int width, height;
	BOOL showError;
} AppCoreOptions;

class AppCore
{
public:
	AppCore(AppCoreOptions);
	~AppCore();

	INT Start(HINSTANCE hInstance);
	INT StartWithArgs(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

	HWND GethWnd();

protected:
	virtual VOID OnJsonMessage(Json::Value);

	virtual VOID OnExit();
	virtual VOID OnInit();

	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE hInstance = NULL;
	HWND hWnd = nullptr;
	AppCoreOptions options;
	LPWSTR webDir = nullptr;
	std::string appId;

	wil::com_ptr<ICoreWebView2Controller> wbController;
	wil::com_ptr<ICoreWebView2> wbWindow;
	wil::com_ptr<ICoreWebView2Environment> wbEnv;

	VOID ErrorMessage(const char *, const char *);

	HRESULT CreateWebFolder();
	HRESULT ClearUserData();
	HRESULT RemoveDataFolders();

	std::string GetUuid();

	LPWSTR PathCombineModuleFileName(std::string filename);
	HRESULT ExtractRessource(INT id, LPWSTR name, LPWSTR extractname);
	HRESULT UnZip(LPWSTR path, std::string dir);
	BOOL DirectoryExists(LPCTSTR szPath);


	VOID SendJsonMessage(Json::Value);

private:
	HRESULT _RegisterClass();
	HRESULT _CreateWindow();
	HRESULT _CreateContext();

	HRESULT CreateCoreWebViewController(HRESULT result, ICoreWebView2Controller *controller);
	HRESULT OnMessageReceived(ICoreWebView2 *webview, ICoreWebView2WebMessageReceivedEventArgs *args);

	static LRESULT CALLBACK _StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};