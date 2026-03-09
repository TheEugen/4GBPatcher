#include "4GBPatcher.h"


#define MAX_LOADSTRING 100

// globals
HINSTANCE hInst;                                // Aktuelle Instanz
WCHAR szTitle[MAX_LOADSTRING];                  // Titelleistentext
WCHAR szWindowClass[MAX_LOADSTRING];            // Der Klassenname des Hauptfensters.
HWND hWndMain;

// forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // Globale Zeichenfolgen initialisieren
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY4GBPATCHER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // init main window
    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY4GBPATCHER));

    MSG msg;

    auto pathHandler = std::make_unique<PathHandler>();
    // give main window reference to pathhandler
    // info: for x64 must be GWLP_USERDATA
    SetWindowLongPtr(hWndMain, GWL_USERDATA, (LONG)pathHandler.get());

    // message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY4GBPATCHER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY4GBPATCHER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Instanzenhandle in der globalen Variablen speichern
    
    hWndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_SIZE_X, WINDOW_SIZE_Y, nullptr, nullptr, hInstance, nullptr);
    
    if (!hWndMain)
        return false;
    
    /// 
     
     // get primary display resolution and place window in the middle of the workarea (screen without taskbar)
    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, (void*)&rc, 0);
    int xPos = rc.right / 2 - WINDOW_SIZE_X / 2;
    int yPos = rc.bottom / 2 - WINDOW_SIZE_Y / 2;
    SetWindowPos(hWndMain, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    
    // textwindows
    CreateWindow(TEXT("static"), TEXT(""), WS_VISIBLE | WS_CHILD | SS_CENTERIMAGE | WS_BORDER, 10, 10, 290, 25, hWndMain, (HMENU)IDC_TEXT_PATH, nullptr, nullptr);
    CreateWindow(TEXT("static"), TEXT("Choose file to view application status"), WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE, 0, 50, 420, 25, hWndMain, (HMENU)IDC_TEXT_STATUS, nullptr, nullptr);
    
    // buttons
    CreateWindow(TEXT("button"), TEXT(BTN_TEXT_CHOOSE), WS_VISIBLE | WS_CHILD | WS_BORDER, 310, 10, 80, 25, hWndMain, (HMENU)IDC_BUTTON_CHOOSEFILE, nullptr, nullptr);
    CreateWindow(TEXT("button"), TEXT(BTN_TEXT_CLOSE), WS_VISIBLE | WS_CHILD | WS_BORDER, 290, 100, 100, 25, hWndMain, (HMENU)IDC_BUTTON_CLOSE, nullptr, nullptr);
    // create buttons and disable until file is chosen
    CreateWindow(TEXT("button"), TEXT(BTN_TEXT_PATCH), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED, 10, 100, 100, 25, hWndMain, (HMENU)IDC_BUTTON_PATCH, nullptr, nullptr);
    CreateWindow(TEXT("button"), TEXT(BTN_TEXT_UNPATCH), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED, 150, 100, 100, 25, hWndMain, (HMENU)IDC_BUTTON_UNPATCH, nullptr, nullptr);
    
    ShowWindow(hWndMain, nCmdShow);
    UpdateWindow(hWndMain);
    
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // get pathHandler reference
    auto* pathHandler = (PathHandler*)GetWindowLongPtr(hWnd, GWL_USERDATA);

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Menüauswahl analysieren:
            switch (wmId)
            {
            case IDC_BUTTON_CHOOSEFILE:
            {
                HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED |
                    COINIT_DISABLE_OLE1DDE);

                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    // Create the FileOpenDialog object.
                    //hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                    //    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
                    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                        IID_IFileOpenDialog, std::_Bit_cast<void**>(&pFileOpen));

                    // filter for openfile dialog, only .exe should be used
                    // TODO: dll files?
                   std::vector<COMDLG_FILTERSPEC> filterspec = { {L"Executable Files", L"*.exe"} };
                    pFileOpen->SetFileTypes(filterspec.size(), filterspec.data());

                    if (SUCCEEDED(hr))
                    {
                        // Show the Open dialog box.
                        hr = pFileOpen->Show(nullptr);

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
                                    pathHandler->setPath(pszFilePath);

                                    const unsigned int patchStatus = detectPatchStatus(pathHandler->getPath(), hWnd);

                                    if (patchStatus > 0)
                                    {
                                        // create message for output window
                                        LPCWSTR appStatus;

                                        // find handle to patchstatus window
                                        HWND hwndPatchStatus = GetDlgItem(hWnd, IDC_TEXT_STATUS);

                                        // find handle to filepath window
                                        HWND hwndFilepath = GetDlgItem(hWnd, IDC_TEXT_PATH);

                                        // TODO: implement patchStatus == 0
                                        if (patchStatus == 1)
                                        {
                                            OutputDebugStringW(L"status: patched\n");

                                            appStatus = OUT_TEXT_APPSTATUS_PATCHED;

                                            // button control
                                            toggleButtons(hWnd, true);
                                        }
                                        else
                                        {
                                            OutputDebugStringW(L"status: unpatched\n");

                                            appStatus = OUT_TEXT_APPSTATUS_UNPATCHED;

                                            // button control
                                            toggleButtons(hWnd, false);
                                        }

                                        // send appstatus to output window
                                        SendMessage(hwndPatchStatus, WM_SETTEXT, 0, (LPARAM)appStatus);

                                        // send file path to output window
                                        SendMessage(hwndFilepath, WM_SETTEXT, 0, (LPARAM)pathHandler->getPath().c_str());
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
                break;
            }
            case IDC_BUTTON_PATCH:
            {
                // patch the file
                if (patchFile(pathHandler->getPath(), true, hWnd))
                {
                    // find output window handle
                    HWND hWndPatchStatus = (GetDlgItem(hWnd, IDC_TEXT_STATUS));

                    // create message for status output window
                    LPCWSTR outputStatusW = OUT_TEXT_APPSTATUS_PATCHED;
                    LPCWSTR messageBoxWStr = OUT_TEXT_MB_PATCHED;
                    toggleButtons(hWnd, true);

                    // send appstatus to output window
                    SendMessage(hWndPatchStatus, WM_SETTEXT, 0, (LPARAM)outputStatusW);

                    // show "successfully un/patched!" message box to user
                    MessageBox(hWnd, messageBoxWStr, APP_NAME, MB_OK);
                }

                break;
            }
            case IDC_BUTTON_UNPATCH:
            {
                // patch the file
                if (patchFile(pathHandler->getPath(), false, hWnd))
                {
                    // find output window handle
                    HWND hWndPatchStatus = (GetDlgItem(hWnd, IDC_TEXT_STATUS));

                    // create message for status output window
                    LPCWSTR outputStatusW = OUT_TEXT_APPSTATUS_UNPATCHED;
                    LPCWSTR messageBoxWStr = OUT_TEXT_MB_UNPATCHED;
                    toggleButtons(hWnd, false);

                    // send appstatus to output window
                    SendMessage(hWndPatchStatus, WM_SETTEXT, 0, (LPARAM)outputStatusW);

                    // show "successfully un/patched!" message box to user
                    MessageBox(hWnd, messageBoxWStr, APP_NAME, MB_OK);
                }

                break;
            }
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDC_BUTTON_CLOSE:
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
    {
        auto hdcStatic = (HDC)wParam;

        // make text black
        SetTextColor(hdcStatic, RGB(0, 0, 0));

        // make background white
        SetBkColor(hdcStatic, RGB(255, 255, 255));

        HBRUSH hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
        return (INT_PTR)hbrBkgnd;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        //HDC hdc = 
        BeginPaint(hWnd, &ps);
        
        EndPaint(hWnd, &ps);
    }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    default:
        break;
    }
    return (INT_PTR)FALSE;
}
