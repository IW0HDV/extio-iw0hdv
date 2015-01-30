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
* @file guiutil.h
* @brief Header for GUI utilities
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#if !defined	__GUIUTIL_H__
#define			__GUIUTIL_H__

struct GuiEvent;

class GuiError {
public:
	GuiError(const char *p) : p_(p) {}
	const char * show();
	operator const char *()  { return p_; }
private:
	const char *p_;
};

//void ShowError(const char *msg);
void AppendText(const GuiEvent &ge /*HWND hDlg, int id, */, const char *pTxt);
bool GetCheckBoxState(const GuiEvent& ge);
void AppendWinTitle(const GuiEvent& ge, const char *p);
void AppendTextToEditCtrl(const GuiEvent & ge, /*HWND hWndEdit,*/ const char * pszText);
void DlgItemPrint(const GuiEvent &ge, const char *pszFmt, ...);

#endif
