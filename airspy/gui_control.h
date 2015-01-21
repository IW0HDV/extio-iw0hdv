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

#include "gui.h"
#include "extio_config.h"
#include "extio_airspy.h"

class AirSpyCtrlGui: public Gui {
public:
	AirSpyCtrlGui (ExtioAirSpyRadio < EXTIO_BASE_TYPE > *);
	~AirSpyCtrlGui () {}

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
    virtual bool ComboBoxSelChange(const GuiEvent &ev);

private:
	ExtioAirSpyRadio < EXTIO_BASE_TYPE > *pr_;
};

#endif
