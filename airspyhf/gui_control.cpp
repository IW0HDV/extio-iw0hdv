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
	
	cfg_.reset ( new Config<AIRSPYHF_CFG_T>((fn+".txt").c_str(), std::make_tuple(-1, 0, 0, 0, 0, 0) ));

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

			// read calibration from config file
			int32_t ppb = cfg_->get<C_CAL,int>();
			LOGT("Calibration from cfg(%p): %d\n", cfg_.get(), ppb);

			//
			// read calibration from the radio
			// if a value != 0 is found, realign configuration file
			// to the value found in hardware
			//
			// this behavior should be useful when you attach
			// a calibrated hardware that has been flashed during the
			// manufacturing process
			//
			if (pr_) {
				int rppb;
				// get calibration and ...
				pr_-> get_calibration (&rppb);
				LOGT("Calibration from radio: %d\n", rppb);
				// ... if different ...
				if (rppb != 0 && rppb != ppb) {
					ppb = rppb;
					// ... save it back to config file
					cfg_->set<C_CAL,int>(ppb);
				}
			}
			// set the PPB entry field text
			snprintf (buf, sizeof(buf), "%d", ppb);
			SendMessage(GetDlgItem(ev.hWnd, ID_EDIT_PPB), WM_SETTEXT, 0, (LPARAM)buf);

			// read GP I/O from configuration file and set check boxes accordingly
			Button_SetCheck(GetDlgItem(ev.hWnd, ID_CB_GPIO_0), (cfg_->get<C_GPIO_0,int>() == 1) ? BST_CHECKED : BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(ev.hWnd, ID_CB_GPIO_1), (cfg_->get<C_GPIO_1,int>() == 1) ? BST_CHECKED : BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(ev.hWnd, ID_CB_GPIO_2), (cfg_->get<C_GPIO_2,int>() == 1) ? BST_CHECKED : BST_UNCHECKED);
			Button_SetCheck(GetDlgItem(ev.hWnd, ID_CB_GPIO_3), (cfg_->get<C_GPIO_3,int>() == 1) ? BST_CHECKED : BST_UNCHECKED);
		}
	} else {
		SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)"AirSpyHf internal error, no sample rate avail");
	}
	//
	// select the range of spin control
	// the max value is negative in order to get going the control with a natural behavior
	// pressing the UP arrow the value increases
	// https://blogs.msdn.microsoft.com/oldnewthing/20051222-12/?p=32873
	//
	SendMessage(GetDlgItem(ev.hWnd, ID_SC_PPB), UDM_SETRANGE, 0, MAKELPARAM(-100000, 100000));

	//
	// setup device list combo box
	//
	const char **dev_list;
	unsigned int nd = AirSpyHfRadio::scan_devices (&dev_list);
	if (nd > 0) {
		int n = -1;

		// fill the combo box

		// test only
		SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), CB_ADDSTRING, 0, (LPARAM)"00010203040506070809101112131415");

		for (unsigned i = 0; i < nd; ++i) {
			// returns the current position
			int x = SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), CB_ADDSTRING, 0, (LPARAM)dev_list[i]);

			//LOGT("Device Serial Number (%d): [%s][%s]\n", i, dev_list[i], pr_->get_serial());

			// store position in the combo of the current device
			if (strcmp (dev_list[i], pr_->get_serial()) == 0) n = x;
		}

		// test only
		SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), CB_ADDSTRING, 0, (LPARAM)"15141312111009080706050403020100");

		if (n == -1) // if we are not there
			// select first row, we have to go somewhere
			SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), CB_SETCURSEL, 0, 0);
		else
			// select the row where the current device is
			SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), CB_SETCURSEL, n, 0);
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
		AppendWinTitle(GuiEvent(pi->hDialog, 0), " S/N:");
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
	LOGT("%s\n", "AirSpyHfCtrlGui::EnableControls");

	GuiEvent ev(pi->hDialog, 0);
	
	::EnableWindow(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), 1);
	::EnableWindow(GetDlgItem(ev.hWnd, ID_PB_FLASH_CAL),  1);
	::EnableWindow(GetDlgItem(ev.hWnd, ID_COMBO_SR),      1);
	
	Gui::Show();
}

void AirSpyHfCtrlGui::DisableControls()
{
	LOGT("%s\n", "AirSpyHfCtrlGui::DisableControls");

	GuiEvent ev(pi->hDialog, 0);

	::EnableWindow(GetDlgItem(ev.hWnd, ID_COMBO_DEVLIST), 0);
	::EnableWindow(GetDlgItem(ev.hWnd, ID_PB_FLASH_CAL),  0);
	::EnableWindow(GetDlgItem(ev.hWnd, ID_COMBO_SR),      0);

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


bool AirSpyHfCtrlGui::OkPressed(const GuiEvent &ev)
{
	if (ev.id == ID_EDIT_PPB) {
		char buf[128] = {0};
		int ppb;

		GetWindowText( GetDlgItem(ev.hWnd, ID_EDIT_PPB), buf, sizeof(buf));
		LOGT("event.id: %d item [%s] selected in PPB edit\n", ev.id, buf );
	  if (sscanf(buf, "%d", &ppb) == 1) {
			cfg_->set<C_CAL,int>(ppb);
			if (pr_) pr_-> set_calibration (ppb);
		}
		return true;
	}
	return false;
}

bool AirSpyHfCtrlGui::ButtonClick(const GuiEvent &ev)
{
	if (ev.id >= ID_CB_GPIO_0 && ev.id <= ID_CB_GPIO_3) {
		int   newStatus;
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			newStatus = 1;
		}
		else {
			newStatus = 0;
		}
		switch (ev.id) {
		case ID_CB_GPIO_0:
			pr_->set_user_output(AIRSPYHF_USER_OUTPUT_0, (newStatus == 1 ? AIRSPYHF_USER_OUTPUT_HIGH : AIRSPYHF_USER_OUTPUT_LOW));
			cfg_->set<C_GPIO_0,int>(newStatus);
			break;
		case ID_CB_GPIO_1:
			pr_->set_user_output(AIRSPYHF_USER_OUTPUT_1, (newStatus == 1 ? AIRSPYHF_USER_OUTPUT_HIGH : AIRSPYHF_USER_OUTPUT_LOW));
			cfg_->set<C_GPIO_1,int>(newStatus);
			break;
		case ID_CB_GPIO_2:
			pr_->set_user_output(AIRSPYHF_USER_OUTPUT_2, (newStatus == 1 ? AIRSPYHF_USER_OUTPUT_HIGH : AIRSPYHF_USER_OUTPUT_LOW));
			cfg_->set<C_GPIO_2,int>(newStatus);
			break;
		case ID_CB_GPIO_3:
			pr_->set_user_output(AIRSPYHF_USER_OUTPUT_3, (newStatus == 1 ? AIRSPYHF_USER_OUTPUT_HIGH : AIRSPYHF_USER_OUTPUT_LOW));
			cfg_->set<C_GPIO_3,int>(newStatus);
			break;
		}
	}
	return false;
}