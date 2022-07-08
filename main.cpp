#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>

#include "resource.h"

HWND g_hwnd;
HWND g_hEdit;
HFONT g_hFont;
COLORREF g_editcolor = RGB(0, 0, 0);
HBRUSH g_hbrbackground = CreateSolidBrush(RGB(255, 255, 255));

char currfile[MAX_PATH];
bool file_isOpened = false;
bool file_saveRequired = false;

void LoadFile();
void SaveFile();
bool GetFileNameSave();
void CheckSaveStatus();
void ChooseFontDialog();

// called by DispatchMessage()
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lParam);

// convert class name into a global variable
char szClassName[] = "texteditor_class";
char strTitle[] = "texteditor 0.1 [Untitled]";

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
	HWND hwnd;
	MSG msg; // message to the app are saved here
	WNDCLASSEX wincl; // data structure for the window class

	// window structure
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_HREDRAW | CS_VREDRAW;
	wincl.cbSize = sizeof(WNDCLASSEX);

	// use default icon and mouse pointer
	wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WIN));
	wincl.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TITLE));
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = MAKEINTRESOURCE(IDC_MENU);
	wincl.cbClsExtra = 0; // remove any extra bytes from the window class
	wincl.cbWndExtra = 0; // remove any extra bytes from the window structure or the window instance
	wincl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	// register the window class, if fails: quit
	if (!RegisterClassEx(&wincl)) return 0;

	// create the window if the class was registered
	hwnd = CreateWindowEx(
		NULL,
		szClassName,
		"texteditor 0.1 [Untitled]",
		WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW, // default window
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1280, // width
		720, // height
		NULL, // is a child window
		0,
		hThisInstance, // instance handler
		NULL
	);
	if (!hwnd)
	{
		MessageBox(NULL, "Window creation failed.\nExiting the program.", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	g_hwnd = hwnd;
	ShowWindow(hwnd, nCmdShow);

	HACCEL hAccel = LoadAccelerators(hThisInstance, MAKEINTRESOURCE(IDC_ACCEL));
	bool done = false;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) { done = true; }
			else if (!TranslateAccelerator(g_hwnd, hAccel, &msg)) { TranslateMessage(&msg); DispatchMessage(&msg); }
		}
	}
	return msg.wParam;
}

void LoadFile()
{
	currfile[0] = '\0';
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hwnd;
	ofn.lpstrFilter = "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
	ofn.lpstrFile = currfile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "txt";
	if (!GetOpenFileName(&ofn)) return;
	HANDLE hFile;
	bool bSuccess = false;
	hFile = CreateFile(currfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != 0xFFFFFFFF)
		{
			LPSTR tempftext;
			tempftext = (char*)GlobalAlloc(GPTR, dwFileSize + 1);
			if (tempftext != NULL)
			{
				DWORD dwRead;
				if (ReadFile(hFile, tempftext, dwFileSize, &dwRead, NULL))
				{
					tempftext[dwFileSize] = 0;
					if (SetWindowText(g_hEdit, tempftext)) bSuccess = true;
				}
				GlobalFree(tempftext);
			}
		}
		CloseHandle(hFile);
	}
	if (!bSuccess)
	{
		MessageBox(g_hwnd, "Failed to load the selected file.", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	SetWindowText(g_hwnd, currfile);
	file_saveRequired = false;
	file_isOpened = true;
}

void SaveFile()
{
	HANDLE hFile = CreateFile(currfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	bool bSuccess = false;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength = GetWindowTextLength(g_hEdit);
		LPSTR pszText;
		DWORD dwBufferSize = dwTextLength + 1;
		pszText = (char*)GlobalAlloc(GPTR, dwBufferSize);
		if (pszText != NULL)
		{
			if (GetWindowText(g_hEdit, pszText, dwBufferSize))
			{
				DWORD dwWritten;
				if (WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL)) bSuccess = true;
			}
			GlobalFree(pszText);
		}
		CloseHandle(hFile);
	}
	if (!bSuccess)
	{
		MessageBox(g_hwnd, "Failed to save the file.", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	file_saveRequired = false;
	file_isOpened = true;
}

bool GetFileNameSave()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hwnd;
	ofn.lpstrFilter = "Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
	ofn.lpstrFile = currfile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "txt";
	if (!GetOpenFileName(&ofn)) return false;
	return true;
}

void CheckSaveStatus()
{
	if(file_saveRequired)
	{
		int res = MessageBox(g_hwnd, "The file has been modified.\nDo you want to save it before continuing?", "File unsaved!", MB_YESNOCANCEL | MB_ICONINFORMATION);
		if (res == IDCANCEL) return;
		if (GetFileNameSave()) SaveFile();
	}
}

void ChooseFontDialog()
{
	CHOOSEFONT cf = {sizeof(CHOOSEFONT)};
	LOGFONT lf;
	GetObject(g_hFont, sizeof(LOGFONT), &lf);
	cf.Flags = CF_EFFECTS | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	cf.hwndOwner = g_hwnd;
	cf.lpLogFont = &lf;
	cf.rgbColors = g_editcolor;
	if (!ChooseFont(&cf)) return;
	HFONT hf = CreateFontIndirect(&lf);
	if (hf)
	{
		g_hFont = hf;
		SendMessage(g_hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
	}
	g_editcolor = cf.rgbColors;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			HWND hEdit;
			HFONT hFont;
			hEdit = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"Edit",
				"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
				1,
				1,
				100,
				100,
				hwnd,
				(HMENU)IDC_EDIT,
				GetModuleHandle(NULL),
				NULL
			);
			if (hEdit == NULL)
			{
				MessageBox(g_hwnd, "Edit control creation failed!", "Error", MB_OK | MB_ICONERROR);
				PostQuitMessage(0);
			}

			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			g_hEdit = hEdit;
			g_hFont = hFont;

			RECT rcClient;
			GetClientRect(g_hwnd, &rcClient);
			SetWindowPos(g_hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
			break;
		}

		case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(g_hwnd, &rcClient);
			SetWindowPos(g_hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
			break;
		}

		case WM_CLOSE:
		{
			if (file_saveRequired)
			{
				int res = MessageBox(g_hwnd, "The file has been modified.\nDo you want to save it before exiting?", "File unsaved!", MB_YESNOCANCEL | MB_ICONINFORMATION);
				if (res == IDCANCEL) return 0;
				if (res == IDYES)
					if (GetFileNameSave()) SaveFile();
			}
			if (MessageBox(g_hwnd, "Are you sure want to exit?", "Confirm exit", MB_YESNO | MB_ICONQUESTION) == IDNO) return 0;
			PostQuitMessage(0);
			break;
		}

		case WM_CTLCOLOREDIT:
		{
			HDC hdcedit = (HDC)wParam;
			SetTextColor(hdcedit, g_editcolor);
			SetBkMode(hdcedit, TRANSPARENT);
			//return (LONG)g_hbrbackground;
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				// file menu
				case IDM_FILENEW:
				{
					CheckSaveStatus();
					file_isOpened = false;
					file_saveRequired = false;
					SetWindowText(g_hEdit, "");
					SetWindowText(g_hwnd, "texteditor 0.1 [Untitled]");
					break;
				}

				case IDM_FILEOPEN:
				{
					CheckSaveStatus();
					LoadFile();
					break;
				}

				case IDM_FILESAVE:
				{
					if (file_saveRequired)
					{
						if (file_isOpened) SaveFile();
						else if (GetFileNameSave()) SaveFile();
					}
					break;
				}

				case IDM_FILESAVEAS:
				{
					if (GetFileNameSave()) SaveFile();
					break;
				}

				case IDM_FILEEXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;

				// edit menu
				case IDM_EDITCUT:
					SendMessage(g_hEdit, WM_CUT, 0, 0);
					break;

				case IDM_EDITCOPY:
					SendMessage(g_hEdit, WM_COPY, 0, 0);
					break;

				case IDM_EDITPASTE:
					SendMessage(g_hEdit, WM_PASTE, 0, 0);
					break;

				// format menu
				case IDM_FORMATWORDWRAP:
					break;

				case IDM_FORMATFONT:
					ChooseFontDialog();
					break;

				// about menu
				case IDM_ABOUTWND:
					MessageBox(g_hwnd, "texteditor 0.1\n\nDeveloper: tomeczeklmaodev\nLicense: MIT Open Source License\nTechnologies used: C++, Win32 API", "About texteditor", MB_OK | MB_ICONINFORMATION);
					break;

				case IDM_ABOUTREPO:
					ShellExecute(0, 0, "https://github.com/tomeczeklmaodev/texteditor/", 0, 0, SW_SHOW); // open the repo website
					break;

				// other
				case IDC_EDIT:
				{
					switch(HIWORD(wParam))
					{
						case EN_CHANGE:
							file_saveRequired = true;
							break;
					}
					break;
				}
				break;
			}
		default:
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

// ShellExecute(0, 0, "https://github.com/tomeczeklmaodev/texteditor/", 0, 0, SW_SHOW); // open the repo website
