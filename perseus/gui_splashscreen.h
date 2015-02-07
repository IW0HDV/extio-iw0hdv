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

class CommandReceiver;

#include "gui.h"

class PerseusSplash: public Gui {
public:
	PerseusSplash(Gui **pG, CommandReceiver **pCr);
	~PerseusSplash();
	void Show();

	void SetHw(const char *);
	template <typename... ARGS>
	void SetStatus (const char *, ARGS... args);
	void AppendMessage(const char *); 

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);

private:
	int sel;
};

#include <memory>
typedef std::shared_ptr<PerseusSplash> PSPLASH;

#endif


