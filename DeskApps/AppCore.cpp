#include "AppCore.h"

AppCore::AppCore(AppCoreOptions _options)
{
	options = _options;
}
AppCore::~AppCore()
{
}

INT AppCore::StartWithArgs(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	this->hInstance = hInstance;
	if (HAS_ERROR(this->_RegisterClass()))
	{
		this->ErrorMessage("ERROR - Regiter Class", "Register window class has failed !");
		return 1;
	}

	if (HAS_ERROR(this->_CreateWindow()))
	{
		this->ErrorMessage("ERROR - Create Window", "Window creation has failed !");
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	if (HAS_ERROR(this->_CreateContext()))
	{
		this->ErrorMessage("ERROR - Create Context", "Webview2 context creation has failed !");
		return 1;
	}

	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	this->OnExit();
	return (INT)message.wParam;
}
INT AppCore::Start(HINSTANCE hInstance)
{
	return this->StartWithArgs(hInstance, NULL, NULL, SW_SHOW);
}

VOID AppCore::OnInit()
{
}
VOID AppCore::OnExit()
{
}

VOID AppCore::OnJsonMessage(Json::Value)
{
}
VOID AppCore::SendJsonMessage(Json::Value value)
{
	Json::StreamWriterBuilder swBuilder;
	std::string reponse = Json::writeString(swBuilder, value);
	std::wstring wResponse(reponse.begin(), reponse.end());
	wbWindow->PostWebMessageAsJson((LPCWSTR)wResponse.c_str());
}

HRESULT AppCore::_CreateContext()
{
	if (this->CreateWebFolder() == E_FAIL)
	{
		return E_FAIL;
	}

	return CreateCoreWebView2EnvironmentWithOptions(nullptr, this->webDir, nullptr, Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([this](HRESULT result, ICoreWebView2Environment *env) -> HRESULT
																																						 {
			if (!env)
			{
				return E_FAIL;
			}

			wbEnv = env;
			env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
				{
					return CreateCoreWebViewController(result, controller);
				}).Get());
			return S_OK; })
																						.Get());
}
HRESULT AppCore::_RegisterClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = AppCore::_StaticWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = options.windowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (RegisterClassExW(&wcex))
	{
		return S_OK;
	}
	return E_FAIL;
}
HRESULT AppCore::_CreateWindow()
{
	RECT rect{};
	MONITORINFO mi = {sizeof(mi)};

	GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
	int w = options.width;
	int h = options.height;
	int x = (mi.rcMonitor.right - mi.rcMonitor.left - w) / 2;
	int y = (mi.rcMonitor.bottom - mi.rcMonitor.top - h) / 2;

	SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

	hWnd = CreateWindowExW(NULL,
						   options.windowClass,
						   options.title,
						   options.style,
						   x, y,
						   w, h,
						   NULL, // options.parent,
						   NULL,
						   hInstance,
						   this);

	if (hWnd)
	{
		return S_OK;
	}
	return E_FAIL;
}

LRESULT AppCore::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

		this->wbController->Close();
		this->wbEnv->Release();

		UnregisterClassW(options.windowClass, hInstance);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}
LRESULT AppCore::_StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	AppCore *app;
	if (message == WM_NCCREATE || message == WM_CREATE)
	{
		CREATESTRUCT *lpcs = reinterpret_cast<CREATESTRUCT *>(lParam);
		app = static_cast<AppCore *>(lpcs->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
		app->hWnd = hWnd;
	}
	else
	{
		app = reinterpret_cast<AppCore *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (app)
	{
		return app->WndProc(hWnd, message, wParam, lParam);
	}
	return NULL;
}

VOID AppCore::ErrorMessage(const char *title, const char *message)
{
	if (options.showError)
	{
		MessageBoxA(NULL, (LPCSTR)message, (LPCSTR)title, MB_OK | MB_ICONERROR);
	}
}
HRESULT AppCore::CreateCoreWebViewController(HRESULT result, ICoreWebView2Controller *controller)
{

	if (controller != nullptr)
	{
		wbController = controller;
		wbController->get_CoreWebView2(&wbWindow);
	}
	else
	{
		return E_FAIL;
	}

	RECT bounds;
	GetClientRect(hWnd, &bounds);
	wbController->put_Bounds(bounds);

	EventRegistrationToken permToken;
	wbWindow->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>([](ICoreWebView2 *webview, ICoreWebView2PermissionRequestedEventArgs *args)
																							 {
			args->put_State(COREWEBVIEW2_PERMISSION_STATE::COREWEBVIEW2_PERMISSION_STATE_ALLOW);
			return S_OK; })
										  .Get(),
									  &permToken);

	EventRegistrationToken msgToken;
	wbWindow->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2 *webview, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT
																						   { return OnMessageReceived(webview, args); })
										 .Get(),
									 &msgToken);

	this->OnInit();

	return S_OK;
}
HRESULT AppCore::CreateWebFolder()
{
	if (HAS_ERROR(this->RemoveDataFolders()))
	{
		return E_FAIL;
	}

	this->appId = this->GetUuid();
	webDir = this->PathCombineModuleFileName(this->appId);

	if (!CreateDirectoryW(webDir, NULL))
	{
		return E_FAIL;
	}

	/*if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (this->RemoveDataFolder() == E_FAIL)
		{
			return E_FAIL;
		}
		else {
			if (!CreateDirectoryW(webDir, NULL))
			{
				return E_FAIL;
			}
		}
	}
	*/

	return S_OK;
}
HRESULT AppCore::OnMessageReceived(ICoreWebView2 *webview, ICoreWebView2WebMessageReceivedEventArgs *args)
{
	LPWSTR jsonMessage;
	args->get_WebMessageAsJson(&jsonMessage);
	if (jsonMessage)
	{
		std::wstring wMessage(jsonMessage);
		std::string message(wMessage.begin(), wMessage.end());

		Json::CharReaderBuilder crBuilder;
		Json::Reader reader;
		Json::Value recievedRoot;

		if (reader.parse(message, recievedRoot))
		{
			this->OnJsonMessage(recievedRoot);
		}
	}
	CoTaskMemFree(jsonMessage);

	return S_OK;
}

std::string AppCore::GetUuid()
{
	UUID uuid;
	char *str;
	std::string out;

	UuidCreate(&uuid);
	UuidToStringA(&uuid, (RPC_CSTR *)&str);
	out = std::string("@") + std::string(str);
	RpcStringFreeA((RPC_CSTR *)&str);

	return out;
}
HWND AppCore::GethWnd()
{
	return hWnd;
}

HRESULT AppCore::ClearUserData()
{
	auto webView2_14 = wbWindow.try_query<ICoreWebView2_14>();
	if (webView2_14)
	{
		wil::com_ptr<ICoreWebView2Profile> webView2Profile;
		webView2_14->get_Profile(&webView2Profile);
		if (webView2Profile)
		{
			auto webView2Profile2 = webView2Profile.try_query<ICoreWebView2Profile2>();
			if (webView2Profile2)
			{
				webView2Profile2->ClearBrowsingDataAll(Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>([](HRESULT error) -> HRESULT
																												{ return S_OK; })
														   .Get());
				return S_OK;
			}
		}
	}
	return E_FAIL;
}
HRESULT AppCore::RemoveDataFolders()
{
	LPWSTR exePath = this->PathCombineModuleFileName("");
	for (const auto &entry : std::filesystem::directory_iterator(exePath))
	{

		if (entry.exists() && entry.is_directory())
		{
			if (entry.path().has_filename() && entry.path().filename().generic_string().rfind("@") == 0)
			{
				SHFILEOPSTRUCTW sf;

				std::wstring cpDir(entry.path().generic_wstring());
				cpDir += std::wstring(1, '\0');

				sf.pFrom = cpDir.c_str();
				sf.wFunc = FO_DELETE;
				sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT;

				if (SHFileOperationW(&sf) != 0)
				{
					// return E_FAIL;
				}
			}
		}
	}

	return S_OK;
}

LPWSTR AppCore::PathCombineModuleFileName(std::string filename)
{
	LPWSTR path = new wchar_t[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	PathRemoveFileSpecW(path);
	PathCchCombine(path, MAX_PATH, path, (PCWSTR)(std::wstring(filename.begin(), filename.end()).c_str()));
	return path;
}

HRESULT AppCore::ExtractRessource(INT id, LPWSTR name, LPWSTR extractpath)
{
	HRSRC hRes = FindResource(this->hInstance, MAKEINTRESOURCE(id), name);
	if (hRes)
	{
		HGLOBAL hMem = LoadResource(this->hInstance, hRes);
		DWORD size = SizeofResource(this->hInstance, hRes);

		if (hMem)
		{
			char* resText = reinterpret_cast<char*>(LockResource(hMem));
			
			std::ofstream outfile(extractpath, std::ios::out | std::ios::binary);
			if (outfile.is_open() && outfile.good())
			{
				outfile.write(resText, size+1);
			}
			
			outfile.close();
			
			FreeResource(hMem);
			return S_OK;
		}
	}

	return E_FAIL;
}


HRESULT AppCore::UnZip(LPWSTR path, std::string dir)
{
	std::wstring tmp(path);
	std::string from(tmp.begin(), tmp.end());

	struct zip* za;
	struct zip_file* zf;
	struct zip_stat sb;

	char buf[100];

	int err;
	int i, len;
	int fd;
	long long sum;

	if ((za = zip_open(from.c_str(), 0, &err)) == NULL) {
		return E_FAIL;
	}

	for (i = 0; i < zip_get_num_entries(za, 0); i++) {
		if (zip_stat_index(za, i, 0, &sb) == 0) {
			std::string sbname(sb.name);
			for (size_t k = 0; k < sbname.length(); k++)
			{
				if (sbname[k] == '/')
				{
					sbname[k] = '\\';
				}
			}

			std::wstring wfname = this->PathCombineModuleFileName(dir + "\\" + sbname);
			std::string fname(wfname.begin(), wfname.end());

			if (sbname[sbname.length() - 1] == '\\')
			{
				std::string tmpname = "";
				for (size_t k = 0; k < sbname.length(); k++)
				{
					if (sbname[k] == '\\' && tmpname.length() != 0)
					{
						LPWSTR dirpath = this->PathCombineModuleFileName(dir + "\\" + tmpname);
						if (!this->DirectoryExists(dirpath))
						{
							if (!CreateDirectoryW(dirpath, NULL))
							{
								return E_FAIL;
							}
						}
					}
					tmpname += sbname[k];
				}
			}
			else {
				zf = zip_fopen_index(za, i, 0);
				if (!zf) {
					return E_FAIL;
				}


				fd = _open(fname.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
				if (fd < 0) {
					return E_FAIL;
				}

				sum = 0;
				while (sum != sb.size) {
					len = zip_fread(zf, buf, 100);
					if (len < 0) {
						return E_FAIL;
					}
					_write(fd, buf, len);
					sum += len;
				}
				_close(fd);
				zip_fclose(zf);
			}
		}
	}

	if (zip_close(za) == -1) {
		return E_FAIL;
	}

	return S_OK;
}

BOOL AppCore::DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
