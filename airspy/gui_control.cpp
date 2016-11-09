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
#include "airspyw.h"
#include <string>

AirSpyCtrlGui::AirSpyCtrlGui(PEXTPRADIO<EXTIO_BASE_TYPE> &pr): 
    Gui(IDD_AIRSPY_CONTROL_DLG), pr_(pr)
{
	std::string fn("AIRSPY");
	
	if (pr_) fn += pr_->get_serial();
	
	cfg_.reset ( new Config<AIRSPY_CFG_T>((fn+".txt").c_str(), std::make_tuple(-1, 5, 10, 6, 0, 0, 0, 10, 10, S_MAN)) );

	LOGT("******* AirSpyCtrlGui: pImpl: %p Gui addr: %p Cfg: %p, (%s)\n", pi, this, cfg_.get(), fn.c_str());

	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

AirSpyCtrlGui::~AirSpyCtrlGui () 
{
}

bool AirSpyCtrlGui::OnInit(const GuiEvent& ev)
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

			LOGT("Sample rate from cfg(%p): %d\n", cfg_.get(), sample_rate);
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
		SendMessage(GetDlgItem(ev.hWnd, ID_COMBO_SR), CB_ADDSTRING, 0, (LPARAM)"Internal error, no sample rate avail");
	}
	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderIfGain = GetDlgItem(ev.hWnd, IDS_IF_GAIN);
	SendMessage(sliderIfGain, TBM_SETRANGE,    (WPARAM)TRUE, (LPARAM)MAKELONG(0, 15));
	SendMessage(sliderIfGain, TBM_SETTICFREQ,  (WPARAM)1,  0);
	int if_gain = cfg_->get<C_IF_G,int>();
	if (pr_) pr_->set_vga_gain(if_gain); 
	SendMessage(sliderIfGain, TBM_SETPOS, (WPARAM)TRUE, if_gain);
    DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_IF_GAIN_VAL), 0), "%d", if_gain);
	
	HWND sliderMixerGain = GetDlgItem(ev.hWnd, IDS_MIXER_GAIN);
	SendMessage(sliderMixerGain, TBM_SETRANGE,    (WPARAM)TRUE, (LPARAM)MAKELONG(0, 15));
	SendMessage(sliderMixerGain, TBM_SETTICFREQ,  (WPARAM)1,  0);
	int mixer_gain = cfg_->get<C_MIX_G,int>();
	if (pr_) pr_->set_mixer_gain(mixer_gain); 
    SendMessage(sliderMixerGain, TBM_SETPOS,    (WPARAM)TRUE, mixer_gain);
	DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_MIXER_GAIN_VAL), 0), "%d", mixer_gain);
    
	HWND sliderLnaGain = GetDlgItem(ev.hWnd, IDS_LNA_GAIN);
	SendMessage(sliderLnaGain, TBM_SETRANGE,    (WPARAM)TRUE, (LPARAM)MAKELONG(0, 15));
	SendMessage(sliderLnaGain, TBM_SETTICFREQ,  (WPARAM)1,  0);
	int lna_gain = cfg_->get<C_LNA_G,int>();
	if (pr_) pr_->set_lna_gain(lna_gain); 
    SendMessage(sliderLnaGain, TBM_SETPOS, (WPARAM)TRUE, lna_gain);
	DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_LNA_GAIN_VAL),0), "%d", lna_gain);
    set_lna_gain_warn (lna_gain == 15);

	HWND sliderSensitivity = GetDlgItem(ev.hWnd, IDS_SENSITIVITY);
	SendMessage(sliderSensitivity, TBM_SETRANGE,    (WPARAM)TRUE, (LPARAM)MAKELONG(0, 20));
	SendMessage(sliderSensitivity, TBM_SETTICFREQ,  (WPARAM)1,  0);
	int sens_value = cfg_->get<C_SENSITIVITY,int>();
	if (pr_) pr_->set_sensitivity_gain(sens_value);
	SendMessage(sliderSensitivity, TBM_SETPOS, (WPARAM)TRUE, sens_value);

	HWND sliderLinearity = GetDlgItem(ev.hWnd, IDS_LINEARITY);
	SendMessage(sliderLinearity, TBM_SETRANGE,    (WPARAM)TRUE, (LPARAM)MAKELONG(0, 20));
	SendMessage(sliderLinearity, TBM_SETTICFREQ,  (WPARAM)1,  0);
	int lin_value = cfg_->get<C_LINEARITY,int>();
	if (pr_) pr_->set_linearity_gain(lin_value);
    SendMessage(sliderLinearity, TBM_SETPOS, (WPARAM)TRUE, lin_value);
	
	// MIXER AGC check box
	int mixer_agc = cfg_->get< C_A_MIX_G, int>();
	LOGT("MIX AGC value from configuration: %d\n", mixer_agc);
	if (pr_) pr_->set_mixer_agc (mixer_agc);
    CheckDlgButton (ev.hWnd, IDCB_AUTO_MIXER_GAIN, mixer_agc);
    // enable manual control accordingly
	::EnableWindow(GetDlgItem(ev.hWnd, IDS_MIXER_GAIN), (mixer_agc == 0));		

	// LNA Automatic Gain Control  check box
	int lna_agc = cfg_->get< C_A_LNA_G, int>();
	LOGT("LAN AGC value from configuration: %d\n", lna_agc);
	if (pr_) pr_->set_lna_agc (lna_agc);
    CheckDlgButton (ev.hWnd, IDCB_AUTO_LNA_GAIN, lna_agc);
    // enable manual control accordingly
    ::EnableWindow(GetDlgItem(ev.hWnd, IDS_LNA_GAIN), (lna_agc == 0));		

	
	// BIAS check box
	int bias = cfg_->get< C_BIAS, int>();
	LOGT("BIAS value from configuration: %d\n", bias);
	if (pr_) pr_->set_rf_bias(bias);	
    CheckDlgButton (ev.hWnd, IDCB_BIAS_TEE, bias);

    //
    // setup for tool bar
    //	
	HICON icon = LoadIcon(Dll::GetMyHandle(), MAKEINTRESOURCE(IDI_ICON1));
	
	SendMessage(ev.hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessage(ev.hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	
	// Initialize the NOTIFYICONDATA structure once
	pi->InitNotifyIconData (IDI_ICON1, icon, "AIRSPY extio");

	// display serial number
	if (pr_) {
		AppendWinTitle(GuiEvent(pi->hDialog, 0), " S/N ");
		AppendWinTitle(GuiEvent(pi->hDialog, 0), pr_->get_serial());
		AppendWinTitle(GuiEvent(pi->hDialog, 0), " - ");
		AppendWinTitle(GuiEvent(pi->hDialog, 0), pr_->version_string());
		LOGT("Device Serial Number: %s\n",  pr_->get_serial());
		LOGT("Device Version String: %s\n", pr_->version_string());
	}

	// select the right gain mode
	// and send commands to hardware
	set_gain_mode ((enum GuiGain)(cfg_->get< C_GAIN_TYPE, short>()));

	return true;
}

void AirSpyCtrlGui::EnableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, true));
	
	Gui::Show();
}

void AirSpyCtrlGui::DisableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	Gui::Show();
}


bool AirSpyCtrlGui::ButtonClick(const GuiEvent &ev)
{
	// BIAS check box
	if ( ev.id == IDCB_BIAS_TEE )  {
	    int bias = (IsDlgButtonChecked(ev.hWnd, IDCB_BIAS_TEE) == BST_CHECKED) ? 1 : 0;
		LOGT("New BIAS value: %d\r\n", bias);
		if (pr_) pr_->set_rf_bias(bias);
		cfg_->set< C_BIAS, int>(bias);
	}

	// MIXER GAIN Automatic Gain Control  check box
	if ( ev.id == IDCB_AUTO_MIXER_GAIN )  {
	    int mixer_agc = (IsDlgButtonChecked(ev.hWnd, IDCB_AUTO_MIXER_GAIN) == BST_CHECKED) ? 1 : 0;
		LOGT("New IDCB_AUTO_MIXER_GAIN value: %d\r\n", mixer_agc);
		if (pr_) pr_->set_mixer_agc (mixer_agc);
		cfg_->set< C_A_MIX_G, int>(mixer_agc);
        // enable manual control accordingly
		::EnableWindow(GetDlgItem(ev.hWnd, IDS_MIXER_GAIN), (mixer_agc == 0));		
	}

	// LNA GAIN Automatic Gain Control  check box
	if ( ev.id == IDCB_AUTO_LNA_GAIN )  {
	    int lna_agc = (IsDlgButtonChecked(ev.hWnd, IDCB_AUTO_LNA_GAIN) == BST_CHECKED) ? 1 : 0;
		LOGT("New IDCB_AUTO_LNA_GAIN value: %d\r\n", lna_agc);
		if (pr_) pr_->set_lna_agc (lna_agc);
		cfg_->set< C_A_LNA_G, int>(lna_agc);
        // enable manual control accordingly
		::EnableWindow(GetDlgItem(ev.hWnd, IDS_LNA_GAIN), (lna_agc == 0));		
	}
	
	// Gain type controls, radio button
	if ( ev.id == IDRB_MANUAL )  {
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			set_gain_mode (S_MAN);
		}
	}
	if ( ev.id == IDRB_LINEARITY )  {
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			set_gain_mode (S_LIN);
		}
	}
	if ( ev.id == IDRB_SENSITIVITY )  {
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			set_gain_mode (S_SEN);
		}
	}
	return true;
}

void AirSpyCtrlGui::set_lna_gain_warn (bool flag)
{
	if (flag == true) DlgItemPrint(GuiEvent(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN),IDC_ST_LNA_GAIN), "%s", "!LNA Gain!");
	             else DlgItemPrint(GuiEvent(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN),IDC_ST_LNA_GAIN), "%s", "LNA Gain");
}


bool  AirSpyCtrlGui::OnHScroll(const GuiEventHScroll& ev)
{
	if (GetDlgItem(ev.hWnd, IDS_LNA_GAIN) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDS_LNA_GAIN), TBM_GETPOS, 0, 0);
		DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_LNA_GAIN_VAL), 0), "%d", newPos);
		LOGT("New LNA GAIN value: %d\r\n", newPos);
		if (pr_) pr_->set_lna_gain (newPos);
		cfg_->set<C_LNA_G,int>(newPos);
		set_lna_gain_warn (newPos == 15);
		return true;
	} else
	if (GetDlgItem(ev.hWnd, IDS_MIXER_GAIN) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDS_MIXER_GAIN), TBM_GETPOS, 0, 0);
		DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_MIXER_GAIN_VAL), 0), "%d", newPos);
		LOGT("New MIXER GAIN value: %d\r\n", newPos);
		if (pr_) pr_->set_mixer_gain (newPos);
		cfg_->set<C_MIX_G,int>(newPos);
		return true;
	} else
	if (GetDlgItem(ev.hWnd, IDS_IF_GAIN) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDS_IF_GAIN), TBM_GETPOS, 0, 0);
		DlgItemPrint( GuiEvent (GetDlgItem(ev.hWnd, IDC_ST_IF_GAIN_VAL), 0), "%d", newPos);
		LOGT("New IF GAIN value: %d\r\n", newPos);
		if (pr_) pr_->set_vga_gain (newPos);
		cfg_->set<C_IF_G,int>(newPos);
		return true;
	} else
	if (GetDlgItem(ev.hWnd, IDS_LINEARITY) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDS_LINEARITY), TBM_GETPOS, 0, 0);
		LOGT("New LINEARITY value: %d\r\n", newPos);
		if (pr_) pr_->set_linearity_gain (newPos);
		cfg_->set<C_LINEARITY,int>(newPos);
		return true;
	} else
	if (GetDlgItem(ev.hWnd, IDS_SENSITIVITY) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDS_SENSITIVITY), TBM_GETPOS, 0, 0);
		LOGT("New SENSITIVITY value: %d\r\n", newPos);
		if (pr_) pr_->set_sensitivity_gain (newPos);
		cfg_->set<C_SENSITIVITY,int>(newPos);
		return true;
	} else
		return false;
}

bool AirSpyCtrlGui::ComboBoxSelChange(const GuiEvent &ev)
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

void AirSpyCtrlGui::set_gain_mode (GuiGain gg)
{
    int man, sen;
    int lin, aut;

	LOGT("MODE: %d\n", gg );

	switch (gg) {
		case S_MAN:
            man = SW_SHOW;
            aut = SW_HIDE;

			ShowWindow(GetDlgItem(pi->hDialog, IDS_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN_VAL), man);

			ShowWindow(GetDlgItem(pi->hDialog, IDS_SENSITIVITY), aut);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LINEARITY), aut);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LINEARITY), aut);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_SENSITIVITY), aut);

			if (IsDlgButtonChecked(pi->hDialog, IDRB_MANUAL) != BST_CHECKED)
				CheckDlgButton (pi->hDialog, IDRB_MANUAL, TRUE);

			// restore manual status
			{
				int mixer_agc = (IsDlgButtonChecked(pi->hDialog, IDCB_AUTO_MIXER_GAIN) == BST_CHECKED) ? 1 : 0;
				LOGT("New IDCB_AUTO_MIXER_GAIN value: %d\r\n", mixer_agc);
				if (pr_) pr_->set_mixer_agc (mixer_agc);
				// enable manual control accordingly
				::EnableWindow(GetDlgItem(pi->hDialog, IDS_MIXER_GAIN), (mixer_agc == 0));

				// LNA GAIN Automatic Gain Control  check box
				int lna_agc = (IsDlgButtonChecked(pi->hDialog, IDCB_AUTO_LNA_GAIN) == BST_CHECKED) ? 1 : 0;
				LOGT("New IDCB_AUTO_LNA_GAIN value: %d\r\n", lna_agc);
				if (pr_) pr_->set_lna_agc (lna_agc);
				// enable manual control accordingly
				::EnableWindow(GetDlgItem(pi->hDialog, IDS_LNA_GAIN), (lna_agc == 0));

				DWORD newPos = SendMessage(GetDlgItem(pi->hDialog, IDS_LNA_GAIN), TBM_GETPOS, 0, 0);
				LOGT("New LNA GAIN value: %d\r\n", newPos);
				if (pr_) pr_->set_lna_gain (newPos);
				set_lna_gain_warn (newPos == 15);

				newPos = SendMessage(GetDlgItem(pi->hDialog, IDS_MIXER_GAIN), TBM_GETPOS, 0, 0);
				LOGT("New MIXER GAIN value: %d\r\n", newPos);
				if (pr_) pr_->set_mixer_gain (newPos);

				newPos = SendMessage(GetDlgItem(pi->hDialog, IDS_IF_GAIN), TBM_GETPOS, 0, 0);
				LOGT("New IF GAIN value: %d\r\n", newPos);
				if (pr_) pr_->set_vga_gain (newPos);
			}
			break;
		case S_LIN:
            man = SW_HIDE;
            sen = SW_HIDE;
            lin = SW_SHOW;

			ShowWindow(GetDlgItem(pi->hDialog, IDS_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN_VAL), man);

			ShowWindow(GetDlgItem(pi->hDialog, IDS_SENSITIVITY), sen);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LINEARITY), lin);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LINEARITY), lin);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_SENSITIVITY), sen);

			if (IsDlgButtonChecked(pi->hDialog, IDRB_LINEARITY) != BST_CHECKED)
				CheckDlgButton (pi->hDialog, IDRB_LINEARITY, TRUE);

			OnHScroll(GuiEventHScroll(pi->hDialog, GetDlgItem(pi->hDialog, IDS_LINEARITY), IDRB_LINEARITY, 0));
			break;
		case S_SEN:
            man = SW_HIDE;
            sen = SW_SHOW;
            lin = SW_HIDE;

			ShowWindow(GetDlgItem(pi->hDialog, IDS_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDCB_AUTO_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_IF_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_MIXER_GAIN_VAL), man);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LNA_GAIN_VAL), man);

			ShowWindow(GetDlgItem(pi->hDialog, IDS_SENSITIVITY), sen);
			ShowWindow(GetDlgItem(pi->hDialog, IDS_LINEARITY), lin);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_LINEARITY), lin);
			ShowWindow(GetDlgItem(pi->hDialog, IDC_ST_SENSITIVITY), sen);

			if (IsDlgButtonChecked(pi->hDialog, IDRB_SENSITIVITY) != BST_CHECKED)
				CheckDlgButton (pi->hDialog, IDRB_SENSITIVITY, TRUE);

			OnHScroll(GuiEventHScroll(pi->hDialog, GetDlgItem(pi->hDialog, IDS_SENSITIVITY), IDRB_SENSITIVITY, 0));
		default:
		    ;
	}
	cfg_->set< C_GAIN_TYPE, short>(gg);
}
