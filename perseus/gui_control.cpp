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
#include "util.h"
#include "guiutil.h"
#include "gui_impl.h" 
#include "gui.h" 
#include "gui_control.h"
#include "perseusw.h"


PerseusCtrlGui::PerseusCtrlGui(ExtioPerseusRadio2 < EXTIO_BASE_TYPE > *pr) : Gui(IDD_PERSEUS_CONTROL_DLG), pr_(pr)
{
	std::string fn("PERSEUS");
		
	if (pr_) fn += pr_->get_serial();
	
	cfg_ = new Config<PERSEUS_CFG_T>((fn+".txt").c_str(), std::make_tuple(192000, 0, 0, 0, 0));
	
	LOGT("********************************* PerseusCtrlGui: pImpl: %p Gui addr: %p\n", pi, this);

	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

PerseusCtrlGui :: ~PerseusCtrlGui () 
{
	delete cfg_;
}

bool PerseusCtrlGui::OnInit(const GuiEvent& ev)
{
	LOGT("Event ref: %p\n", ev);
	
	const int *psr;
	int nsr;
	int sample_rate =  PERSEUS_DEFAULT_SAMPLE_RATE;
	if (cfg_) {
	    sample_rate = cfg_->get<C_SR,int>();
	    LOGT("Sample rate from cfg(%p): %d\n", cfg_, sample_rate);
	}
	
	if (pr_) {
		psr = pr_->get_sample_rate_v(nsr);
		if (psr && nsr) {
			for (int i=0; i<nsr; ++i) {
				char buf [128];
				snprintf (buf, sizeof(buf), "%d", psr[i]);
				LOGT("Event ref: %d: [%s]\n", i, buf);
				SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)buf);
				if (sample_rate == psr[i]) SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_SETCURSEL, i, 0);
			}
		}
	}
	SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_ATT), CB_ADDSTRING, 0, (LPARAM)"0 dB");
	SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_ATT), CB_ADDSTRING, 0, (LPARAM)"-10 dB");
	SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_ATT), CB_ADDSTRING, 0, (LPARAM)"-20 dB");
	SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_ATT), CB_ADDSTRING, 0, (LPARAM)"-30 dB");
	SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_ATT), CB_SETCURSEL, 0, 0);

	CheckDlgButton(ev.hWnd, ID_CB_PRESELECTOR, BST_CHECKED);
	if (pr_) pr_->set_preselector (true);
	CheckDlgButton(ev.hWnd, ID_CB_DITHER, BST_UNCHECKED);
	if (pr_) pr_->set_dither (false);
	CheckDlgButton(ev.hWnd, ID_CB_AMP, BST_UNCHECKED);
	if (pr_) pr_->set_preamp (false);

	HICON icon = LoadIcon(Dll::GetMyHandle(), MAKEINTRESOURCE(IDI_ICON1));
	
	SendMessage(ev.hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessage(ev.hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	
	// Initialize the NOTIFYICONDATA structure once
	pi->InitNotifyIconData (IDI_ICON1, icon, "Perseus extio");

	if (pr_) AppendWinTitle(GuiEvent(pi->hDialog, 0), pr_->get_serial());
	return true;
}

void PerseusCtrlGui::EnableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, true));
	
	Gui::Show();
}

void PerseusCtrlGui::DisableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, false));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, ID_CB_AMP), true));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, ID_CB_DITHER), true));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, ID_COMBO_ATT), true));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, ID_COMBO_SR), true));

	Gui::Show();
}

bool PerseusCtrlGui::ComboBoxSelChange(const GuiEvent &ev)
{
	if (ev.id == ID_COMBO_SR) {
		char buf[128] = {0};
		
		// recover the item of combo box that has been clicked
		int sel = ComboBox_GetCurSel(GetDlgItem(ev.hWnd, ID_COMBO_SR));
		ComboBox_GetLBText(GetDlgItem(ev.hWnd, ID_COMBO_SR), sel, buf);
		LOGT("event.id: %d item #%d [%s] selected in SR combo\n", ev.id, sel, buf );
		
		int nsr;
		if (sscanf (buf, "%d", &nsr) == 1 && pr_) {
		    cfg_->set<C_SR,int>(nsr);
			pr_->setSampleRateHW(nsr);
		}
		
		return true;
	}
	
	if (ev.id == ID_COMBO_ATT) {
		int sel = ComboBox_GetCurSel(GetDlgItem(ev.hWnd, ID_COMBO_ATT));
		LOGT("Perseus Ctrl Gui: event.id: %d ATT# %d selected in Attenuator combo\n", ev.id, sel);
		if (pr_) pr_->set_attenuator(sel);
		
		return true;
	}
	return false;
}

bool PerseusCtrlGui::ButtonClick(const GuiEvent &ev)
{
	// some button/check box has been clicked on
	if ( ev.id == ID_CB_AMP )  {
		if (pr_) pr_->set_preamp (IsDlgButtonChecked(ev.hWnd, ID_CB_AMP) == BST_CHECKED);
	}
	if ( ev.id == ID_CB_DITHER )  {
		if (pr_) pr_->set_dither (IsDlgButtonChecked(ev.hWnd, ID_CB_DITHER) == BST_CHECKED);
	}
	if ( ev.id == ID_CB_PRESELECTOR )  {
		if (pr_) pr_->set_preselector (IsDlgButtonChecked(ev.hWnd, ID_CB_PRESELECTOR) == BST_CHECKED);
	}
	return true;
}
