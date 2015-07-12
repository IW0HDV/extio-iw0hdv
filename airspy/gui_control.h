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
 
#if !defined __GUI_CONTROL_H__
#define      __GUI_CONTROL_H__

#include <memory>
class AirSpyCtrlGui;
typedef std::shared_ptr<AirSpyCtrlGui> PCTRLGUI;

#include "gui.h"
#include "extio_config.h"
#include "extio_airspy.h"
#include <config.h>
	
typedef std::tuple<int, int, int, int, int, int, int, int> AIRSPY_CFG_T;
	

class AirSpyCtrlGui: public Gui {
public:
	AirSpyCtrlGui (PEXTPRADIO<EXTIO_BASE_TYPE> &pExr);
	~AirSpyCtrlGui ();

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
    virtual bool ComboBoxSelChange(const GuiEvent &ev);

private:

    void set_lna_gain_warn (bool);

    PEXTPRADIO<EXTIO_BASE_TYPE> pr_;
	
	enum { C_SR = 0, C_LNA_G = 1, C_MIX_G = 2, C_IF_G = 3, C_A_MIX_G = 4, C_A_LNA_G = 5, C_BIAS = 6, C_FREQ_C = 7 } ;
	
	std::unique_ptr < Config<AIRSPY_CFG_T> > cfg_;
	
};

#endif
