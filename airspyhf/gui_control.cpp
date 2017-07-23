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
#include <windows.h>
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
#include "airspyhfw.h"
#include <string>

AirSpyHfCtrlGui::AirSpyHfCtrlGui(PEXTPRADIO<EXTIO_BASE_TYPE> &pr): 
    Gui(IDD_AIRSPY_CONTROL_DLG), pr_(pr)
{
	std::string fn("AIRSPYHF");
	
	if (pr_) fn += pr_->get_serial();
	
	cfg_.reset ( new Config<AIRSPYHF_CFG_T>((fn+".txt").c_str(), std::make_tuple(-1) ));

	LOGT("******* AirSpyHfCtrlGui: pImpl: %p Gui addr: %p Cfg: %p, (%s)\n", pi, this, cfg_.get(), fn.c_str());

	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

AirSpyHfCtrlGui::~AirSpyHfCtrlGui () 
{
}

bool AirSpyHfCtrlGui::OnInit(const GuiEvent& ev)
{
	LOGT("Event ref: %p\n", ev);

	// detect dynamically samplerates
	int nsr;
	if ( pr_ && (nsr = pr_-> get_samplerates ()) ) {
		char buf [256];

		// fill the drop down box in GUI with all sample rates available from hardware
		for (int i = 0; i < nsr; ++i) {
			int sr = pr_-> get_samplerate_n (i);
			LOGT("%d: %d\n", i, sr);
			if (sr > 0) {
				snprintf (buf, sizeof(buf), "%d", sr);
				SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)buf);
			}
		}
		if (cfg_) {
			// read sample rate from configuration file
			int sample_rate = cfg_->get<C_SR,int>();
			// the first time the configuration file is created we don't have a valid sample rate
			// take the first one available from the hardware
			if (sample_rate < 0) {
				sample_rate = pr_-> get_samplerate_n (0);
				// save it back to config file
				cfg_->set<C_SR,int>(sample_rate);
			} else {
				LOGT("Sample rate from cfg(%p): %d\n", cfg_.get(), sample_rate);
			}

			// convert to string
			snprintf (buf, sizeof(buf), "%d", sample_rate);

			int index = SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_SELECTSTRING, -1, (LPARAM)buf);
			if (index != CB_ERR) {
				SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_SETCURSEL, index, 0);
				if (pr_) pr_-> set_sample_rate(sample_rate);
			} else {
				snprintf (buf, sizeof(buf), "Sample rate %d not available in hardware", sample_rate);
				SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)buf );
			}
		}
	} else {
		SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)"AirSpyHf internal error, no sample rate avail");
	}
	//
	// setup for tool bar
	//	
	HICON icon = LoadIcon(Dll::GetMyHandle(), MAKEINTRESOURCE(IDI_ICON1));
	
	SendMessage(ev.hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessage(ev.hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	
	// Initialize the NOTIFYICONDATA structure once
	pi->InitNotifyIconData (IDI_ICON1, icon, "AIRSPYHF extio");

	// display serial number
	if (pr_) {
		AppendWinTitle(GuiEvent(pi->hDialog, 0), " S/N ");
		AppendWinTitle(GuiEvent(pi->hDialog, 0), pr_->get_serial());
		AppendWinTitle(GuiEvent(pi->hDialog, 0), " - ");
		AppendWinTitle(GuiEvent(pi->hDialog, 0), pr_->version_string());
		LOGT("Device Serial Number: %s\n",  pr_->get_serial());
		LOGT("Device Version String: %s\n", pr_->version_string());
	}

	return true;
}

void AirSpyHfCtrlGui::EnableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, true));
	
	Gui::Show();
}

void AirSpyHfCtrlGui::DisableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	Gui::Show();
}


bool AirSpyHfCtrlGui::ComboBoxSelChange(const GuiEvent &ev)
{
	if (ev.id == ID_COMBO_SR) {
		char buf[128] = {0};
		
		// recover the item of combo box that has been clicked
		int sel = ComboBox_GetCurSel(GetDlgItem(ev.hWnd, ID_COMBO_SR));
		ComboBox_GetLBText(GetDlgItem(ev.hWnd, ID_COMBO_SR), sel, buf);
		LOGT("event.id: %d item #%d [%s] selected in SR combo\n", ev.id, sel, buf );
		
		int nsr;
		if (sscanf (buf, "%d", &nsr) == 1 && pr_) pr_->setSampleRateHW(nsr);
		cfg_->set<C_SR,int>(nsr);
		
		return true;
	}
	return false;
}

