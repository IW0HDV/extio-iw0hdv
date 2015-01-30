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
#include "log.h"
#include "hpsdrw.h"
#include "dllmain.h" // for GetMyHandle()
#include "guievent.h"
#include "guiutil.h"
#include "gui_impl.h"
#include "gui.h" 

#include "hpsdr.h"
#include "extio_config.h"
#include "extIO_hpsdr.h"

static const char *buildString = " - " __DATE__ ", " __TIME__ " - "
#if defined _MSC_VER
"ms";
#elif defined __MINGW32__
"gcc";
#else
"";
#endif

#include "gui_splashscreen.h"

/**
 *
 * Splash Screen dialog
 *
 */



bool HpsdrSplash::ListBoxDoubleClick(const GuiEvent &ev)
{
	if (ev.id == IDC_LBOX_RADIO_FOUND) {
		// recover the item of list box that has been clicked
		sel = ListBox_GetCurSel(ev.hWnd);
		LOGT("xxxxxxxxxxxxxxxxxx GUI Splash: item #%d selected\n", sel);
		if (!(*(ppGui_))) {
			LOGT("Gui Splash: BOARD ID creating gui: [%s]\n", pDev->board_id);

			if (!(pExr)) {

				// Create Radio
				pExr = CreateExtioHpsdrRadio<EXTIO_SAMPLE_TYPE>(pDev->board_id, *(ppCr_), pe_);

				if (! (pExr)) {
					GuiError("Hardware unsupported, unable to start receiver !").show();
					return 0;
				} else {
					
					// Create Gui
					*(ppGui_) = pExr->CreateGui(EXTIO_DEFAULT_SAMPLE_RATE);

					if (*(ppGui_)) {
						(*(ppGui_))->setRadio(pExr);// assign ExtioRadio to Gui
						Hide();						// hide ourselves (Splash)
						(*(ppGui_))->Show();		// show Radio Gui instead
						//pe_->signalStartPressed (); // finally, start the parent DSP software
					} else
						return false;
				}
			}
		}
	}
	return true;
}



bool HpsdrSplash::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);
	
	if (n == 2 && pszText)	{
		SetWindowText(GetDlgItem(ev.hWnd, IDC_SPLSH_MSG), (LPCTSTR)(char *)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} else
	if (n == 3 && pszText) {
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), pszText);
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), "\r\n");
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} else 
	if (n == 4 && pszText)	{
		SendMessage(GetDlgItem(ev.hWnd, IDC_LBOX_RADIO_FOUND), LB_INSERTSTRING, -1 /* Index */, (LPARAM)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} 
	return true;
}




HpsdrSplash::HpsdrSplash(GuiHpsdr **p, CommandReceiver **ppCr, ExtIODll *pe):
	GuiHpsdr(IDD_SPLASH_DIALOG), sel(-1), ppGui_(p), ppCr_(ppCr), pDev(0), pe_(pe)
{
	OnInit(GuiEvent(pi->hDialog, -1));
}

HpsdrSplash::~HpsdrSplash()
{
	LOGT("%s\n", "xxxxxxxxxxxxxxxxxx GUI Splash: deleted");	
}


bool HpsdrSplash::OnInit(const GuiEvent& ev)
{
	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);
	return true;
}


void HpsdrSplash::AppendMessage(const char *pszBuf)
{
	PostMessage(pi->hDialog, WM_USER + 3, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
}


template <typename... ARGS>
void HpsdrSplash::SetStatus(const char *format, ARGS... args)
{
	char szBuf[1024];

	snprintf(szBuf, sizeof(szBuf), format, args...);
	PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0));
}

int HpsdrSplash::ScanStarted()
{
	SetStatus("%s", "Searching for hardware..."); 
	return 0; 
}

int HpsdrSplash::ScanStopped(int nh)
{ 
	if (nh == 0) SetStatus("%s", "Search done. No device found"); 
	else SetStatus("Search done. %d device(s) found", nh);
	return 0; 
}

int	HpsdrSplash::InterfaceFound(Ethernet::NetInterface *pni)	
{
	AppendMessage(pni->name);  
	return 0;
}

int	HpsdrSplash::DeviceFound(Ethernet::Device *pd)		
{
	// append a clear text (progress) message
	AppendMessage(pd->board_id); 
	// append to the list box
	char szBuf[1024];
	snprintf(szBuf, sizeof(szBuf), "%s - %s", pd->board_id, pd->ip_address);
	PostMessage(pi->hDialog, WM_USER + 4, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0) );
	pDev = pd;
	return 0; 
}

void HpsdrSplash::Show()
{
	int x, y;

	if (pi && pi->hDialog) {
		WSize(pi->hDialog).center(x, y);
		SetWindowPos(pi->hDialog, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
		ShowWindow(pi->hDialog, SW_SHOW);
	}
}
