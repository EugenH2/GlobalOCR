#pragma once
#include "resource.h"

#define MAX_LOADSTRING 100


DWORD WINAPI AlphaThread(LPVOID);

// Globale Variablen:
HANDLE h_tAlpha;
bool bAThreadSuspended = 0, bAThreadSwitchedOff = 0;

HDC hdc, hdcLayeredWnd;
HINSTANCE hInst;                                // Aktuelle Instanz
WCHAR szTitle[MAX_LOADSTRING];                  // Titelleistentext
WCHAR szWindowClass[MAX_LOADSTRING];            // Der Klassenname des Hauptfensters.
int wDesktopResolution = GetSystemMetrics(SM_CXVIRTUALSCREEN);
int hDesktopResolution = GetSystemMetrics(SM_CYVIRTUALSCREEN);
std::string recognizedWords;

// Vorwärtsdeklarationen der in diesem Codemodul enthaltenen Funktionen:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    LayeredWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



//
//  FUNKTION: MyRegisterClass()
//
//  ZWECK: Registriert die Fensterklasse.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GLOBALOCR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GLOBALOCR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


//
//   FUNKTION: InitInstance(HINSTANCE, int)
//
//   ZWECK: Speichert das Instanzenhandle und erstellt das Hauptfenster.
//
//   KOMMENTARE:
//
//        In dieser Funktion wird das Instanzenhandle in einer globalen Variablen gespeichert, und das
//        Hauptprogrammfenster wird erstellt und angezeigt.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    LoadLibrary(TEXT("Msftedit.dll"));

    //Create Main Window
    hInst = hInstance; // Instanzenhandle in der globalen Variablen speichern

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_ICONIC,
        CW_USEDEFAULT, 0, 1225, 675, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }


    //Create RICHEDIT50W for instruction 
    HWND hwndEdit = CreateWindowEx(0, L"RICHEDIT50W", L" The application needs the following three files:\n\
 1. Download the Text Detection Model from here: https://drive.google.com/file/d/1vY_KsDZZZb_svd5RT6pjyI8BS1nPbBSX/view and add it to the folder ModelDetection:\n\
 2. Download the Text Recognition Model from here: https://drive.google.com/file/d/1JPIhUeXldbUGrTuYt_Y2_MuUyQU7bDZ2/view?usp=sharing and add it to the folder ModelRecognition:\n\
 3. Download the Vocabulary File from here: https://drive.google.com/uc?export=dowload&id=1oPOYx5rQRp8L6XQciUwmwhMCfX0KyO4b and add it to the folder ModelRecognition:\n\
 4. Restart the application.\n\n\
 The Textdetection Model is for locating Words. Then the Textrecognizion Model uses the located words and recognizes the letters of the words.\n\
 And the Vocabulary is for encoding the classifications, which stand for a letter representation.\n\n\n\n\n\n\n\n\n\
 Hotkeys:\n F2: for pausing/resuming the application\n\
 F3: for copying to clipboard, from where it can be inserted in another application e.g. with \"Ctrl/Strg + v\" or \"right mouse-click + paste\" in notepad\n\
 F4: for switching off/on the application\n\n\n\n\
 More Text Detection Models can be found here: https://drive.google.com/drive/folders/1T9n0HTP3X3Y_nJ0D1ekMhCQRHntORLJG \n\
 More Text Recognition Models can be found here: https://drive.google.com/drive/folders/1cTbQ3nuZG-EKWak6emD_s8_hHXWz7lAr \n\
 Not all models can be used, though.",
        ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_READONLY, //WS_BORDER
        0, 0, 1100, 550,
        hWnd, NULL, hInstance, NULL);

    //symlink behavior 
    LRESULT mask = SendMessage(hwndEdit, EM_GETEVENTMASK, 0, 0);
    SendMessage(hwndEdit, EM_SETEVENTMASK, 0, mask | ENM_LINK);
    ::SendMessage(hwndEdit, EM_AUTOURLDETECT, TRUE, NULL);


    //Create 2 buttons for open folder of the instrunction
    HWND hwndOpenFileButton1 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Open Folder",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_TEXT | WS_OVERLAPPED | BS_DEFPUSHBUTTON,  // Styles 
        1100,         // x position 
        14,         // y position 
        78,        // Button width
        18,        // Button height
        hWnd,     // Parent window
        (HMENU)IDB_OPENFILE1,//200       
        hInst,
        NULL);

    HFONT hFont = CreateFont(14, 5, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, //Change button font
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    SendMessage(hwndOpenFileButton1, WM_SETFONT, (WPARAM)hFont, TRUE);


    HWND hwndOpenFileButton2 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Open Folder",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_TEXT | WS_OVERLAPPED | BS_DEFPUSHBUTTON,  // Styles 
        1100,         // x position 
        35,         // y position 
        78,        // Button width
        33,        // Button height
        hWnd,     // Parent window
        (HMENU)IDB_OPENFILE2,//200       
        hInst,
        NULL);

    hFont = CreateFont(14, 5, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    SendMessage(hwndOpenFileButton2, WM_SETFONT, (WPARAM)hFont, TRUE);


    ShowWindow(hWnd, SW_SHOWMINIMIZED);
    UpdateWindow(hWnd);


    //Create extra LayeredWindow (TRANSPARENT EXCLUDEFROMCAPTURE TOPMOST):
    WNDCLASSEX windowclassforwindow2;
    ZeroMemory(&windowclassforwindow2, sizeof(WNDCLASSEX));
    windowclassforwindow2.cbClsExtra = NULL;
    windowclassforwindow2.cbSize = sizeof(WNDCLASSEX);
    windowclassforwindow2.cbWndExtra = NULL;
    windowclassforwindow2.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    windowclassforwindow2.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowclassforwindow2.hIcon = NULL;
    windowclassforwindow2.hIconSm = NULL;
    windowclassforwindow2.hInstance = hInst;
    windowclassforwindow2.lpfnWndProc = (WNDPROC)LayeredWndProc;
    windowclassforwindow2.lpszClassName = L"window class2";
    windowclassforwindow2.lpszMenuName = NULL;
    windowclassforwindow2.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&windowclassforwindow2))
    {
        int nResult = GetLastError();
        MessageBox(NULL,
            L"LayeredWindow class creation failed for main window. Application won't work for that operation system.",
            L"LayeredWindow Class Failed.",
            MB_ICONERROR | MB_SETFOREGROUND);
    }

    HWND hWndLayered = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        windowclassforwindow2.lpszClassName,
        L"Layered Window",
        WS_POPUP | WS_VISIBLE,
        0,
        0,
        wDesktopResolution,
        hDesktopResolution,
        NULL,
        NULL,
        hInst,
        NULL);

    SetLayeredWindowAttributes(hWndLayered, RGB(0, 0, 0), 0, LWA_COLORKEY);
    SetWindowDisplayAffinity(hWndLayered, 0x00000011); // = WDA_EXCLUDEFROMCAPTURE

    if (!hWndLayered)
    {
        int nResult = GetLastError();

        MessageBox(NULL,
            L"LayeredWindow class creation failed for main window. Application won't work for that operation system.",
            L"LayeredWindow Class Failed.",
            MB_ICONERROR | MB_SETFOREGROUND);
    }

    //ShowWindow(hWndLayered, nCmdShow);
    //UpdateWindow(hWndLayered);

    return TRUE;
}

//
//  FUNKTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ZWECK: Verarbeitet Meldungen für das Hauptfenster.
//
//  WM_COMMAND  - Verarbeiten des Anwendungsmenüs
//  WM_PAINT    - Darstellen des Hauptfensters
//  WM_DESTROY  - Ausgeben einer Beendenmeldung und zurückkehren
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {

        case EN_LINK: //Process Hyperlinks
        {
            ENLINK* enLinkInfo = (ENLINK*)lParam;
            if (enLinkInfo->msg == WM_LBUTTONUP)
            {
                TEXTRANGEW tr;
                tr.chrg = ((ENLINK*)lParam)->chrg;
                tr.lpstrText = new wchar_t[(tr.chrg.cpMax - tr.chrg.cpMin) + 1];

                //Select copy all the text from enLinkInfo->chrg.cpMin to enLinkInfo->chrg.cpMax
                SendMessageW(enLinkInfo->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

                //Lauch the url
                ShellExecute(NULL, L"open", tr.lpstrText, NULL, NULL, SW_SHOW);

                delete[] tr.lpstrText;
            }
            break;
        }

        return 0;
        }

        return 0;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Menüauswahl analysieren:
        switch (wmId)
        {
        case IDB_OPENFILE1: //Open file Button 1
        {
            ShellExecute(NULL, NULL, L"ModelDetection", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        case IDB_OPENFILE2: //Open file Button 2
        {
            ShellExecute(NULL, NULL, L"ModelRecognition", NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:
    {
        hdc = GetDC(hWnd);

        // Create thread for textdetection and recognition
        DWORD dwAlphaID;
        h_tAlpha = CreateThread(NULL, 0, AlphaThread, (LPVOID)1, 0,
            &dwAlphaID);

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        // TODO: Zeichencode, der hdc verwendet, hier einfügen...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        ReleaseDC(hWnd, hdc);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Meldungshandler für Infofeld.
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
    }
    return (INT_PTR)FALSE;
}


LRESULT CALLBACK LayeredWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message)
    {
    case WM_HOTKEY:
    {
        switch (wParam)
        {

        case 2://F2 Hotkey for pausing/resuming thread
        {
            if (!bAThreadSuspended)
            {
                SuspendThread(h_tAlpha);
                bAThreadSuspended = 1;
            }
            else
            {
                ResumeThread(h_tAlpha);
                bAThreadSuspended = 0;
            }

            break;
        }
        case 3://F3 Hotkey for copy to Clipboard
        {
            OpenClipboard(hwnd);
            EmptyClipboard();
            HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, recognizedWords.size() + 1);
            if (!hg) {
                CloseClipboard();
                return 1;
            }
            memcpy(GlobalLock(hg), recognizedWords.c_str(), recognizedWords.size() + 1);
            GlobalUnlock(hg);
            SetClipboardData(CF_TEXT, hg);
            CloseClipboard();
            GlobalFree(hg);

            break;
        }
        case 4://F4 Hokey for Switch off/on Thread
        {
            if (!bAThreadSwitchedOff)
            {
                bAThreadSwitchedOff = 1;//Let the Alphathread end himself
                if (bAThreadSuspended)
                {
                    ResumeThread(h_tAlpha);
                    bAThreadSuspended = 0;
                }

                ShowWindow(hwnd, SW_HIDE); //Hide Layeredwindow
            }
            else
            {
                if (!h_tAlpha) //Create only one Alphathread
                {
                    bAThreadSwitchedOff = 0;
                    h_tAlpha = CreateThread(NULL, 0, AlphaThread, (LPVOID)1, 0, NULL);
                    ShowWindow(hwnd, SW_SHOWNORMAL);

                    if (bAThreadSuspended) //That bool is independent and could be still 1
                    {
                        ResumeThread(h_tAlpha);
                        bAThreadSuspended = 0;
                    }
                }
            }

            break;
        }

        }
        break;
    }
    case WM_CREATE:
    {
        //Register F2,F3,F4 hotkeys
        RegisterHotKey(hwnd, 2, 0x0000, VK_F2);
        RegisterHotKey(hwnd, 3, 0x0000, VK_F3);
        RegisterHotKey(hwnd, 4, 0x0000, VK_F4);

        hdcLayeredWnd = GetDC(hwnd);

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        // TODO: Zeichencode, der hdc verwendet, hier einfügen...
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_CLOSE:
    {

        break;
    }
    case WM_DESTROY:
    {
        ReleaseDC(hwnd, hdcLayeredWnd);
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, Message, wParam, lParam);
    }

    return 0;
}
