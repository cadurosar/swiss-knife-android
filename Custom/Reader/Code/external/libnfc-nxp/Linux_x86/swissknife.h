#ifndef SWISSKNIFE_HEADER
	#define SWISSKNIFE_HEADER

	#ifdef SWISSKNIFEVERSION
		uint8_t rapidBitExchange, receivedMessages;
		unsigned long startSend[32],timeSent[32],msg3[32],receiving[32],msgsize[32],data[32];
		extern void phLlcNfc_H_ComputeCrc( uint8_t * pData, uint8_t length, uint8_t * pCrc1, uint8_t *pCrc2);
		void SwissKnife_Log(uint8_t * toPrint,uint32_t size,char * initialMessage);
		#if SWISSKNIFEVERSION == 8
			int SwissKnife8Bits(uint8_t * message, uint32_t size);
		#else
			int SwissKnife1Bit(uint8_t * message, uint32_t size);			
		#endif
	#endif

#endif
