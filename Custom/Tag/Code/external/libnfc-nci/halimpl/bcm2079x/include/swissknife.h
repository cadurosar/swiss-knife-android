#ifndef SWISSKNIFE_HEADER

	#define SWISSKNIFE_HEADER


	#include <stdio.h>
	#include <stdlib.h>

	uint8_t shortCircuit, sent;

	FILE* file;
	struct timespec startOfReceiving;
	uint8_t sentMessages;		

    #ifdef LOGSWISSKNIFE
		void SwissKnife_Log(uint8_t * toPrint,uint32_t size,char * initialMessage);
	#endif

	#if SWISSKNIFEVERSION == 8
		int SwissKnife8Bits(uint8_t * pbuf, uint32_t size);
	#else
		int SwissKnife1Bit(uint8_t * pbuf, uint32_t size);			
	#endif
	int swissknife(uint8_t * pbuf, uint32_t size);
#endif
