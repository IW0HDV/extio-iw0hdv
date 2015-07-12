/** 
* @file hpsdr_dump.cpp
* @brief Utility functions,partially derived from the util.c in ghpsdr3 package
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-05
*/

#if !defined NDEBUG
#include <stdlib.h>
#if defined _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include <stdio.h>
#include <string.h>
#include "log.h"
#include "util.h"

#define OZY_BUFFER_SIZE 512

/** 
* @brief Dump Ozy buffer
* 
* @param prefix
* @param buffer
*/
void dump_ozy_buffer(char* prefix,int frame,unsigned char* buffer) {
    int i;
    LOGT("%s(%d)\n",prefix,frame);
    for(i=0;i<OZY_BUFFER_SIZE;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_ozy_header(char* prefix,int frame,unsigned char* buffer) {
    int i;
    LOGT("%s(%d)\n",prefix,frame);
    i=0;
    LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
            i,
            buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
            buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
            );
    LOGT("%s","\n");
}

void dump_iq_buffer(unsigned char* buffer) {
    int i;
    LOGT("%s", "iq ...\n");
    for(i=0;i<8192;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_udp_buffer(unsigned char* buffer) {
    int i;
    LOGT("%s", "udp ...\n");
    for(i=0;i<512;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_metis_buffer(char* prefix,int frame,unsigned char* buffer) {
    LOGT("%s header\n",prefix);
    LOGT("  %02X%02X%02X%02X%02X%02X%02X%02X\n",
                buffer[0],buffer[1],buffer[2],buffer[3],
                buffer[4],buffer[5],buffer[6],buffer[7]);
    dump_ozy_buffer(prefix,frame,&buffer[8]);
    dump_ozy_buffer(prefix,frame,&buffer[520]);
}

