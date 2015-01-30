/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */

#if !defined __GUI_IMPL__H__
#define __GUI_IMPL_H__

/*
 *  Gui base class
 *
 */
#include <shellapi.h>

#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 100 )

#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif

class GuiImpl {
public:

	GuiImpl(): hDialog(NULL)
	{}
	HWND hDialog;
	NOTIFYICONDATA g_notifyIconData ;
	int id_icon;


	virtual ~GuiImpl () {}

public:
	static BOOL CALLBACK CtrlBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static BOOL CtrlBoxDlgProcOnInit(HWND hDlg, HWND hFocus, LPARAM lParam);

	static BOOL CtrlBoxDlgProcOnHScroll(
		HWND hWnd,         // hwnd
		HWND hwndCtl,      // (int)(LOWORD(wParam))
		UINT codeNotify,   // (UINT)HIWORD(wParam))
		int  pos
	);

	static BOOL CtrlBoxDlgProcOnCommand(
		HWND hWnd,         // hwnd
		int id,            // (int)(LOWORD(wParam))
		HWND hCtl,         // (HWND)(lParam)
		UINT codeNotify    // (UINT)HIWORD(wParam))
		);

	static BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lParam);
	

	void Minimize();
	void Restore();
	// Initialize the NOTIFYICONDATA structure.
	// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
	// for details on the NOTIFYICONDATA structure.
	void InitNotifyIconData (int ID_TRAY_APP_ICON, HICON hIcon, const char *tip);
};

#endif

