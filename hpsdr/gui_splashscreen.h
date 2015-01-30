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

#if !defined __GUI_SPLASHSCREEN_H__
#define __GUI_SPLASHSCREEN_H__

#include "gui_control.h"

class ExtIODll;

class HpsdrSplash: public GuiHpsdr, public ScanWatcher {
public:
	HpsdrSplash(GuiHpsdr **pG, CommandReceiver **pCr, ExtIODll *pe);
	~HpsdrSplash();
	void Show();

	void SetHw(const char *);
	template <typename... ARGS>
	void SetStatus (const char *, ARGS... args);
	void AppendMessage(const char *); 

	virtual int ScanStarted();
	virtual int ScanStopped(int nh);
	virtual int	InterfaceFound(Ethernet::NetInterface *pni);
	virtual int	DeviceFound(Ethernet::Device *pd);
	int GetSel() { return sel; }

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);
	virtual bool ListBoxDoubleClick(const GuiEvent &ev);

private:
	int sel;
	GuiHpsdr **ppGui_;
	CommandReceiver **ppCr_;
	Ethernet::Device *pDev;
	ExtIODll *pe_;
};
#endif
