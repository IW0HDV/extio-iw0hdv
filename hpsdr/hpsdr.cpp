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
 * @file hpsdr.cpp
 * @brief HPSDR Hermes modeling classes
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#if defined _MSC_VER
#include <strsafe.h>
#endif


#include "util.h"
#include "hpsdr.h"
#include "log.h"

//
// Ethernet class data
//
#pragma data_seg (".SS_HPSDR")
struct Ethernet::Device Ethernet :: devs [MAX_DEVICES] SHARED = {0};
struct Ethernet::NetInterface Ethernet :: interfaces[MAX_INTERFACES] SHARED = {0};
int Ethernet :: nif  SHARED = 0;
int Ethernet :: dev_found SHARED = 0;
#pragma data_seg ()

struct Ethernet::Device * Ethernet :: search_dev_by_ip (const char * ip)
{
	for (unsigned i = 0; i < ARRAY_SIZE(devs); ++i)
		if (strcmp(ip, devs[i].ip_address) == 0) return &(devs[i]);
	return 0;
}

std::list < struct Ethernet::NetInterface > Ethernet :: getInterfaceList ()
{
	std::list < struct NetInterface > rl;

	for (int i = 0; i < nif; ++i) rl.push_back (interfaces[i]);
	return rl;
}

std::list < struct Ethernet::Device > Ethernet :: getDeviceList ()
{
	std::list < struct Device > dl;

	for (int i = 0; i < dev_found; ++i) dl.push_back (devs[i]);
	return dl;
}

struct Ethernet::Device * Ethernet :: found (int n)
{
	if (dev_found) {
		if (n < dev_found)
			return &devs[dev_found-1];
		else
			return &devs[0];
	}
	return 0;
}

//
// constants for scan_interface
//

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

//
// http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancediphelperfunction13i.html
//

int Ethernet :: scan_interface (int /* x */, char * /* ifName */  )
{
	/* Declare and initialize variables */

    //DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i = 0;

	int found_interface = 0;

	//int                nRet;
    //struct hostent    *phe;
    //struct in_addr    *paddr;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    //PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    //PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    //IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    //IP_ADAPTER_PREFIX *pPrefix = NULL;

    family = AF_INET; // IP v4 only !

	LOGT("%s", "Calling GetAdaptersAddresses function with family = ");
	if (family == AF_INET) {LOGT("%s\n", "AF_INET");}
    if (family == AF_INET6) {LOGT("%s\n", "AF_INET6");}
    if (family == AF_UNSPEC) {LOGT("%s\n", "AF_UNSPEC");}

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
        if (pAddresses == NULL) {
            LOGT("%s\n", "Memory allocation failed for IP_ADAPTER_ADDRESSES struct");
            return 0;
        }

        dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            free(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
		while (pCurrAddresses && !found_interface) {
			LOGT("Length of the IP_ADAPTER_ADDRESS struct: %ld\n",
				pCurrAddresses->Length);
			LOGT("IfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
			LOGT("Adapter name: %s\n", pCurrAddresses->AdapterName);
			LOGT("Friendly name: %wS\n", pCurrAddresses->FriendlyName);
			pUnicast = pCurrAddresses->FirstUnicastAddress;
			if (pUnicast != NULL) {
				LOGT("ADDRESS: %s\n", inet_ntoa(((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr));
#if 0
				if ((!wcscmp(pCurrAddresses->FriendlyName, L"Ethernet"))
					&&
					(strcmp(inet_ntoa ( ((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr ), "127.0.0.1") != 0)) found_interface = 1;
#endif
				memcpy(&(interfaces[nif].b_ip_address), &(((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr), sizeof(struct in_addr));
				strcpy(interfaces[nif].ip_address, inet_ntoa(((struct sockaddr_in *)(pUnicast->Address.lpSockaddr))->sin_addr));
				for (i = 0; pUnicast != NULL; i++)
					pUnicast = pUnicast->Next;
				LOGT("Number of Unicast Addresses: %d\n", i);
			}
			else {
				LOGT("%s", "No Unicast Addresses\n");
			}

			if (pCurrAddresses->PhysicalAddressLength != 0) {

				for (i = 0; i < 6; i++) interfaces[nif].hw_address[i] = pCurrAddresses->PhysicalAddress[i];

				LOGT("Physical address: %02X-%02X-%02X-%02X-%02X-%02X\n", (interfaces[nif].hw_address[0]) & 0xFF, (interfaces[nif].hw_address[1]) & 0xFF, (interfaces[nif].hw_address[2]) & 0xFF,
					(interfaces[nif].hw_address[3]) & 0xFF, (interfaces[nif].hw_address[4]) & 0xFF, (interfaces[nif].hw_address[5]) & 0xFF
					);
			}
			LOGT("Friendly name: [%wS]\n", pCurrAddresses->FriendlyName);
			#if defined _MSC_VER
			{
				// Convert to a char*
				size_t origsize = wcslen(pCurrAddresses->FriendlyName) + 1;
				const size_t newsize = sizeof (interfaces[nif].name);
				size_t convertedChars = 0;
				wcstombs_s(&convertedChars, interfaces[nif].name, wcslen(pCurrAddresses->FriendlyName) + 1, pCurrAddresses->FriendlyName, _TRUNCATE);
			}
			#else 
			#if defined __MINGW32__
			{
				// Convert to a char*
				size_t origsize = wcslen(pCurrAddresses->FriendlyName) + 1;
				//const size_t newsize = sizeof (interfaces[nif].name);
				//size_t convertedChars = 0;
				wcstombs(interfaces[nif].name, pCurrAddresses->FriendlyName, origsize);
			}
			//strcpy (interfaces[nif].name, "N/A");
			#endif
			#endif
			LOGT("Status: %d\n", pCurrAddresses->OperStatus);
			{
				interfaces[nif].status = pCurrAddresses->OperStatus;
			}

			nif++;
			pCurrAddresses = pCurrAddresses->Next;
        }
	
    } else {
        LOGT("Call to GetAdaptersAddresses failed with error: %d\n", dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            LOGT("%s", "No addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                    NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   
                    // Default language
                    (LPTSTR) & lpMsgBuf, 0, NULL)) {
                LOGT("Error: %s", lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    free(pAddresses);
                return 0;
            }
        }
    }

    if (pAddresses) {
        free(pAddresses);
    }
	return nif;
}

#define PORT 1024
#define DISCOVERY_SEND_PORT PORT
#define DISCOVERY_RECEIVE_PORT PORT
#define DATA_PORT PORT

bool Ethernet :: scan_devices (ScanWatcher *psw)   // was discover()
{
    int rc;
    int i;
	//static int data_socket = -1;
	//static struct sockaddr_in data_addr;
	//static int data_addr_length;
	static unsigned char buffer[70];

	psw->ScanStarted();
    LOGT("%s\n", "Looking for Metis/Hermes/Angelia card on all interfaces");

//    discovering=1;
	
    // get my MAC address and IP address
    if ( scan_interface (0, 0) <= 0) return false;

		for (int n=0; n < nif; n++) {

			LOGT("Interface [%s]: IP Address: %s STATUS: %d\n", interfaces[n].name, interfaces[n].ip_address, interfaces[n].status);

            LOGT("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                interfaces[n].hw_address[0]&0xFF, interfaces[n].hw_address[1]&0xFF, interfaces[n].hw_address[2]&0xFF, 
				interfaces[n].hw_address[3]&0xFF, interfaces[n].hw_address[4]&0xFF, interfaces[n].hw_address[5]&0xFF );

			psw->InterfaceFound(&(interfaces[n]));

			// bind to this interface
		    struct sockaddr_in name = {0} ;
			if (interfaces[n].status != IfOperStatusUp) { 
				LOGT("INOPERATIVE: Interface [%s]: IP Address: %s\n", interfaces[n].name, interfaces[n].ip_address);
				continue;
			}

			struct sockaddr_in discovery_addr = {0} ;
            int discovery_length = 0 ;

			interfaces[n].d_socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (interfaces[n].d_socket < 0) {
				LOGT("create socket failed for d_socket %s\n", strerror(errno));
				continue;
			}

			name.sin_family      = AF_INET;
			name.sin_addr.s_addr = inet_addr(interfaces[n].ip_address);
			name.sin_port        = htons(DISCOVERY_SEND_PORT);
			bind (interfaces[n].d_socket, (struct sockaddr*)&name, sizeof(name));


		    // allow broadcast on the socket
			int broadcast = 1;
			rc = setsockopt (interfaces[n].d_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast));
			if(rc != 0) {
				LOGT("cannot set SO_BROADCAST: rc=%d\n", rc);
				return false;
			}
			int rcv_tmo = 10;   // reading answers with 10 ms timeout
			rc = setsockopt (interfaces[n].d_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&rcv_tmo, sizeof(int));
			if ( rc == SOCKET_ERROR ) {
                LOGT("setsockopt SO_RCVTIMEO failed! [%s]\n", strerror(errno));
			}

			discovery_length=sizeof(discovery_addr);
			memset(&discovery_addr,0,discovery_length);
			discovery_addr.sin_family=AF_INET;
			discovery_addr.sin_port=htons(DISCOVERY_SEND_PORT);
			discovery_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

			for (int x=0; x<1; ++x) {  // two attempts only !!!!!
				// broadcast the discovery message 
				buffer[0]=0xEF;
				buffer[1]=0xFE;
				buffer[2]=0x02;         
				for (i=0; i<60; i++) buffer[i+3]=0x00;

				if (sendto(interfaces[n].d_socket, (const char *)buffer,63,0,(struct sockaddr*)&discovery_addr,discovery_length)<0) {
					LOGT("sendto socket failed for discovery_socket: [%s]\n", strerror(errno));
					continue;
				} else {
					LOGT("#%d discovery message sent.\n", x);
				} 


				struct sockaddr_in from_addr;
				unsigned long my_ip_address = inet_addr(interfaces[n].ip_address);
				int length;
				int bytes_read;
				//char *rc = 0;

				for (int nr = 0; nr < 3; ++nr) {
					unsigned char inp_buf[1024];

					length = sizeof(from_addr);

   					bytes_read = recvfrom (interfaces[n].d_socket, (char *) inp_buf, sizeof(inp_buf), 0, (struct sockaddr*)&from_addr, &length);
					if (bytes_read < 0) {
						LOGT("Reading discovery: %s\n", strerror(errno));
						continue;
					}
					//LOG(("RECEIVED %d bytes: from [%s]\n", bytes_read, inet_ntoa ( ((struct sockaddr_in *)&from_addr)->sin_addr)) );

					if (memcmp ( &(((struct sockaddr_in *)&from_addr)->sin_addr), &my_ip_address, sizeof(my_ip_address)) == 0) {
						LOGT("%s\n", "WARNING: ignoring fake answer coming from ourselves !");
						continue;
					}

					if (inp_buf[0]==0xEF && inp_buf[1]==0xFE) {
						switch(inp_buf[2]) {
							case 1:
								LOGT("%s", "unexpected data packet when in discovery mode\n");
								break;
							case 2:  // response to a discovery packet - hardware is not yet sending
							case 3:  // response to a discovery packet - hardware is already sending
								if (dev_found < MAX_DEVICES) {
									struct Device card;

									if (inp_buf[3] == 0 && inp_buf[4] == 0 && inp_buf[5] == 0 &&
										inp_buf[6] == 0 && inp_buf[7] == 0 && inp_buf[8] == 0) {
										LOGT("%s", "NULL MAC address in answer, skipping\n");
										break;
									}

									// get MAC address from reply
									snprintf(card.mac_address, sizeof(card.mac_address), "%02X:%02X:%02X:%02X:%02X:%02X",
                                            inp_buf[3]&0xFF, inp_buf[4]&0xFF, inp_buf[5]&0xFF, inp_buf[6]&0xFF, inp_buf[7]&0xFF, inp_buf[8]&0xFF);
									LOGT("Radio MAC address %s\n", card.mac_address);
    
									// get ip address from packet header
									snprintf(card.ip_address, sizeof(card.ip_address), "%lu.%lu.%lu.%lu",
											 from_addr.sin_addr.s_addr      & 0xFF,
											(from_addr.sin_addr.s_addr>>8)  & 0xFF,
											(from_addr.sin_addr.s_addr>>16) & 0xFF,
											(from_addr.sin_addr.s_addr>>24) & 0xFF);
									LOGT("Radio IP address %s\n", card.ip_address);
									card.code_version = inp_buf[9];
									// Board_ID = 1 byte,
									// from "Metis - How it works", 1.33, 28 Feb 2015
									// http://svn.tapr.org/filedetails.php?repname=OpenHPSDR+Main&path=%2Ftrunk%2FMetis%2FDocumentation%2FMetis-+How+it+works_V1.33.pdf
									// 0x00 = Metis,
									// 0x01 = Hermes,
									// 0x02 = Griffin,
									// 0x04 = Angelia,
									// 0x05 = Orion,
									// 0x06 = Hermes_Lite
									switch (inp_buf[10]) {
										case 0x00:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Metis" );
											break;
										case 0x01:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Hermes" );
											break;
										case 0x02:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Griffin" );
											break;
										case 0x04:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Angelia" );
											break;
										case 0x05:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Orion" );
											break;
										case 0x06:
											snprintf (card.board_id, sizeof(card.board_id), "%s", "Hermes_Lite" );
											break;
										default: 
											snprintf (card.board_id, sizeof(card.board_id), "%s", "unknown" );
											break;
									}       
									LOGT("***** Board id: %s (%02X)\n", card.board_id, inp_buf[10]);
									LOGT("***** version:  %1.2f\n", card.code_version /10.0);

									card.b_card_ip_address = interfaces[n].b_ip_address;

									// check if this device was already discovered
									if ( search_dev_by_ip (card.ip_address) == 0) {
										devs[dev_found] = card;
										psw->DeviceFound(&(devs[dev_found]));
										dev_found++;
									} else {
										LOGT("%s", "Duplicated response, discard\n");
									}


									if(inp_buf[2]==3) {
										LOGT("%s", "Radio is sending\n");
									}
								} else {
									LOGT("%s", "too many radio cards!\n");
									break;
								}
							break;
							default:
								LOGT("unexpected packet type: 0x%02X\n",inp_buf[2]);
								break;
							}
						} else {
							LOGT("received bad header bytes on data port %02X,%02X\n", inp_buf[0], inp_buf[1] );
						}
				}
	
		}
		closesocket (interfaces[n].d_socket);
	}
	for (int x = 0; x < dev_found; ++x) {
		LOGT("%d: ip: %s mac: %s bid: %s code ver: %d card ip: %p\n", x,
			devs[x].ip_address, devs[x].mac_address, devs[x].board_id, devs[x].code_version, devs[x].b_card_ip_address);
	}
	psw->ScanStopped(dev_found);
	return true;
}

void	Ethernet :: startReceive (struct Device *p)
{
    int i;
    int rc;
    struct hostent *h;
	unsigned char buffer [64];

    LOGT("%s", "starting receive thread\n");

    //discovering = 0;

    h = gethostbyname (p->ip_address);
    if (h == NULL) {
        LOGT("unknown host %s\n", p->ip_address);
        return;
    } else {
        LOGT("host %s\n", p->ip_address);
	}

	//
	// check if socket is yet available (after a STOP)
	//
	if (data_socket == -1) {
		struct sockaddr_in xname = {0} ;
//		int on;

		data_socket = socket (PF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (data_socket < 0) {
			LOGT("BAD SOCKET !!!!!! [%s]\n", strerror(errno));
			return;
		} else {
			LOGT("Socket created: %d\n", data_socket);
		}
		// bind to this interface
		xname.sin_family = AF_INET;
		xname.sin_addr.s_addr = p->b_card_ip_address;
		xname.sin_port = htons(DISCOVERY_SEND_PORT);
		rc = bind (data_socket, (struct sockaddr*)&xname, sizeof(xname));
		if (rc != 0) {
			LOGT("BIND on %s FAILED: %s\n", inet_ntoa(xname.sin_addr), strerror (errno));
		} else {
			LOGT("BIND to %s successfull\n", inet_ntoa(xname.sin_addr));
		}

		// extablish a sort of a receive queue
		int bsize = 102400000;
		rc = setsockopt (data_socket, SOL_SOCKET, SO_RCVBUF, (const char *)&bsize, sizeof(bsize));
		if(rc != 0) {
			LOGT("cannot set SO_RCVBUF: rc=%d\n", rc);
		}

	    // start a receive thread to get discovery responses
	    rc = pthread_create ( &receive_thread_id, NULL, receive_thread, (void *)this);
		if (rc != 0) {
			LOGT("pthread_create failed on receive thread: rc=%d\n", rc);
			return;
		} else {
			LOGT("%s\n", "receive thread: thread succcessfully started");
		}
	}

    data_addr_length = sizeof(data_addr);
    memset (&data_addr, 0, data_addr_length);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port   = htons(DATA_PORT);
    memcpy((char *)&data_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);

	pFlow->initialization(this);
	LOGT("%s", "Flow initialization done\n");

	Sleep(300);

    // send a packet to start the stream
    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x04;    // data send state send (0x00=stop)
    buffer[3]=0x01;    // I/Q only

    for (i=0; i<60; i++) buffer[i+4] = 0x00;

    send_buffer (data_socket, &buffer[0], 64);
    LOGT("%s", "START COMMAND SENT\n");

	LOGT("%s", "starting watchdog_thread\n");
	watchdog_timeout_in_ms = 250;
    // start a watchdog to make sure we are receiving frames
    rc = pthread_create ( &watchdog_thread_id, NULL, watchdog_thread, (void *)this);
    if(rc != 0) {
        LOGT("pthread_create failed on watchdog thread: rc=%d\n", rc);
        return;
    }

}


//static unsigned char input_buffer[20480];


void* Ethernet :: receive_thread (void* arg) 
{
	Ethernet *pTgt = (Ethernet *)arg;
	
	struct sockaddr_in addr;
	int length;
	int bytes_read;
	
	const char *p_fatal_error = 0;
	static const char *sync_err = "synch error";
	static const char *unexp_pt = "unexpected packet type";
	static const char *exiting = "exiting...";
	static const char *unexp_eplen = "unexpected EP lenght" ;
	int loop = 1;

	length = sizeof(addr);
	while (loop) {
		bytes_read = recvfrom (pTgt->data_socket, (char *) pTgt->input_buffer, sizeof(pTgt->input_buffer), 0, (struct sockaddr*)&addr, &length);
        if(bytes_read < 0) {
			p_fatal_error = strerror (errno);
			LOGT("recvfrom socket failed for receive_thread: [%s]\n", p_fatal_error);
            break;
        }
		//LOGT("RECEIVED %d bytes: from [%s]\n", bytes_read, inet_ntoa ( ((struct sockaddr_in *)&addr)->sin_addr));

        if (pTgt->input_buffer[0] == 0xEF && pTgt->input_buffer[1] == 0xFE) {
            switch (pTgt->input_buffer[2]) {
                case 1:
                    {
                        // get the end point
                        pTgt->ep = pTgt->input_buffer[3] & 0xFF;

                        // get the sequence number
                        pTgt->sequence = ((pTgt->input_buffer[4]&0xFF)<<24)+((pTgt->input_buffer[5]&0xFF)<<16)+((pTgt->input_buffer[6]&0xFF)<<8)+(pTgt->input_buffer[7]&0xFF);
                        //LOGT("received data ep=%d sequence=%ld\n", pTgt->ep, pTgt->sequence);

                        switch (pTgt->ep) {
                            case 6:
                                // process the data
								if ( pTgt->pFlow->processFromRadio (&(pTgt->input_buffer[8])) < 0) {
									p_fatal_error = sync_err;
									loop = 0;
								}
								if ( pTgt->pFlow->processFromRadio (&(pTgt->input_buffer[520])) < 0) {
									p_fatal_error = sync_err;
									loop = 0;
								}
                                break;
                            case 4:
                                //LOGT("EP4 data\n");
                                break;
                            default:
                                LOGT("unexpected EP %d length=%d\n", pTgt->ep, bytes_read );
								p_fatal_error = unexp_eplen;
								loop = 0;
                                break;
                        }
                    }
                    break;
                case 2:  // response to a discovery packet - hardware is not yet sending
                case 3:  // response to a discovery packet - hardware is already sending
                    LOGT("%s", "unexepected discovery response when not in discovery mode\n");
                    break;
                default:
                    LOGT("unexpected packet type: 0x%02X\n", pTgt->input_buffer[2]);
					p_fatal_error = unexp_pt;
					loop = 0;
					break;
            }
        } else {
            LOGT("received bad header bytes on data port %02X,%02X\n", pTgt->input_buffer[0], pTgt->input_buffer[1] );
        }

    }
	if (p_fatal_error) {
		pTgt->FatalError(p_fatal_error);
		return (void *)p_fatal_error;
	} else
		return (void *)exiting;    
}

int Ethernet :: write (unsigned char ep, unsigned char* buffer, int length) {
    int i;

    if (offset==8) {
        send_sequence++;
        output_buffer[0]=0xEF;
        output_buffer[1]=0xFE;
        output_buffer[2]=0x01;
        output_buffer[3]=ep;
        output_buffer[4]=(send_sequence>>24)&0xFF;
        output_buffer[5]=(send_sequence>>16)&0xFF;
        output_buffer[6]=(send_sequence>>8)&0xFF;
        output_buffer[7]=(send_sequence)&0xFF;

        // copy the buffer over
        for (i = 0; i < 512; i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=520;
    } else {
        // copy the buffer over
        for ( i = 0; i < 512; i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=8;

        // send the buffer
        send_buffer (data_socket, &output_buffer[0], 1032);
    }
    return length;
}

void Ethernet :: send_buffer (int s, unsigned char* buffer, int length) {
//LOGT("length=%d\n",length);
    if (sendto (s, (const char *)buffer, length, 0, (struct sockaddr*)&data_addr,data_addr_length)<0) {
		LOGT("sendto socket failed: %s\n", strerror(errno));
	}
}


void* Ethernet :: watchdog_thread (void* arg) 
{
	Ethernet *pTgt = (Ethernet *)arg;
	long last_sequence=-1;
	static const char *pTmoMsg = "Timeout receiving packets from hardware.";

	// check if packets received
	LOGT ("%s\n", "watchdog_thread running");
	while(1) {
		//LOGT("watchdog sleeping...\n");
		Sleep (pTgt->watchdog_timeout_in_ms);
		if ( pTgt->sequence == last_sequence ) {
			LOGT("No packets for %d ms: sequence=%ld\n", pTgt->watchdog_timeout_in_ms, pTgt->sequence );
			break;
		}
		last_sequence = pTgt->sequence;
	}

	pTgt->TransmissionTmo (pTmoMsg);
	LOGT("%s\n", pTmoMsg);
	return 0;
}

void	Ethernet :: stopReceive  ()
{
	void *rc;

	unsigned char xbuffer[64] = {0};

	// set up a packet to stop the stream
    xbuffer[0]=0xEF;
    xbuffer[1]=0xFE;
    xbuffer[2]=0x04;    // data send state (0x00=stop)
    xbuffer[3]=0x00;    // I/Q only STOP 

	//for (int i=0; i<60; i++) xbuffer[i+4]=0x00;

	// send stop command
	send_buffer(data_socket, &xbuffer[0], sizeof(xbuffer));
	LOGT("%s\n", "Stop command sent");
	// wait for watchdog to detect that hw has really stopped
	pthread_join (watchdog_thread_id, &rc);
	LOGT("%s\n", "Destroying receiver thread....");
	// destroy receiver thread
	closesocket (data_socket);
	pthread_join (receive_thread_id, &rc);
	data_socket = -1;

	LOGT("%s\n", (char *)rc);
}


#if 0
template < class T , int  N_SAMPLES, int N_BLOCKS >
bool Receiver< T , N_SAMPLES, N_BLOCKS> :: is_buffer_full (bool process_tx = false)
{
	// when we have enough samples send them to the clients
	if ( ns == N_SAMPLES ) {
		int d=0;
		// copy the samples into the output buffer
		for (int j=0; j < N_SAMPLES; j++)
			cb_buffer[ni][nb*(N_SAMPLES*2) + d++] = input_buffer[j+N_SAMPLES],  // I part of sample
			cb_buffer[ni][nb*(N_SAMPLES*2) + d++] = input_buffer[j];              // Q part of sample
			nb++;
		if (nb >= N_BLOCKS) {
			pr->process_iq_from_rx ((unsigned char *) cb_buffer[ni] , sizeof(cb_buffer[0]));
			// flips buffers
			nb = 0;
			ni = (ni + 1) % 2;
		}
		if (process_tx)
			pr->process_iq_audio_to_radio ( (unsigned char *)&output_buffer[0],           (unsigned char *)&output_buffer[N_SAMPLES], 
											(unsigned char *)&output_buffer[N_SAMPLES*2], (unsigned char *)&output_buffer[N_SAMPLES*3] );
		ns = 0;
		return true;
	} else
		return false;
}
#endif

const int Flow :: usableBufLen [9] = {
		0,			// filler
		512 - 0,	// 1 RX
		512 - 0,	// 2 RX
		512 - 4,
		512 - 10,
		512 - 24,
		512 - 10,
		512 - 20,
		512 - 4		// 8 RX
	};



int Flow :: processFromRadio  (unsigned char *b)
{
	// check for synchronization sequence
	if (!((b[0] == SC && b[1] == SC && b[2] == SC))) {
		LOGT("%s\n", "!!!!!!!!!!!!!!!!! SYNC ERROR !!!!!!!!!!!!!!!!!");
		DumpHpsdrBuffer ("SYNC ERROR", nrxp, b);
		return -1;
	}
	// process control data
	pr->getControlData((CtrlBuf *)(&b[3]));

	int n = 8;   // start to extract data after synch sequence (3) and control data (5)

	// extract the samples
	while ( n < usableBufLen [pr->getNumberOfRx()] ) {
		//int ls; // left sample
		//int rs; // right sample
		//int ms; // mic sample

		// extract each of the receivers samples data
		for (int r = 0; r < pr->getNumberOfRx(); r++) {
#if 0
			// samples from hardware are 24 bit signed integer
			// put them in a regular int
			ls  = (int)((signed char)   b[n++]) << 16;
			ls += (int)((unsigned char) b[n++]) << 8 ;
			ls += (int)((unsigned char) b[n++])      ;
			rs  = (int)((signed char)   b[n++]) << 16;
			rs += (int)((unsigned char) b[n++]) << 8 ;
			rs += (int)((unsigned char) b[n++])      ;
			// next, rescale to 32 bit
			ls <<= 8; rs <<= 8;
#endif
			HpsdrRxIQSample *pi =  (HpsdrRxIQSample *) &b[n];
			n = n + 3;
			HpsdrRxIQSample *pq =  (HpsdrRxIQSample *) &b[n];
			n = n + 3;
			// append into receiver buffer
			pr->rx[r].append_input_iq ( *pq, *pi ) ;
			if (r) pr->rx[r].next_sample();
		}
		// send to dspserver
		//ms  = (int)((signed char)   b[n++]) << 8;
		//ms += (int)((unsigned char) b[n++]);
		HpsdrMicSample *pms = (HpsdrMicSample *) &b[n];
		n = n + 2;
		pr->rx[0].append_input_mic (*pms);
		pr->rx[0].next_sample();

		// check for buffer full
		for (int r = 0; r < pr->getNumberOfRx(); r++) 
			if (r == 0 ) {
				int bf = pr->rx[r].is_buffer_full (r == 0);	// the receiver 0 contains the data for output stream
															// so signals it in order to trigger the output stream
				if (bf) processToRadio (0);
			} else {
				pr->rx[r].is_buffer_full (0);
			}
	}

	nrxp++;
	return 0;
}


int nxx = 0;

void Flow :: processToRadio (unsigned char *b)
{
	ob[0] = SC;
	ob[1] = SC;
	ob[2] = SC;

	CtrlBuf *pcb = (CtrlBuf *) &(ob [3]);

	pcb->c[0] = send_status << 1;
	pcb->c[1] = pcb->c[2] = pcb->c[3] = pcb->c[4] = 0;

	pr->setControlData(pcb);

	//
	// fill in TX I/Q and audio data TBD
	//
	// TBD
	for (unsigned n = 8; n < sizeof(ob); ++n) ob[n] = 0;

	if ( (nxx < 32) /* || send_status == 2  */ ) { DumpHpsdrHeader ("sending to radio", 0 , ob); }
	nxx++;

	// send the buffer to hardware
	pl->write(0x02, ob, sizeof(ob));


	// simple round robin for control data TBD
	++send_status;
	if (send_status > 11) send_status = 0;
}


void Flow :: initialization (Link *pL)
{
	pl = pL;

	// send frames in order to initialize hardware (priming)
	// TBD
	for (int i = 0; i <16; ++i) {
		processToRadio (0);
		Sleep (10);
	}
}

#if 1
template < int  N_SAMPLES >
bool Receiver<N_SAMPLES> :: is_buffer_full (bool process_tx)
{
	// when we have enough samples send them to the clients
	if ( ns == N_SAMPLES ) {
		pr->process_iq_from_rx ( ni, input_buffer_i, input_buffer_q, N_SAMPLES);
		if (process_tx) 
			pr->process_iq_audio_to_radio ( (unsigned char *)&output_buffer[0],           (unsigned char *)&output_buffer[N_SAMPLES], 
			(unsigned char *)&output_buffer[N_SAMPLES*2], (unsigned char *)&output_buffer[N_SAMPLES*3] );
		ns = 0;
		return true;
	} else {
		return false;
	}
}
#endif

int		Radio :: getNumberOfRx () { return n_rx; }
void	Radio :: setNumberOfRx(int n) { LOGT("# of receivers set to: %d\n", n);  n_rx = n; }


// utilities

void DumpHpsdrBuffer (const char* rem, int np, const unsigned char* b) {
	int i;
	LOGT("%s: packet #%d\n", rem, np);
	for(i = 0; i < Flow::O_BUF_SIZE; i += 16) {
		LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
				i,
				b[i+0], b[i+1],  b[i+2],  b[i+3],  b[i+4],  b[i+5],  b[i+6],  b[i+7],
				b[i+8], b[i+9],  b[i+10], b[i+11], b[i+12], b[i+13], b[i+14], b[i+15]
			);
	}
	LOGT("%s","\n");
}


void DumpHpsdrHeader (const char* remark, int np, const unsigned char* b) {
	LOGT("%s: packet #%d\n", remark, np);
    int i = 0;
    LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
            i,
			b[i+0], b[i+1],  b[i+2],  b[i+3],  b[i+4],  b[i+5],  b[i+6],  b[i+7],
			b[i+8], b[i+9],  b[i+10], b[i+11], b[i+12], b[i+13], b[i+14], b[i+15]
		);
    LOGT("%s","\n");
}

