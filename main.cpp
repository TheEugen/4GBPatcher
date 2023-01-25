#include <windows.h>
#include <shobjidl.h> 

#include "resources.h"
#include "pathhandler.h"
#include "patch.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void toggleButtons(HWND hwnd, bool patchStatus)
{
	// find handle to patch button
	HWND hwndPatchBtn = GetDlgItem(hwnd, IDC_BUTTON_PATCH);

	// find handle to unpatch button
	HWND hwndUnpatchBtn = GetDlgItem(hwnd, IDC_BUTTON_UNPATCH);

	// toggle patch button
	EnableWindow(hwndPatchBtn, !patchStatus);

	// toggle unpatch button
	EnableWindow(hwndUnpatchBtn, patchStatus);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"MainWindow";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"4GB Patcher",    // Window text
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, 420, 175,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	// TODO catch
	if (hwnd == NULL)
	{
		return 0;
	}

	// textwindows
	CreateWindow(TEXT("static"), TEXT(""), WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | WS_BORDER, 10, 10, 290, 25, hwnd, (HMENU)IDC_TEXT_PATH, NULL, NULL);
	CreateWindow(TEXT("static"), TEXT("Choose file to view application status"), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE, 0, 50, 420, 25, hwnd, (HMENU)IDC_TEXT_STATUS, NULL, NULL);

	// buttons
	CreateWindow(TEXT("button"), TEXT(BTN_TEXT_CHOOSE), WS_VISIBLE | WS_CHILD | WS_BORDER, 310, 10, 80, 25, hwnd, (HMENU)IDC_BUTTON_CHOOSEFILE, NULL, NULL);
	CreateWindow(TEXT("button"), TEXT(BTN_TEXT_CLOSE), WS_VISIBLE | WS_CHILD | WS_BORDER, 290, 100, 100, 25, hwnd, (HMENU)IDC_BUTTON_CLOSE, NULL, NULL);
	// create buttons and disable until file is chosen
	EnableWindow(CreateWindow(TEXT("button"), TEXT(BTN_TEXT_PATCH), WS_VISIBLE | WS_CHILD | WS_BORDER, 10, 100, 100, 25, hwnd, (HMENU)IDC_BUTTON_PATCH, NULL, NULL), false);
	EnableWindow(CreateWindow(TEXT("button"), TEXT(BTN_TEXT_UNPATCH), WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 100, 100, 25, hwnd, (HMENU)IDC_BUTTON_UNPATCH, NULL, NULL), false);
	
	// init pathhandler
	PathHandler* pathhandler = NULL;

	// give main window reference to pathhandler
	SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG)pathhandler);

	// show window
	ShowWindow(hwnd, nCmdShow);

	// message loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
	{
		// get pathhandler reference
		PathHandler* pathhandler = (PathHandler*)GetWindowLongPtr(hwnd, GWL_USERDATA);

		if(LOWORD(wParam) == IDC_BUTTON_CHOOSEFILE)
		{
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
				COINIT_DISABLE_OLE1DDE);

			if (SUCCEEDED(hr))
			{
				IFileOpenDialog* pFileOpen;

				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
					IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

				// filter for openfile dialog, only .exe should be used
				COMDLG_FILTERSPEC filterspec[1] = {{L"Executable Files", L"*.exe"}};
				pFileOpen->SetFileTypes(1, filterspec);

				if (SUCCEEDED(hr))
				{
					// Show the Open dialog box.
					hr = pFileOpen->Show(NULL);

					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem* pItem;
						hr = pFileOpen->GetResult(&pItem);

						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

							if (SUCCEEDED(hr))
							{
								// if pathhandler is null (first time choosing) create new with filepath
								if (!pathhandler)
									pathhandler = new PathHandler(pszFilePath);

								if (detectPatchStatus(pathhandler->getPath(), hwnd))
								{
									OutputDebugStringW(TEXT("status: patched\n"));

									// create message for output window
									std::wstring appStatus = OUT_TEXT_APPSTATUS_PATCHED;
									LPCWSTR lpAppStatus = appStatus.c_str();

									// find handle to patchstatus window
									HWND hwndPatchStatus = GetDlgItem(hwnd, IDC_TEXT_STATUS);

									// send appstatus to output window
									SendMessage(hwndPatchStatus, WM_SETTEXT, NULL, (LPARAM)lpAppStatus);

									// find handle to textpath window
									HWND hwndFilepath = GetDlgItem(hwnd, IDC_TEXT_PATH);

									// send file path to output window
									SendMessage(hwndFilepath, WM_SETTEXT, NULL, (LPARAM)pathhandler->getPath().c_str());

									// button control
									toggleButtons(hwnd, true);

								}
								else
								{
									OutputDebugStringW(TEXT("status: unpatched\n"));

									// create message for output window
									std::wstring appStatus = OUT_TEXT_APPSTATUS_UNPATCHED;
									LPCWSTR lpAppStatus = appStatus.c_str();

									// find status output window handle
									HWND hwndPatchStatus = GetDlgItem(hwnd, IDC_TEXT_STATUS);

									// send the appstatus to output window
									SendMessage(hwndPatchStatus, WM_SETTEXT, NULL, (LPARAM)lpAppStatus);

									// find path output window handle
									HWND hwndFilepath = GetDlgItem(hwnd, IDC_TEXT_PATH);

									// send the filepath to output window
									SendMessage(hwndFilepath, WM_SETTEXT, NULL, (LPARAM)pathhandler->getPath().c_str());

									// button control
									toggleButtons(hwnd, false);
								}
							}
							// ?
							CoTaskMemFree(pszFilePath);
							pItem->Release();
						}
					}
					pFileOpen->Release();
				}
				CoUninitialize();
			}
		}
		else if ((LOWORD(wParam) == IDC_BUTTON_PATCH) || (LOWORD(wParam) == IDC_BUTTON_UNPATCH))
		{
			// debug
			OutputDebugStringW(TEXT("received patch btn click"));

			// patch the file
			if (patchFile(pathhandler->getPath(), (LOWORD(wParam) == IDC_BUTTON_PATCH), hwnd))
			{
				// create message for status output window
				std::wstring output_statusW;
				LPCWSTR messageBoxWStr;

				if (LOWORD(wParam) == IDC_BUTTON_PATCH)
				{
					output_statusW = OUT_TEXT_APPSTATUS_PATCHED;
					messageBoxWStr = L"Application successfully patched!";
					toggleButtons(hwnd, true);
				}
				else
				{
					output_statusW = OUT_TEXT_APPSTATUS_UNPATCHED;
					messageBoxWStr = L"Application successfully unpatched!";
					toggleButtons(hwnd, false);
				}

				LPCWSTR output_statusWLP = output_statusW.c_str();

				// find output window handle
				HWND hwndPatchStatus = (GetDlgItem(hwnd, IDC_TEXT_STATUS));

				// send appstatus to output window
				SendMessage(hwndPatchStatus, WM_SETTEXT, NULL, (LPARAM)output_statusWLP);

				// show message box to user
				MessageBox(hwnd, messageBoxWStr, L"4GB Patcher", MB_OK);
			}
			
		}
		else if (LOWORD(wParam) == IDC_BUTTON_CLOSE)
		{
			PostQuitMessage(0);
		}
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;

		// make text black
		SetTextColor(hdcStatic, RGB(0, 0, 0));

		// make background white
		SetBkColor(hdcStatic, RGB(255, 255, 255));

		HBRUSH hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
		return (INT_PTR)hbrBkgnd;
	}

	return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
