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
 * @file log.cpp
 * @brief Generic log functions
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

#define _DONT_DECLARE_TEMPLATE_
#include "log.h"

#if !defined NDEBUG || defined FLOG


#if defined _WIN32 || defined __MINGW32__
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include "windows.h"
#include "logw.h"    // for log windows resources
#endif

struct LogImpl {
	LogImpl():
#if defined WIN32 && !defined NDEBUG
		hLogDlg(NULL),
#endif
		pLog(0) {}

#if defined WIN32 && !defined NDEBUG
	HWND hLogDlg;
#endif
	FILE *pLog;
};


#if defined _WIN32 && !defined NDEBUG
#include "dllmain.h" // for GetMyHandle()


static void AppendTextToEditCtrl(HWND hWndEdit, LPCTSTR pszText)
{
	int n = GetWindowTextLength(hWndEdit); 

	if (n >= (LOG_DLG_SIZE/2)) {
		// trim 10KB
		SendMessage (hWndEdit, EM_SETSEL,     (WPARAM)10240, (LPARAM)n  );
		SendMessage (hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)"" );
	}
	SendMessage (hWndEdit, EM_SETSEL,     (WPARAM)n,     (LPARAM)n       );
	SendMessage (hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText );

}


BOOL CALLBACK LogDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	    case WM_CREATE:
			SendMessage (GetDlgItem(hwnd, IDC_LOG_TEXT), EM_SETLIMITTEXT, (WPARAM)LOG_DLG_SIZE, (LPARAM)0);
			break;

	    case WM_INITDIALOG:
		    break;

	    case WM_COMMAND:
        // WM_COMMAND is sent when an event occurs
        // the HIWORD of WPARAM holds the notification event code
			break;

		case WM_USER+2:
			{
				Log *pLog = (Log *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

				if (pLog) {
					short flag = static_cast<short>(lParam);
					if (flag) {
						char buff[100];
						time_t now = time(0);
						strftime(buff, 100, "%Y-%m-%d %H:%M:%S ", localtime(&now));
						AppendTextToEditCtrl(GetDlgItem(hwnd, IDC_LOG_TEXT), buff);
					}

					char *pszText = reinterpret_cast<char *>(wParam);
					if (pszText) {
						AppendTextToEditCtrl(GetDlgItem(hwnd, IDC_LOG_TEXT), (LPCTSTR)pszText);
						pLog->xstrdel(pszText, __LINE__);
					}
				}
				else {
					MessageBox(0, "WM_USER+2 null pi", "Warning!", MB_OK | MB_ICONINFORMATION);
				}
			}
            break;
		default:
			return FALSE;
	}
	return TRUE;
}



inline void Log::LogPostMsg(const char *pszText, bool timestamp)
{
	if (pszText && strlen(pszText) && pi && (pi->hLogDlg != NULL)) {
		PostMessage(pi->hLogDlg, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(pszText)), static_cast<LPARAM>(((timestamp == true) ? 1 : 0)));
	}
}

void Log::CreateLogDialog(const char *pszName)
{
	LOGT("********************************* GetMyHandle: %p\n", ::GetMyHandle());

	pi->hLogDlg = CreateDialog(::GetMyHandle(),
								MAKEINTRESOURCE(IDD_LOG_DIALOG),
								HWND_DESKTOP,
								LogDlgProc
								);
	if (pi->hLogDlg != NULL) {
		// setup class instance pointer into Windows object user data
		SetWindowLongPtr(pi->hLogDlg, GWLP_USERDATA, (LONG)this);

		char oldTitle[256];
		char newTitle[1024];
		GetWindowText(pi->hLogDlg, oldTitle, sizeof(oldTitle));
		snprintf(newTitle, sizeof(newTitle), "%s - %s", oldTitle, pszName);
		SetWindowText(pi->hLogDlg, newTitle);
		ShowWindow(pi->hLogDlg, SW_SHOW);
	}
	else {
		//MessageBox(0, "CreateLogDialog returned NULL", "Warning!", MB_OK | MB_ICONINFORMATION);
		ErrorLog("Log::CreateLogDialog");
	}
}

#endif

void Log::open(const char *pszLogFileName, int n)
{
	if (pi && pi->pLog == 0 && pszLogFileName && strlen(pszLogFileName)) {
		char buf[1024] = {0};
		snprintf(buf, sizeof(buf), "%s-%d.log", pszLogFileName, n);
		pi->pLog = fopen(buf, "w+");
#if defined WIN32 && !defined NDEBUG
		CreateLogDialog(buf);
#endif
		char szBuf[1024] = { 0 };
		GetCurrentDirectory(sizeof(szBuf), szBuf);
		LOGX("Current directory: [%s]\n", szBuf);
	}
}

void Log::close(void)
{
#if defined WIN32 && !defined NDEBUG
	if (pi && pi->hLogDlg) {
		fprintf (pi->pLog, "Destroying log window: [%p]\n", pi);
		DestroyWindow (pi->hLogDlg);
	}
#endif	
	if (pi && pi->pLog != 0) {
		fflush(pi->pLog);
		fclose(pi->pLog);
	}

}

Log :: Log()
{
	pi = new LogImpl;
}
Log :: ~Log()
{
	close();
}

void Log::log_funcname_printf(int f, const char *pszFunctionName, int nLine, const char *pszFmt, ...)
{
	if (!pi && !pi->pLog) {
		return;
	}
	va_list ap;
	char szFmt[BUFSIZ];
	char szBuf[BUFSIZ];

	snprintf(szFmt, sizeof(szFmt), "%s:%d - %s", pszFunctionName, nLine, pszFmt);

	va_start(ap, pszFmt);
	vsprintf(szBuf, szFmt, ap);
	va_end(ap);

	fprintf(pi->pLog, "%s", szBuf);
	fflush(pi->pLog);
#if CONSOLE
	fprintf(stdout, "%s", szBuf);
	fflush (stdout);
#endif	
#if defined WIN32 && !defined NDEBUG
	if (f) LogPostMsg(szBuf, true);
#endif
}

void Log::log_printf(const char *format, ...)
{
	if (!pi && !pi->pLog) {
		return;
	}
	va_list ap;
	char szBuf[BUFSIZ];

	va_start(ap, format);
	vsprintf(szBuf, format, ap);
	va_end(ap);
	fprintf(pi->pLog, "%s", szBuf);
	fflush(pi->pLog);
#if CONSOLE
	fprintf(stdout, "%s", szBuf);
	fflush(stdout);
#endif
#if defined WIN32 && !defined NDEBUG
	LogPostMsg(szBuf, false);
#endif
}

void Log::log_printf_mod(const char *pszFile, int nLine)
{
	if (!pi && !pi->pLog) {
		return;
	}	char szBuf[BUFSIZ];

	snprintf(szBuf, sizeof(szBuf), "%s: %4.4d: ", pszFile, nLine);
	fprintf(pi->pLog, "%s", szBuf);
#if CONSOLE
	fprintf(stdout, "%s", szBuf);
	fflush(stdout);
#endif
#if defined WIN32 && !defined NDEBUG
	LogPostMsg(szBuf, true);
#endif

}


//Log LLog;

#endif


#if 0
static HWND hLogDlg = 0;

void log_CreateLogDialog(const char *pszName)
{
	hLogDlg = CreateDialog(GetMyHandle(),
		MAKEINTRESOURCE(IDD_LOG_DIALOG),
		NULL,
		LogDlgProc);
	if (hLogDlg != NULL)	{
		char oldTitle[256];
		char newTitle[1024];
		GetWindowText(hLogDlg, oldTitle, sizeof(oldTitle));
		snprintf(newTitle, sizeof(newTitle), "%s - %s", oldTitle, pszName);
		SetWindowText(hLogDlg, newTitle);
		ShowWindow(hLogDlg, SW_SHOW);
	}
	else {
		MessageBox(0, "CreateDialog returned NULL", "Warning!", MB_OK | MB_ICONINFORMATION);
	}
}


inline void LogPostMsg(const char *pszText, bool timestamp)
{
	if (pszText && strlen(pszText) && hLogDlg) {
		PostMessage(hLogDlg, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(pszText)), static_cast<LPARAM>(((timestamp == true) ? 1 : 0)));
	}
}



void log_open(const char *pszLogFileName)
{
	if (pLog == 0 && pszLogFileName) {
		char buf[1024];
		snprintf(buf, sizeof(buf), "%s-%d.log", pszLogFileName, GetInstanceNumber());
		pLog = fopen(buf, "w+");
#if defined WIN32 && !defined NDEBUG
		log_CreateLogDialog(buf);
#endif
		char szBuf[1024] = { 0 };
		GetCurrentDirectory(sizeof(szBuf), szBuf);
		LOGX("Current directory: [%s]\n", szBuf);
	}
}
void log_close(void)
{
	if (pLog != 0) {
		fflush(pLog);
		fclose(pLog);
	}
#if defined WIN32 && !defined NDEBUG
	if (hLogDlg) DestroyWindow(hLogDlg);
#endif
}
void log_funcname_printf(int f, const char *pszFunctionName, int nLine, const char *pszFmt, ...)
{
	va_list ap;
	char szFmt[BUFSIZ];
	char szBuf[BUFSIZ];

	snprintf(szFmt, sizeof(szFmt), "%s:%d - %s", pszFunctionName, nLine, pszFmt);

	va_start(ap, pszFmt);
	vsprintf(szBuf, szFmt, ap);
	va_end(ap);

	fprintf(pLog, "%s", szBuf);
	fflush(pLog);
#if defined WIN32 && !defined NDEBUG
	if (f) LogPostMsg(szBuf, true);
#endif
}

void log_printf(const char *format, ...)
{
	va_list ap;
	char szBuf[BUFSIZ];

	va_start(ap, format);
	vsprintf(szBuf, format, ap);
	va_end(ap);
	fprintf(pLog, "%s", szBuf);
	fflush(pLog);
#if defined WIN32 && !defined NDEBUG
	LogPostMsg(szBuf, false);
#endif
}

void log_printf_mod(const char *pszFile, int nLine)
{
	char szBuf[BUFSIZ];

	snprintf(szBuf, sizeof(szBuf), "%s: %04.4d: ", pszFile, nLine);
	fprintf(pLog, "%s", szBuf);
#if defined WIN32 && !defined NDEBUG
	LogPostMsg(szBuf, true);
#endif

}

void test()
{
	const char *ss = "sssssssssss";
	int ii = 33;

	//SLOG_OPEN("xxxxxxxxxxx");
	//SLOGT("%s %d\n", ss, ii);
	//SLOG_CLOSE;
}
#endif
