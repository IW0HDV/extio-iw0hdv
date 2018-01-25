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
* @file gui.h
* @brief Header for Extio DLL GUI
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#if !defined __GUI_H__
#define		 __GUI_H__


const char *getBuildString ();

/*
 *  Gui utilities
 *  
 */


struct WSize {

	WSize() = delete;
	WSize(HWND hd)
	{
		RECT rd, rD;
		GetWindowRect(GetDesktopWindow(), &rD);
		GetWindowRect(hd, &rd);
		x1 = rd.left, x2 = rd.right, y1 = rd.top, y2 = rd.bottom;
		w = x2 - x1;
		h = y2 - y1;
		x1_dt = rD.left, x2_dt = rD.right, y1_dt = rD.top, y2_dt = rD.bottom;
		w_dt = rD.right - rD.left;
		h_dt = rD.bottom - rD.top;
	}
	void center(int &x, int &y)
	{
		x = (w_dt / 2) - (w / 2);
		y = (h_dt / 2) - (h / 2);
	}
	void lower_right(int &xx, int &yy)
	{
		xx = x2_dt - w;
		yy = y2_dt - h;
	}

	int x1, x2, y1, y2, w, h;
	int x1_dt, x2_dt, y1_dt, y2_dt, w_dt, h_dt;
};


class	GuiImpl;
struct	GuiEvent;
struct	GuiEventHScroll;





class Gui : public MsgAllocator {
public:
	Gui();
	Gui(int resource_id);

	virtual ~Gui();

	virtual void Show();
	virtual void Hide();

	void setHw(const char *);
	void appendMessage(const char *);
	int  getRecNumber(void);

	virtual void EnableControls();
	virtual void DisableControls();

protected:
	void EnableAll(const GuiEvent& ev1, const GuiEvent& ev2);
	// events managers
	virtual bool OnInit(const GuiEvent& ev) = 0;
	virtual bool ButtonClick(const GuiEvent &ev) { return false; }
	virtual bool ButtonDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool ListBoxDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool OnHScroll(const GuiEventHScroll& ev) { return false; }
	virtual bool ComboBoxSelChange(const GuiEvent &ev) { return false; }
	virtual bool OnWmUser(int n, const GuiEvent& ev) { return false; }
	virtual bool OkPressed(const GuiEvent &ev) { return false; }

	friend GuiImpl;
	GuiImpl *pi;
};



#endif
