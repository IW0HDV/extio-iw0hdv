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
* @file gui.cpp
* @brief Header for Extio DLL GUI classes
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "dllmain.h" // for GetMyHandle()

#include "guievent.h"
#include "guiutil.h"
#include "gui.h" 
#include "gui_impl.h" 




Gui::Gui(): pi(0)
{
}

Gui::Gui (int id) : pi(new GuiImpl)
{
	LOGT("********************************* pImpl: %p Gui addr: %p\n", pi, this);

	if (pi) {
		LOGT("xxxxxxxxxxxxxxxxxxx GetMyHandle: %p\n", Dll::GetMyHandle());
		pi->hDialog = CreateDialogParam(Dll::GetMyHandle(),
										MAKEINTRESOURCE(id),
										 GetDesktopWindow(), //HWND_DESKTOP,
										GuiImpl::CtrlBoxDlgProc,
										(LONG)this
										);
		if (pi->hDialog != NULL) {
			// do not show window here !!!

			// setup class instance pointer into Windows object user data
			SetWindowLongPtr(pi->hDialog, GWLP_USERDATA, (LONG)this);
			return;
		}
	}
	ErrorLog(TEXT("Gui: main dialog failed !"));
}


Gui :: ~Gui()
{
	LOGT("********************************* GUI destructor: Gui addr: %p\n", pi);
	
	if (pi) {
		// Once you get the quit message, before exiting the app
		// clean up and remove the tray icon
		//if( !IsWindowVisible( pi->hDialog ) ) {
			Shell_NotifyIcon(NIM_DELETE, &(pi->g_notifyIconData) );
		//}
		if (pi->hDialog) DestroyWindow(pi->hDialog);
		delete pi;
	}
}

bool Gui :: OnInit(const GuiEvent&) 
{ 
	LOGT("%s\n", "*********************************");
	return false; 
}


void Gui::Show()
{
	int x, y;

	if (pi && pi->hDialog) {
		WSize(pi->hDialog).lower_right(x, y);
		LOGT("xxxxxxxxxxxxxxxxxx GUI Show: move to x= %d y= %d\n", x, y);
		SetWindowPos(pi->hDialog, HWND_TOPMOST, x, y-(y*0.05), 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		//ShowWindow(pi->hDialog, SW_SHOW );
		//SetWindowPos(pi->hDialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE);
		BringWindowToTop(pi->hDialog);
		//SetFocus(pi->hDialog);
		//SetActiveWindow (pi->hDialog);
		//SetForegroundWindow (pi->hDialog);
	}
}

void Gui::Hide()
{
	if (pi && pi->hDialog) {
		LOGT("%s\n", "xxxxxxxxxxxxxxxxxx GUI HIDE");
		ShowWindow(pi->hDialog, SW_HIDE);
	}
}

void Gui::EnableControls()
{
	if (pi && pi->hDialog) EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, true));
}

void Gui::DisableControls()
{
	if (pi && pi->hDialog) EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, false));
}

BOOL CALLBACK GuiImpl::MyEnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	GuiEvent *pev = (GuiEvent *)lParam;

	if (pev) {
		bool fEnable = (pev->id > 0);

// for debug only
//		char szWindowText[256];
//		::GetWindowText(hWnd, szWindowText, sizeof(szWindowText));
//		LOGT("%10.10s: %d\n", szWindowText, fEnable);

		if (pev->hWnd == 0) // if nullhandle is specified, applies to children
			::EnableWindow(hWnd, fEnable);
		else                // if an handle is specified, applies only to that window
			if (pev->hWnd == hWnd) ::EnableWindow(hWnd, fEnable);
	}
	return TRUE;
}


void Gui::EnableAll (const GuiEvent& ev1, const GuiEvent& ev2)
{
	::EnumChildWindows(ev1.hWnd, GuiImpl::MyEnumWindowsProc, (LPARAM)&ev2);
}



void Gui::setHw (const char *pszBuf)
{
	if (pszBuf && strlen(pszBuf) && pi && pi->hDialog) {
		PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
	}
}

void Gui::appendMessage (const char *pszBuf)
{
	if (pszBuf && strlen(pszBuf) && pi && pi->hDialog) PostMessage(pi->hDialog, WM_USER + 3, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
}



