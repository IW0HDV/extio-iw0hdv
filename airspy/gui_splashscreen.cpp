/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
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


#include "util.h"
#include "airspy.hpp"
#include "log.h"
#include "log.h"
#include "dllmain.h" // for GetMyHandle()
#include "Extio_config.h"
#include "ExtIO_airspy.h"
#include "guievent.h"
#include "guiutil.h"
#include "gui_impl.h" 
#include "gui.h" 
#include "airspyw.h"
#include "gui_splashscreen.h"

AirSpySplash::AirSpySplash(Gui **p, CommandReceiver **ppCr):
	Gui(IDD_SPLASHSCREEN) //, sel(-1), ppGui_(p), ppCr_(ppCr), pDev(0)
{
	OnInit(GuiEvent(pi->hDialog, -1));
}

bool AirSpySplash::OnInit(const GuiEvent& ev)
{
	AppendWinTitle(GuiEvent(pi->hDialog, 0), getBuildString());
	SetStatus ("%s", "");
	return true;
}


void AirSpySplash::AppendMessage(const char *pszBuf)
{
	PostMessage(pi->hDialog, WM_USER + 3, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
}


template <typename... ARGS>
void AirSpySplash::SetStatus(const char *format, ARGS... args)
{
	char szBuf[1024];

	snprintf(szBuf, sizeof(szBuf), format, args...);
	PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0));
}

void AirSpySplash::Show()
{
	int x, y;

	if (pi && pi->hDialog) {
		WSize(pi->hDialog).center(x, y);
		SetWindowPos(pi->hDialog, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
		ShowWindow(pi->hDialog, SW_SHOW);
	}
}



bool AirSpySplash::OnWmUser(int n, const GuiEvent& ev)
{

	char * pszText = reinterpret_cast<char *>(ev.id);

	if (n == 2 && pszText)	{
		SetWindowText(GetDlgItem(ev.hWnd, IDC_SPLASH_MSG), (LPCTSTR)(char *)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
		return TRUE;
	} 
#if 0
	else
	if (n == 3 && pszText) {
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), pszText);
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), "\r\n");
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
		return TRUE;		
	} else 
	if (n == 4 && pszText)	{
		SendMessage(GetDlgItem(ev.hWnd, IDC_LBOX_RADIO_FOUND), LB_INSERTSTRING, -1 /* Index */, (LPARAM)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
		return TRUE;		
	}
#endif	
	return FALSE;
}

