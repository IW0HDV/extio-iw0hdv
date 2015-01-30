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

/**
* @file gui.h
* @brief Header for Extio DLL GUI
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/


#if !defined __GUICONTROL_H__
#define		 __GUICONTROL_H__

class GuiImpl;
struct GuiEvent;
struct GuiEventHScroll;

class GuiHpsdr : public Gui {
public:
	GuiHpsdr();
	GuiHpsdr(int resource_id);

	virtual ~GuiHpsdr();

	void setRadio(ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > *pExr);
	ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > * getRadio();

	void setHw(const char *);
	int  getRecNumber(void);
	void setHwAddressGUI(const Ethernet::Device *);

//	virtual void EnableControls();
//	virtual void DisableControls();

protected:
	//void EnableAll(const GuiEvent& ev1, const GuiEvent& ev2);
	// events managers
	virtual bool OnInit(const GuiEvent& ev) = 0;
	virtual bool ButtonClick(const GuiEvent &ev) { return false; }
	virtual bool ButtonDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool ListBoxDoubleClick(const GuiEvent &ev) { return false; }
	virtual bool OnHScroll(const GuiEventHScroll& ev) { return false; }
	virtual bool OnWmUser(int n, const GuiEvent& ev) { return false; }

	Radio *pr;
	ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > *pExr;

};



class HermesGui: public GuiHpsdr {
public:
	HermesGui (int sample_rate);
	~HermesGui () {}

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);

private:
	int sr;
};

class MercuryGui: public GuiHpsdr {
public:
	MercuryGui (int sample_rate);
	~MercuryGui () {}

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
	virtual bool ButtonClick(const GuiEvent &ev);
	virtual bool OnHScroll(const GuiEventHScroll& ev);
	virtual bool OnWmUser(int n, const GuiEvent& ev);

private:
	int sr;
};

class ExtIODll;

class CommandReceiver: public GuiHpsdr {
public:
	CommandReceiver (ExtIODll *);
	~CommandReceiver ();
	virtual bool OnInit(const GuiEvent& ev);
	
	void SendOtherInstancesNewSampleRate (unsigned int nsr);
	void SendOtherInstancesStart ();
	void SendOtherInstancesStop ();
	//void SendOtherInstancesHWLO(long freq);
	void SendOtherInstancesClose ();

	virtual bool OnWmUser(int n, const GuiEvent& ev);

protected:
	
	ExtIODll *pEio;
};

#endif
