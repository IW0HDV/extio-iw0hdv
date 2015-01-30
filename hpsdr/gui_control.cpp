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

/**
* @file gui_control.cpp
* @brief Gui control panel for HPSDR Extio DLL GUI classes
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


#include "util.h"
#include "hpsdr.h"
#include "log.h"
#include "hpsdrw.h"
#include "log.h"
#include "dllmain.h" // for GetMyHandle()
#include "extio_config.h"
#include "extio_hpsdr.h"
#include "guievent.h"
#include "guiutil.h"
#include "gui_impl.h"
#include "gui.h"
#include "gui_control.h" 

static const char *buildString = " - " __DATE__ ", " __TIME__ " - "
#if defined _MSC_VER
"ms";
#elif defined __MINGW32__
"gcc";
#else
"";
#endif



GuiHpsdr::GuiHpsdr(): Gui()
{
}

GuiHpsdr::GuiHpsdr (int id) : Gui(id), pr(0), pExr(0)
{
	LOGT("********************************* pImpl: %p Gui addr: %p\n", pi, this);
}


GuiHpsdr :: ~GuiHpsdr()
{
	LOGT("********************************* GUI destructor: Gui addr: %p\n", pi);
	
}

bool GuiHpsdr :: OnInit(const GuiEvent&) 
{ 
	LOGT("%s\n", "*********************************");
	return false; 
}



void GuiHpsdr::setHwAddressGUI(const Ethernet::Device *pDev)
{
	char szBuf[1024];

	if (pDev) {
		snprintf(szBuf, sizeof(szBuf), 
				"%s - %s\t\t\t%s %1.1f", 
				pDev->ip_address, 
				pDev->mac_address, 
				pDev->board_id, 
				((float)pDev->code_version) / 10.0f);
	} else {
		snprintf(szBuf, sizeof(szBuf), "%s", "HARDWARE NOT FOUND !!!!!");
	}
	if (pi && pi->hDialog) PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0));
	//	free (pszBuf);  FREE IS DONE IN DIALOG BOX !!!!!!!
}


void GuiHpsdr::setHw (const char *pszBuf)
{
	if (pszBuf && strlen(pszBuf) && pi && pi->hDialog) {
		PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
	}
}


int  GuiHpsdr::getRecNumber (void)
{
	char  buf[256];
	int   n = 1;

	if (pi && pi->hDialog) {
		SendMessage(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf);
		if (sscanf(buf, "%d", &n) == 1) return n;
	}
	LOGT("Internal error !! Returning %d\n", n);
	return n;
}

void GuiHpsdr::setRadio (ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > *pR)
{
	if (pR && pi) {
		pExr = pR;
		pr = pExr->getRadio();
	}
}
ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > * GuiHpsdr::getRadio()
{
	if (pExr) {
		return pExr;
	} else
		return 0;
}




/*
 * H E R M E S   Gui
 *
 *
 */ 


HermesGui::HermesGui(int sample_rate) : GuiHpsdr(IDD_HERMES), sr(sample_rate)
{

	LOGT("********************************* HermesCreateGUI: addr: %p\n", this);
	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

void HermesGui::EnableControls()
{
	if (pi) {
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, true));
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), false));
	}
	Gui::Show();
}

void HermesGui::DisableControls()
{
	if (pi) {
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, false));
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), true));
	}
	Gui::Show();
}


bool HermesGui::OnInit(const GuiEvent& ev)
{ 
	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderAtt = GetDlgItem(ev.hWnd, IDC_SLIDER_ATT);
	SendMessage(sliderAtt, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 30));
	SendMessage(sliderAtt, TBM_SETPOS, (WPARAM)0, 0);
	SendMessage(sliderAtt, TBM_SETTICFREQ, (WPARAM)10, 0);
	SendMessage(sliderAtt, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)MAKELONG(0, 10));
	// fillin the rx number combo box
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"2");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"3");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"4");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"5");
	// default at first item, one receiver(s)
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 1, 0);

	switch (sr) {
	case 384000:
		// order of parameters in the call: 
		//                                     first             last             check
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_384K);
		break;
	case 192000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_192K);
		break;
	case 96000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_96K);
		break;
	case 48000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_48K);
		break;
	}
	
	// default to Antenna 1
	CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, IDC_ANT_1 );
	if (pr) (pr)->setTxAnt(0);

	EnableAll(ev, GuiEvent(0, false));
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), true));

	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);

	return true; 
}
bool HermesGui::ButtonClick(const GuiEvent &ev)
{
	// some button/checkbox has been clicked
	// the lower word of wParam holds the controls ID
	if (ev.id >= IDC_RADIO_BW_384K && ev.id <= IDC_RADIO_BW_48K)  {
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, ev.id);
		switch (ev.id) {
		case IDC_RADIO_BW_384K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 384 kS/s\r\n");
			LOGT("%s", "New Bandwith: 384 kS/s\r\n");
			(pExr)->setSampleRateHW(384000);
			break;
		case IDC_RADIO_BW_192K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 192 kHz\r\n");
			LOGT("%s", "New Bandwith: 192 kHz\r\n");
			(pExr)->setSampleRateHW(192000);
			break;
		case IDC_RADIO_BW_96K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 96 kHz\r\n");
			LOGT("%s", "New Bandwith: 96 kHz\r\n");
			(pExr)->setSampleRateHW(96000);
			break;
		case IDC_RADIO_BW_48K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 48 KHz\r\n");
			LOGT("%s", "New Bandwith: 48 KHz\r\n");
			(pExr)->setSampleRateHW(48000);
			break;
		}
	}
	if (ev.id >= IDC_CB_DITHER && ev.id <= IDC_CB_RANDOMIZER)  {

//		HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		const char *pszState;
		int   newStatus;

		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = " activated\r\n";
			newStatus = 1;
		}
		else {
			pszState = " deactivated\r\n";
			newStatus = 0;
		}
		switch (ev.id) {
		case IDC_CB_PREAMP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Preamp");
			pr->setPreamp(newStatus==1);
			break;
		case IDC_CB_RANDOMIZER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Randomizer");
			pr->setRandomizer(newStatus==1);
			break;
		case IDC_CB_DITHER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Dither");
			pr->setDither(newStatus==1);
			break;
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
	}
	
	if (ev.id >= IDC_ANT_1 && ev.id <= IDC_ANT_3) {
		CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, ev.id);
		switch (ev.id) {
		case IDC_ANT_1:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 1\r\n");
			(pr)->setTxAnt(0);
			break;
		case IDC_ANT_2:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 2\r\n");
			(pr)->setTxAnt(1);
			break;
		case IDC_ANT_3:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 3\r\n");
			(pr)->setTxAnt(2);
			break;			
		}
	}
	return true;
}

bool HermesGui::OnHScroll(const GuiEventHScroll& ev)
{
	if (GetDlgItem(ev.hWnd, IDC_SLIDER_ATT) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_GETPOS, 0, 0);
		int snap = (newPos % 10);
		LOGT("NEW: %d SNAP: %d\r\n", newPos, snap);
		if (snap) {
			if (snap > 5) newPos += (10 - snap);
			else newPos = newPos - snap;
			if (newPos > 30) newPos = 30;
			LOGT("MOVE: %d\r\n", newPos);
			SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)newPos);
		}
		else {
			LOGT("New attenuator value: %d\r\n", newPos);
			pr->setAttenuator(newPos);
		}
		return true;
	}
	else
		return false;
}

bool HermesGui::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);

	if ((n == 2) && pszText) {
		SetWindowText (GetDlgItem(ev.hWnd, IDC_STATIC_HW), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
	if ((n = 3) && pszText) {
		AppendTextToEditCtrl (GuiEvent(ev.hWnd, IDC_MSG_PANE), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
		return false;
}




bool MercuryGui::OnInit(const GuiEvent& ev)
{
	LOGT("Event ref: %p\n", ev);

	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderAtt = GetDlgItem(ev.hWnd, IDC_SLIDER_ATT);
	SendMessage(sliderAtt, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 30));
	SendMessage(sliderAtt, TBM_SETPOS, (WPARAM)0, 0);
	SendMessage(sliderAtt, TBM_SETTICFREQ, (WPARAM)10, 0);
	SendMessage(sliderAtt, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)MAKELONG(0, 10));
	// fillin the rx number combo box
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"2");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"3");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"4");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"5");
	// default at first item, one receiver(s)
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 1, 0);

	switch (sr) {
	case 384000:
		// first last check
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_384K);
		break;
	case 192000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_192K);
		break;
	case 96000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_96K);
		break;
	case 48000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_48K);
		break;
	}
	CheckRadioButton(ev.hWnd, IDC_ALEX_FILTER_AUTO, IDC_ALEX_FILTER_AUTO, IDC_ALEX_FILTER_AUTO);

	// default to Antenna 1
	CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, IDC_ANT_1 );

	// disable all controls
	EnableAll(ev, GuiEvent(0, false));
	
	// enable receivers selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), true));
	// enable bandwidth selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_384K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_192K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_96K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_48K), true));	
	
	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);

	return true;
}


bool MercuryGui::ButtonClick(const GuiEvent &ev)
{
	// some button/checkbox has been clicked
	// GuiEvent ev object contains the handle of dialog and the id of control that has been clicked

	if (ev.id >= IDC_RADIO_BW_384K && ev.id <= IDC_RADIO_BW_48K)  {
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, ev.id);
		switch (ev.id) {
		case IDC_RADIO_BW_384K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 384 kS/s\r\n");
			LOGT("%s\n", "New Bandwith: 384 kHz");
			(pExr)->setSampleRateHW(384000);
			break;
		case IDC_RADIO_BW_192K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 192 kHz\r\n");
			LOGT("%s\n", "New Bandwith: 192 kHz");
			(pExr)->setSampleRateHW(192000);
			break;
		case IDC_RADIO_BW_96K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 96 kHz\r\n");
			LOGT("%s\n", "New Bandwith: 96 kHz");
			(pExr)->setSampleRateHW(96000);
			break;
		case IDC_RADIO_BW_48K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 48 KHz\r\n");
			LOGT("%s\n", "New Bandwith: 48 KHz");
			(pExr)->setSampleRateHW(48000);
			break;
		}
	}


	if (ev.id >= IDC_RB_ALEX_LP_3020 && ev.id <= IDC_RB_ALEX_LP_1715) {
		CheckRadioButton(ev.hWnd, IDC_RB_ALEX_LP_3020, IDC_RB_ALEX_LP_1715, ev.id);
		switch (ev.id) {
		case IDC_RB_ALEX_LP_3020:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 30/20 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_3020m);
			break;
		case IDC_RB_ALEX_LP_6040:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 60/40 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_6040m);
			break;
		case IDC_RB_ALEX_LP_80:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 80 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_80m);
			break;
		case IDC_RB_ALEX_LP_160:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 160 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_160m);
			break;
		case IDC_RB_ALEX_LP_6:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 6 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_6m);
			break;
		case IDC_RB_ALEX_LP_1210:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 12/10 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_1210m);
			break;
		case IDC_RB_ALEX_LP_1715:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 17/15 m\r\n");
			(pr)->setLP(AlexFilter::LowPass::_1715m);
			break;
		}
	}

	if (ev.id >= IDC_ALEX_HP_BYPASS && ev.id <= IDC_ALEX_HP_6M_LNP) {
		CheckRadioButton(ev.hWnd, IDC_ALEX_HP_BYPASS, IDC_ALEX_HP_6M_LNP, ev.id);
		switch (ev.id) {
		case IDC_ALEX_HP_BYPASS:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: bypass\r\n");
			(pr)->setHP(AlexFilter::HighPass::_bypass);
			break;
		case IDC_ALEX_HP_13MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 13 MHz\r\n");
			(pr)->setHP(AlexFilter::HighPass::_13M);
			break;
		case IDC_ALEX_HP_20MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 20 MHz\r\n");
			(pr)->setHP(AlexFilter::HighPass::_20M);
			break;
		case IDC_ALEX_HP_9_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 9.5 MHz\r\n");
			(pr)->setHP(AlexFilter::HighPass::_9_5M);
			break;
		case IDC_ALEX_HP_6_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 6.5 MHz\r\n");
			(pr)->setHP(AlexFilter::HighPass::_6_5M);
			break;
		case IDC_ALEX_HP_1_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 1.5 MHz\r\n");
			(pr)->setHP(AlexFilter::HighPass::_1_5M);
			break;
		case IDC_ALEX_HP_6M_LNP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 6 MHz + LNP\r\n");
			(pr)->setHP(AlexFilter::HighPass::_6M);
			break;
		}
	}

	if (ev.id == IDC_ALEX_FILTER_AUTO) {
		const char *pszState;

		// HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = "ALEX AUTO\n";
			(pr)->setManual(false);
		}
		else {
			pszState = "ALEX MANUAL\r\n";
			(pr)->setManual(true);
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
		LOGT("%p %s", (pr), pszState);
	}

	if (ev.id >= IDC_CB_DITHER && ev.id <= IDC_CB_RANDOMIZER)  {
		//HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		const char *pszState;
		int   newStatus;
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = " activated\r\n";
			newStatus = 1;
		}
		else {
			pszState = " deactivated\r\n";
			newStatus = 0;
		}
		switch (ev.id) {
		case IDC_CB_PREAMP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Preamp");
			pr->setPreamp(newStatus == 1);
			break;
		case IDC_CB_RANDOMIZER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Randomizer");
			pr->setRandomizer(newStatus == 1);
			break;
		case IDC_CB_DITHER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Dither");
			pr->setDither(newStatus == 1);
			break;
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
	}
	
	if (ev.id >= IDC_ANT_1 && ev.id <= IDC_ANT_3) {
		CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, ev.id);
		switch (ev.id) {
		case IDC_ANT_1:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 1\r\n");
			(pr)->setTxAnt(0);
			break;
		case IDC_ANT_2:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 2\r\n");
			(pr)->setTxAnt(1);
			break;
		case IDC_ANT_3:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 3\r\n");
			(pr)->setTxAnt(2);
			break;			
		}
	}
	return true;
}

void MercuryGui::EnableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, true));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), false));
	
	Gui::Show();
}

void MercuryGui::DisableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, false));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), true));
	// enable bandwidth selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_384K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_192K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_96K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_48K), true));	
	Gui::Show();
}


MercuryGui::MercuryGui(int sample_rate) : GuiHpsdr(IDD_MERCURY), sr(sample_rate)
{
	LOGT("********************************* MercuryCreateGUI: pImpl: %p Gui addr: %p\n", pi, this);

	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

bool MercuryGui::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);

	if ((n == 2) && pszText) {
		LOGT("2: %s\n", pszText);
		SetWindowText (GetDlgItem(ev.hWnd, IDC_STATIC_HW), (LPCTSTR)pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
	if ((n == 3) && pszText) {
		LOGT("3: %s\n", pszText);
		AppendTextToEditCtrl (GuiEvent(ev.hWnd, IDC_MSG_PANE), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
		return false;
}

bool MercuryGui::OnHScroll(const GuiEventHScroll& ev)
{
	if (GetDlgItem(ev.hWnd, IDC_SLIDER_ATT) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_GETPOS, 0, 0);
		int snap = (newPos % 10);
		LOGT("NEW: %d SNAP: %d\r\n", newPos, snap);
		if (snap) {
			if (snap > 5) newPos += (10 - snap);
			else newPos = newPos - snap;
			if (newPos > 30) newPos = 30;
			LOGT("MOVE: %d\r\n", newPos);
			SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)newPos);
		}
		else {
			LOGT("New attenuator value: %d\r\n", newPos);
			pr->setAttenuator(newPos);
		}
		return true;
	} else
	return false;
}



//
// Command receiver Globals
//

#pragma data_seg (".SS_GUI")

// !!!! have to be initialized vars, due to shared segments rules constraints
HWND hCmdRec[10] SHARED = { 0 };

#pragma data_seg ()



CommandReceiver :: CommandReceiver (ExtIODll *peio):  GuiHpsdr(IDD_CMD_RECEIVER), pEio(peio)
{
	LOGT("********************************* pImpl: %p CmdReceiver addr: %p\n", pi, this);
	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}


CommandReceiver :: ~CommandReceiver ()
{
	LOGT("********************************* Command Receiver destructor: obj addr: %p\n", pi);
	unsigned int n = Dll::GetInstanceNumber () - 1;  // change to a proper array index
	
	if ( n < ARRAY_SIZE(hCmdRec) ){
		hCmdRec [n] = 0;
	}

}

bool CommandReceiver::OnInit(const GuiEvent& ev)
{
	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);
	unsigned int n = Dll::GetInstanceNumber () - 1;  // change to a proper array index
	
	if ( n < ARRAY_SIZE(hCmdRec) ){
		LOGT("********************************* Command Receiver OnInit: obj addr: %p, instance#: %d - %p\n", pi, n, pi->hDialog);
		hCmdRec [n] = pi->hDialog;
	}
	return true;
}

void CommandReceiver :: SendOtherInstancesNewSampleRate (unsigned int nsr)
{
	unsigned int n = Dll::GetInstanceNumber () - 1; // change to a proper array index
	
	for (unsigned int i=0; i < ARRAY_SIZE(hCmdRec); ++i ) {
		if (i != n && hCmdRec[i]) {
			LOGT("Sending new sample rate: %d to instance %d +1\n", nsr, i);
			PostMessage (hCmdRec[i], WM_USER+1, reinterpret_cast<WPARAM>(nsr), static_cast<LPARAM>(0));
		}
	}
}

void CommandReceiver :: SendOtherInstancesStart (void)
{
	unsigned int n = Dll::GetInstanceNumber () - 1; // change to a proper array index
	
	for (unsigned int i=0; i < ARRAY_SIZE(hCmdRec); ++i ) {
		if (i != n && hCmdRec[i]) {
			LOGT("Sending START to instance %d +1\n", i);
			PostMessage (hCmdRec[i], WM_USER+2, static_cast<WPARAM>(0), static_cast<LPARAM>(0));
		}
	}
}

void CommandReceiver :: SendOtherInstancesStop (void)
{
	unsigned int n = Dll::GetInstanceNumber () - 1; // change to a proper array index
	
	for (unsigned int i=0; i < ARRAY_SIZE(hCmdRec); ++i ) {
		if (i != n && hCmdRec[i]) {
			LOGT("Sending STOP to instance %d +1\n", i);
			PostMessage (hCmdRec[i], WM_USER+3, static_cast<WPARAM>(0), static_cast<LPARAM>(0));
		}
	}
}
#if 0
void CommandReceiver :: SendOtherInstancesHWLO(long freq) 
{
	unsigned int n = Dll::GetInstanceNumber () - 1; // change to a proper array index
	
	for (unsigned int i=0; i < ARRAY_SIZE(hCmdRec); ++i ) {
		if (i != n && hCmdRec[i]) {
			LOGT("Sending new HWLO: %d to instance %d +1\n", freq, i);
			PostMessage (hCmdRec[i], WM_USER+4, static_cast<WPARAM>(freq), static_cast<LPARAM>(0));
		}
	}
}
#endif
void CommandReceiver :: SendOtherInstancesClose() 
{
	unsigned int n = Dll::GetInstanceNumber () - 1; // change to a proper array index
	
	for (unsigned int i=0; i < ARRAY_SIZE(hCmdRec); ++i ) {
		if (i != n && hCmdRec[i]) {
			LOGT("Sending CLOSE to instance %d +1\n", i);
			PostMessage (hCmdRec[i], WM_USER+5, static_cast<WPARAM>(0), static_cast<LPARAM>(0));
		}
	}
}

bool CommandReceiver::OnWmUser(int n, const GuiEvent& ev)
{
	if (n == 1)	{
		LOGT("********************************* Command Receiver WM_USER + %d Change Sample rate: obj addr: %p\n", n, pi);
		int nsr = ev.id;
		LOGT("New sample rate received: %d\n", nsr);
		if (Dll::GetInstanceNumber() != 1) {
			LOGT("new sample rate: %d\n", nsr);
			//(*ExtioCallback) (-1, 100, 0., 0);
			if (pEio) pEio->signalSamplerateChanged();
		}
		return true;
	} else
	if (n == 2)	{
		LOGT("********************************* Command Receiver WM_USER + %d START: obj addr: %p\n", n, pi);
		if (Dll::GetInstanceNumber() != 1) {
			LOGT("%s\n", "START");
			//(*ExtioCallback) (-1, 107, 0., 0);
			if (pEio) pEio->signalStartPressed();
			
		}
		return true;
	} else
	if (n == 3)	{
		LOGT("********************************* Command Receiver WM_USER + %d STOP: obj addr: %p\n", n, pi);
		if (Dll::GetInstanceNumber() != 1) {
			LOGT("%s\n", "STOP");
			//(*ExtioCallback) (-1, 108, 0., 0);
			if (pEio) pEio->signalStartReleased();
		}
		return true;
	} else
	if (n == 4)	{
		LOGT("********************************* Command Receiver WM_USER + %d HWLO: obj addr: %p\n", n, pi);
		long freq = ev.id;
		LOGT("HWLO command received: %d\n", freq);
		if (Dll::GetInstanceNumber() != 1) {
			LOGT("new Hardware Local Oscillator: %d\n", freq);
			//(*ExtioCallback) (-1, 101, 0., 0);
			if (pEio) pEio->signalLocalOscillatorChanged ();
		}
		return true;
	} else
	if (n == 5)	{
		LOGT("********************************* Command Receiver WM_USER + %d CLOSE: obj addr: %p\n", n, pi);
		long freq = ev.id;
		LOGT("CLOSE command received: %d\n", freq);
		if (Dll::GetInstanceNumber() != 1) {
			PostMessage( GetAncestor( pi->hDialog, GA_ROOTOWNER), WM_QUIT, 0,0 );
		}
		return true;
	} else	
		return false;
}
