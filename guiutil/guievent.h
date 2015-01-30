/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */

/**
* @file guievent.h
* @brief Header for GUI events
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#if !defined __GUIEVENT_H__
#define __GUIEVENT_H__

struct GuiEvent {
	GuiEvent(HWND h, int i) : hWnd(h), id(i) {}
	HWND hWnd;
	int  id;
};

struct GuiEventHScroll {
	GuiEventHScroll(HWND hW, HWND hC, unsigned cn, int p) :
		hWnd(hW), hwndCtl(hC), codeNotify(cn), pos(p) {}

	HWND hWnd;		// hwnd
	HWND hwndCtl;	// (int)(LOWORD(wParam))
	UINT codeNotify;// (UINT)HIWORD(wParam))
	int  pos;
};
#endif