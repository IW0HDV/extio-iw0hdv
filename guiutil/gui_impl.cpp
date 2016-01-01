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

/**
* @file gui_impl.cpp
* @brief Header for Extio DLL GUI classes
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#if defined _MSC_VER || defined __MINGW32__
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#include <windowsx.h>
#include <commctrl.h>			// Include header
#pragma comment(lib, "comctl32.lib")	// Include library
#pragma warning( disable : 4995 )
#endif

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include "log.h"
#include "guievent.h"
#include "guiutil.h"
#include "gui_impl.h" 
#include "gui.h" 

static const char *buildString = " - " __DATE__ ", " __TIME__ " - "
#if defined _MSC_VER
"ms";
#elif defined __MINGW32__
"gcc";
#else
"";
#endif

const char *getBuildString ()
{
	return buildString;
}




//
// (hwnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L
//
BOOL GuiImpl::CtrlBoxDlgProcOnCommand (
	HWND hWnd,         // hwnd
	int id,            // (int)(LOWORD(wParam))
	HWND hCtl,         // (HWND)(lParam)
	UINT codeNotify    // (UINT)HIWORD(wParam))
)
{
	LOGT("%p\n", GetWindowLongPtr(hWnd, GWLP_USERDATA));
	Gui *pGui = (Gui *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pGui) {
		LOGT("********************************* Gui addr: %p pi: %p\n", pGui, pGui->pi);

		// some button/checkbox has been clicked
		// the lower word of wParam holds the controls ID
		if (codeNotify == BN_CLICKED) pGui->ButtonClick(GuiEvent(hWnd, id));
		if (codeNotify == LBN_DBLCLK) pGui->ListBoxDoubleClick(GuiEvent(hWnd, id));

		// CBN_SELCHANGE notification code
		// Sent when the user changes the current selection in the list box of a combo box. 
		// The user can change the selection by clicking in the list box or by using the arrow keys. 
		// The parent window of the combo box receives this notification code in the form of a WM_COMMAND message. 
		if (codeNotify == CBN_SELCHANGE) pGui->ComboBoxSelChange(GuiEvent(hWnd, id));
	}
	return TRUE;
}



BOOL GuiImpl::CtrlBoxDlgProcOnInit(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	LOGT("%08x\n", lParam);
	//Gui *pGui = (Gui *)lParam;
	LOGT("********************************* Gui addr: %p\n", (Gui *)lParam);

	return TRUE;
}


BOOL CALLBACK GuiImpl::CtrlBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL fProcessed = FALSE;
	Gui *pGui = (Gui *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	//LOGT("********************************* %u %ld %d Gui addr: %p\n", uMsg, wParam, lParam, pGui);

	switch (uMsg) {
		HANDLE_MSG(hDlg, WM_INITDIALOG, GuiImpl::CtrlBoxDlgProcOnInit);
		HANDLE_MSG(hDlg, WM_COMMAND, GuiImpl::CtrlBoxDlgProcOnCommand);
		HANDLE_MSG(hDlg, WM_HSCROLL, GuiImpl::CtrlBoxDlgProcOnHScroll);
	
	case WM_SYSCOMMAND:
		switch( wParam & 0xfff0 ) { // (filter out reserved lower 4 bits:  see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
			case SC_MINIMIZE:
				LOGT( "%s\n", "Got an actual SC_MINIMIZE Message!" ) ;
			case SC_CLOSE:  // redundant to SC_CLOSE, it appears
				LOGT( "%s\n", "Got an actual WM_MINIMIZE Message!" ) ;
				pGui->pi->Minimize() ; 
				return TRUE ;  // avoid default minimize process
			break;
		}
		break;
		
	case WM_CLOSE:
		LOGT( "%s\n", "Got an actual WM_CLOSE Message!" ) ;
		pGui->pi->Minimize() ;
		return 0;
		break;

	default:
		// process WM_USER messages avoiding to be fooled from WM_APP
		if (uMsg > WM_USER && uMsg < WM_APP) {
		
			// Our user defined WM_TRAYICON message.
			// We made this message up, and we told
			// 
			if (uMsg == WM_TRAYICON ) {
				LOGT( "Tray icon notification, from %d\n", wParam ) ;
    
				//if ( wParam ==  pGui->pi->id_icon )
				//	printf( "Its the ID_TRAY_APP_ICON.. one app can have several tray icons, ya know..\n" ) ;
				//	break;
				//}

				// the mouse button has been released.
    
				// I'd LIKE TO do this on WM_LBUTTONDOWN, it makes
				// for a more responsive-feeling app but actually
				// the guy who made the original post is right.
				// Most apps DO respond to WM_LBUTTONUP, so if you
				// restore your window on WM_LBUTTONDOWN, then some
				// other icon will scroll in under your mouse so when
				// the user releases the mouse, THAT OTHER ICON will
				// get the WM_LBUTTONUP command and that's quite annoying.
				if (lParam == WM_LBUTTONUP) {
					LOGT( "%s\n", "You have restored me!" ) ;
					pGui->pi->Restore();
					//ShowWindow(pGui->pi->hDialog, SW_SHOW);
				}
			} else
				//LOGT("********************************* MSG: 0x%08X WM_USER + %d\n", uMsg, uMsg - WM_USER);
				if (pGui) pGui->OnWmUser(uMsg - WM_USER, GuiEvent(hDlg, wParam));
		} else {
			fProcessed = FALSE;
		}
		break;
	}
	return (fProcessed);
}

BOOL GuiImpl::CtrlBoxDlgProcOnHScroll (
	HWND hWnd,		// hwnd
	HWND hwndCtl,	// (int)(LOWORD(wParam))
	UINT codeNotify,// (UINT)HIWORD(wParam))
	int  pos
	)
{
	Gui *pGui = (Gui *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (pGui) {
		LOGT("********************************* Gui addr: %p pi: %p\n", pGui, pGui->pi);
		return pGui->OnHScroll(GuiEventHScroll(hWnd, hwndCtl, codeNotify, pos));
	}
	else {
		return FALSE;
	}
}


// These next 2 functions are the STARS of this example.
// They perform the "minimization" function and "restore"
// functions for our window.  Notice how when you "minimize"
// the app, it doesn't really "minimize" at all.  Instead,
// you simply HIDE the window, so it doesn't display, and
// at the same time, stick in a little icon in the system tray,
// so the user can still access the application.
void GuiImpl::Minimize()
{
	LOGT ("%s\n", "*********************** Minimize");
	// add the icon to the system tray
	Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

	// ..and hide the main window
	Gui *pGui = (Gui *)GetWindowLongPtr(hDialog, GWLP_USERDATA);
	pGui->Hide();
}

// Basically bring back the window (SHOW IT again)
// and remove the little icon in the system tray.
void GuiImpl::Restore()
{
	LOGT ("%s\n", "*********************** Restore %p", hDialog);

	// show the window
	Gui *pGui = (Gui *)GetWindowLongPtr(hDialog, GWLP_USERDATA);
	pGui->Show();
	UpdateWindow(hDialog);
	// Remove the icon from the system tray
	Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
}

// Initialize the NOTIFYICONDATA structure.
// See MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx
// for details on the NOTIFYICONDATA structure.
void GuiImpl::InitNotifyIconData (int ID_TRAY_APP_ICON, HICON hIcon, const char *tip)
{
	memset( &g_notifyIconData, 0, sizeof( NOTIFYICONDATA ) ) ;
	g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	
	// Tie the NOTIFYICONDATA struct to our
	// global HWND (that will have been initialized
	// before calling this function)
	g_notifyIconData.hWnd = hDialog;
	// Now GIVE the NOTIFYICON.. the thing that
	// will sit in the system tray, an ID.
	g_notifyIconData.uID = ID_TRAY_APP_ICON;
	// The COMBINATION of HWND and uID form
	// a UNIQUE identifier for EACH ITEM in the
	// system tray.  Windows knows which application
	// each icon in the system tray belongs to
	// by the HWND parameter.
	/////
  
	/////
	// Set up flags.
	g_notifyIconData.uFlags =	NIF_ICON |		// promise that the hIcon member WILL BE A VALID ICON!!
								NIF_MESSAGE |	// when someone clicks on the system tray icon,
												// we want a WM_ type message to be sent to our WNDPROC
								NIF_TIP;		// we're gonna provide a tooltip as well, son.

	g_notifyIconData.uCallbackMessage = WM_TRAYICON; //this message must be handled in hwnd's window procedure. more info below.
	// Load da icon.  Be sure to include an icon "green_man.ico" .. get one
	// from the internet if you don't have an icon
	g_notifyIconData.hIcon = hIcon; //(HICON)LoadImage( NULL, TEXT("Tonev-Windows-7-Windows-7-headphone.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE  ) ;
	// set the tooltip text.  must be LESS THAN 64 chars
	stringcopy(g_notifyIconData.szTip, TEXT(tip));
}

