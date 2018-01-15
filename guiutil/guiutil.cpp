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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#include <windowsx.h>
#include <commctrl.h>			// Include header
#pragma comment(lib, "comctl32.lib")	// Include library
#pragma warning( disable : 4995 )

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include "util.h"
#include "log.h"
#include "guievent.h"
#include "guiutil.h"

void DlgItemPrint(const GuiEvent &ge, const char *pszFmt, ...)
{
	va_list ap;
	char szBuf[BUFSIZ*4] = { 0 };

	va_start(ap, pszFmt);
	vsnprintf(szBuf, sizeof(szBuf), pszFmt, ap);
	va_end(ap);

    SetWindowText(ge.hWnd, szBuf);
}


const char * GuiError :: show() {
	MessageBox (NULL, (LPCTSTR)p_, TEXT("Error"), MB_OK);
	return p_;
}

void AppendText(const GuiEvent &ge /*HWND hDlg, int id, */, const char *pTxt)
{
	HWND hCtrl = GetDlgItem(ge.hWnd, ge.id);

	int length = GetWindowTextLength(hCtrl);
	if (length > 0) {
		char* tempChar;
		tempChar = (char*)GlobalAlloc(GPTR, length + 1 + strlen(pTxt));
		GetDlgItemText(ge.hWnd, ge.id, (LPSTR)tempChar, length + 1);
		strcpy_s(tempChar + strlen(tempChar), length + 1 + strlen(pTxt), pTxt);
		strcpy_s(tempChar, length + 1 + strlen(pTxt), pTxt);
		SetWindowText(hCtrl, (LPCSTR)tempChar);
	}
}

void AppendTextToEditCtrl(const GuiEvent & ge, const char * pszText)
{
	HWND hCtl = GetDlgItem(ge.hWnd, ge.id);
	int nLength = GetWindowTextLength(hCtl);
	SendMessage(hCtl, EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength);
	SendMessage(hCtl, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText);
}

BOOL GetCheckBoxState(HWND hDlg, int id)
{
	HWND hCtrl = GetDlgItem(hDlg, id);
	if (hCtrl != NULL)
		return (SendMessage(hCtrl, BM_GETSTATE, 0, 0) == BST_CHECKED);
	else
		return FALSE;
}

bool GetCheckBoxState(const GuiEvent& ge)
{
	HWND hCtrl = GetDlgItem(ge.hWnd, ge.id);
	if (hCtrl != NULL)
		return (SendMessage(hCtrl, BM_GETSTATE, 0, 0) == BST_CHECKED);
	else
		return false;
}


void AppendWinTitle(const GuiEvent& ge, const char *p)
{
	char szBuf[1024] = {0};
	char szNew[2028] = {0};
	if (GetWindowText(ge.hWnd, szBuf, sizeof (szBuf)) > 0) {
		snprintf(szNew, sizeof(szNew), "%s %s", szBuf, p);
		SetWindowText(ge.hWnd, szNew);
	}
}

void ResetWinTitle(const GuiEvent& ge, const char *p)
{
	SetWindowText(ge.hWnd, p);
}

