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
		APP_NAME,    // Window text
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_SIZE_X, WINDOW_SIZE_Y,

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

	// get primary display resolution and place window in the middle of the workarea (screen without taskbar)
	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (void*)&rc, 0);
	int xPos = rc.right / 2 - WINDOW_SIZE_X / 2;
	int yPos = rc.bottom / 2 - WINDOW_SIZE_Y / 2;
	SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

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
	PathHandler* pathhandler = new PathHandler();

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
	{
		PostQuitMessage(0);

		// return 0 means we have successfully handled a message
		return 0;
	}
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
				// TODO: dll files?
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
								pathhandler->setPath(pszFilePath);

								const unsigned int patchStatus = detectPatchStatus(pathhandler->getPath(), hwnd);
								
								if (patchStatus > 0)
								{
									// create message for output window
									LPCWSTR appStatus;

									// find handle to patchstatus window
									HWND hwndPatchStatus = GetDlgItem(hwnd, IDC_TEXT_STATUS);

									// find handle to filepath window
									HWND hwndFilepath = GetDlgItem(hwnd, IDC_TEXT_PATH);

									if (patchStatus == 1)
									{
										OutputDebugStringW(TEXT("status: patched\n"));

										appStatus = OUT_TEXT_APPSTATUS_PATCHED;

										// button control
										toggleButtons(hwnd, true);
									}
									else
									{
										OutputDebugStringW(TEXT("status: unpatched\n"));

										appStatus = OUT_TEXT_APPSTATUS_UNPATCHED;

										// button control
										toggleButtons(hwnd, false);
									}

									// send appstatus to output window
									SendMessage(hwndPatchStatus, WM_SETTEXT, NULL, (LPARAM)appStatus);

									// send file path to output window
									SendMessage(hwndFilepath, WM_SETTEXT, NULL, (LPARAM)pathhandler->getPath().c_str());
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
			if (LOWORD(wParam) == IDC_BUTTON_PATCH)
				OutputDebugStringW(TEXT("received patch btn click"));
			else
				OutputDebugStringW(TEXT("received unpatch btn click"));

			// patch the file
			if (patchFile(pathhandler->getPath(), (LOWORD(wParam) == IDC_BUTTON_PATCH), hwnd))
			{
				// find output window handle
				HWND hwndPatchStatus = (GetDlgItem(hwnd, IDC_TEXT_STATUS));

				// create message for status output window
				LPCWSTR outputStatusW;
				LPCWSTR messageBoxWStr;

				if (LOWORD(wParam) == IDC_BUTTON_PATCH)
				{
					outputStatusW = OUT_TEXT_APPSTATUS_PATCHED;
					messageBoxWStr = OUT_TEXT_MB_PATCHED;
					toggleButtons(hwnd, true);				
				}
				else
				{
					outputStatusW = OUT_TEXT_APPSTATUS_UNPATCHED;
					messageBoxWStr = OUT_TEXT_MB_UNPATCHED;
					toggleButtons(hwnd, false);
				}

				// send appstatus to output window
				SendMessage(hwndPatchStatus, WM_SETTEXT, NULL, (LPARAM)outputStatusW);

				// show "successfully un/patched!" message box to user
				MessageBox(hwnd, messageBoxWStr, APP_NAME, MB_OK);
			}
			
		}
		else if (LOWORD(wParam) == IDC_BUTTON_CLOSE)
		{
			PostQuitMessage(0);
		}

		return 0;
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

	// called when didn't processed any message
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
