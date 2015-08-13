#ifdef SWISSKNIFEVERSION

    #include "swissknife.h"
    #include "nfc_hal_int.h"
    #include "userial.h"
    #include <string.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <time.h>
    #include "openssl/hmac.h"

    
    /*-----------------------------------------------------------------------------------
                                    COMMON GLOBAL VARIABLES
    ------------------------------------------------------------------------------------*/
    static uint8_t initialized = 0;
    static uint8_t fileNumber = 0,waitingForNa,id;
    static HMAC_CTX ctx;
    static long zeroMessage[32],firstMessage[32],data[32],startSend[32],timeSent[32];
    static struct timespec timeData;
    static uint8_t waitingForNa;
    static uint8_t receivedNa,sendingLastMessage;  

    /*-----------------------------------------------------------------------------------
                                    COMMON FUNCTION NAMES
    ------------------------------------------------------------------------------------*/
    static void initialize();
    static void SaveDataToFile();
    static void VerifyPhase();
    static void RapidBitExchange(uint8_t * message);
    static void ProcessNa(uint8_t * message);
    static void ReceivedAID();
    static time_t GenerateTimeSeed();
    static int EndProtocol();

    #if SWISSKNIFEVERSION == 8

        /*-----------------------------------------------------------------------------------
                                    GLOBAL VARIABLES 8 BITS
        ------------------------------------------------------------------------------------*/
        static uint8_t Na[32], Nb[32],A[32],Tb[32];
        static uint8_t Cb[32],x[32],received[32];
        static uint8_t Z[2][32],PreCalc[32][256];    
    #else

        /*-----------------------------------------------------------------------------------
                                    GLOBAL VARIABLES 1 BIT
        ------------------------------------------------------------------------------------*/
        static uint8_t Na[4],Nb[20],A[20],Tb[20];
        static uint32_t Cb,x,received,Z[2];
        static uint32_t int_a,int_Tb, int_Nb, int_Na;

    #endif



    /*-----------------------------------------------------------------------------------
                                    COMMON FUNCTIONS
    ------------------------------------------------------------------------------------*/

    static time_t GenerateTimeSeed(){
        struct timespec realtime;
        clock_gettime(CLOCK_REALTIME, &realtime);
        time_t seed = realtime.tv_nsec * realtime.tv_sec;
        return seed;
    }

    static void ReceivedAID(){
        initialize();
        HAL_TRACE_ERROR0("Protocol Starts Here");
        shortCircuit = 1;
        receivedNa = 0;
        sendingLastMessage = 0;
        sentMessages = 0;
        uint8_t answer[12] = {0x10,0x00,0x00,0x08,0x48,0x65,0x6c,0x6c,0x6f,0x20,0x50,0x43};
        USERIAL_Write(USERIAL_NFC_PORT, answer, 12);
        sent = 1;
        waitingForNa = 1;
    }

    static int EndProtocol(){
        uint8_t answer[12] = {0x10,0x00,0x00,0x08,0x48,0x65,0x6c,0x6c,0x6f,0x20,0x50,0x44};
        USERIAL_Write(USERIAL_NFC_PORT, answer, 12);
        sentMessages = 0;
        sent = 1;
        return 10;
    }

    void SwissKnife_Log(uint8_t * toPrint,uint32_t size,char * initialMessage){
        uint32_t szPos;
        char buf[2][100];
        int posicao1 = 0;
        int posicao2 = 0;
        snprintf(buf[0],sizeof buf[0],"%s",initialMessage);
        
        for (szPos = 0; szPos < size; szPos++) {
            posicao1 = (szPos+1)%2;
            posicao2 = szPos%2;
            uint8_t * valor = (uint8_t *) toPrint + szPos;
            snprintf(buf[posicao1],sizeof buf[posicao1],"%s %02x",buf[posicao2], *valor);
        }
        posicao1 = (szPos+1)%2;
        posicao2 = szPos%2;
        HAL_TRACE_ERROR1("%s",buf[posicao2]);
    }

    #if SWISSKNIFEVERSION == 8

        /*-----------------------------------------------------------------------------------
                                    8 BITS FUNCTIONS
        ------------------------------------------------------------------------------------*/    

        static void HMAC_SHA256(unsigned char * result,uint32_t resultLen,void * message,uint32_t messageLength,void * key,uint32_t keyLength){
            HMAC_Init_ex(&ctx, key, keyLength, EVP_sha256(), NULL);
            HMAC_Update(&ctx, (unsigned char*) message, messageLength);
            HMAC_Final(&ctx, result, &resultLen);
            HMAC_CTX_cleanup(&ctx);
        }

        static void initialize(){
            if(!initialized){
                initialized=1;
                id=0x1;
                HMAC_CTX_init(&ctx);
                HMAC_SHA256(Cb,32,"This is our shared secret please don't tell anyone",50, "This is my secret powerful master key",37);
                HMAC_SHA256(x,32,"Carlos Eduardo Rosar Kos Lassance",33, "This is my secret powerful master key",37);
            }    
        }

        static void SaveDataToFile(){
            int i;
            char fileName[255] = "";
            sprintf(fileName, "/sdcard/SwissKnife/Test-%d.txt", fileNumber);
            fileNumber++;
            FILE * file = fopen(fileName,"w");
            if(file != NULL)
            {
                /*
                int j;
                fprintf(file, "Na: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", Na[i]);
                }
                fprintf(file, "\nNb: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", Nb[i]);
                }
                fprintf(file, "\nReceived: ");
                for(i=0;i<32;i++){
                        fprintf(file,"%02x", received[i]);
                }
                fprintf(file, "\nSent: ");
                for(i=0;i<32;i++){
                        fprintf(file,"%02x", PreCalc[i][received[i]]);
                }
                fprintf(file, "\nTb: ");
                for(i=0;i<32;i++){
                        fprintf(file,"%02x", Tb[i]);
                }
                //*/                        
                fprintf(file,"\n");               
                for(i=0;i<32;i++){
                    fprintf(file, "%lu,%lu, %lu, %lu, %lu\n",zeroMessage[i], firstMessage[i],data[i],startSend[i], timeSent[i]);
                }
                fclose(file);
            }
        }

        static void GenerateTagData(){
            int i;
            time_t seed = GenerateTimeSeed();
            HMAC_SHA256(Nb,32,&seed,8,&x,32);
            uint8_t message[64];
            memcpy ( &message[0], Nb, 32 );
            memcpy ( &message[32], Cb, 32 );    
            HMAC_SHA256(&A[0],32,message,64,&x,32);
            int j = 0;
            for(j=0;j<32;j++)
                Z[0][j] = A[j];
            for(j=0;j<32;j++)
                Z[1][j] = A[j] ^ x[j];            
            for(i=0;i<32;i++){
                received[i] = 0;
           }
           int round = 0;
           int challenge;   
           for(round = 0;round<32;round++){
                for(challenge=0;challenge<256;challenge++){
                    unsigned char c[8],r[8];
                    int number;
                    unsigned char mask[8] = {1,2,4,8,16,32,64,128};
                    for(number=0;number<8;number++){
                        c[number] =  challenge & mask[number];
                        if(c[number] == 0) 
                            r[number] = Z[0][round] & mask[number];
                        else
                            r[number] = Z[1][round] & mask[number];
                    }
                    PreCalc[round][challenge] = 0;
                    for(number=0;number<8;number++){
                        PreCalc[round][challenge] |= r[number];
                    }   
                }
            }
        }

        static void ProcessNa(uint8_t * message){
            int index1;
            for(index1 = 0;index1 < 16;index1++){
                Na[index1+16*receivedNa] = message[3+index1] ;
            }
            if(receivedNa == 0){
                GenerateTagData();
            }
            uint8_t answer[20] = {0x10,0x00,0x00,0x10};
            memcpy ( &answer[4], &Nb[16*(receivedNa)], 16 );
            receivedNa++;
            if(receivedNa == 2)
                waitingForNa = 0;
            USERIAL_Write(USERIAL_NFC_PORT, answer, 20);
            sent = 1;
        }

        static void RapidBitExchange(uint8_t * message){
            struct timespec timeStart, timeEnd;
            received[sentMessages] = message[3] ; //Store the byte
            uint8_t answer[5] = {0x10,0x00,0x00,0x01,PreCalc[sentMessages][message[3]]};
            clock_gettime(CLOCK_MONOTONIC, &timeStart);
            USERIAL_Write(USERIAL_NFC_PORT, answer, 5);
            clock_gettime(CLOCK_MONOTONIC, &timeEnd);
            data[sentMessages] = (timeData.tv_sec*1000000000)+timeData.tv_nsec; 
            startSend[sentMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
            timeSent[sentMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 
            sentMessages++;
            sent = 1;
        }

        static void VerifyPhase(){
            if(sendingLastMessage == 0){
                int i;
                uint8_t seedTb[97];
                memcpy ( &seedTb[0], Na, 32 );
                memcpy ( &seedTb[32], Nb, 32 );
                memcpy ( &seedTb[64], &id, 1 );
                memcpy ( &seedTb[65], received, 32 );
                HMAC_SHA256(Tb, 32, seedTb,97, &x, 32);
            }
            uint8_t answer[20] = {0x10,0x00,0x00,0x10};
            if(sendingLastMessage < 2){
                memcpy ( &answer[4], &received[(sendingLastMessage)*16], 16 );
            }
            else
                memcpy ( &answer[4], &Tb[(sendingLastMessage-2)*16], 16 );
            USERIAL_Write(USERIAL_NFC_PORT, answer, 20);
            sendingLastMessage++;
            if(sendingLastMessage == 4){
                sentMessages=0;
                HAL_TRACE_ERROR0("Protocol ends Here");
                SaveDataToFile();
            }
        }

        int SwissKnife8Bits(uint8_t * pbuf, uint32_t size){
                /* Start of my code CADU */
                int sent = 0;
                int i = 0;
                UINT8 * message = (UINT8 *) pbuf ;

                clock_gettime(CLOCK_MONOTONIC, &timeData);

                #ifdef LOGSWISSKNIFE
                    SwissKnife_Log(message,size,"Receiving Message:");
                #endif

            message++; //We ignore the first byte of the message because it does not matter to us.
            if(message[0] == 0x00){
                if(message[3] == 0x00 && message[4] == 0xA4 && message[5] == 0x04 && message[6] == 0x00 && message[7] == 0x07){
                    if(message[8] == 0x00 && message[9] == 0x01 && message[10] == 0x00 && message[11] == 0x01 && message[12] == 0x00 && message[13] == 0x01 && message[14] == 0x00){
                        ReceivedAID();
                    }
                    else // It's not our AID, do not shortCircuit.
                        shortCircuit = 0;
                }
                else if(message[3] == 0xF0 && message[4] == 0XDE && message[5] == 0X00){
                        return EndProtocol();
                }
                else if(shortCircuit){
        	        if(waitingForNa && receivedNa < 2){ //RECEIVED Na, Generate Nb, A, Z and send Nb
                        ProcessNa(message);
        	        }
        		    else if (sentMessages < 32){
                        RapidBitExchange(message);
        		    }
        		    else if(sentMessages == 32){
                        VerifyPhase();
        		    }
        		    else{
        		        shortCircuit = 0; //Received invalid message stop shortCircuiting.
        		    }
                    sent = 1; 
                    return 1;
                }            
            } 
            else if(shortCircuit && message[0] == 0x60 && sentMessages < 32) {
                zeroMessage[sentMessages] = (startOfReceiving.tv_sec*1000000000)+startOfReceiving.tv_nsec; 
                firstMessage[sentMessages] = (timeData.tv_sec*1000000000)+timeData.tv_nsec; 
            }
            return 0;
        }

    #else

        /*-----------------------------------------------------------------------------------
                                    1 BIT FUNCTIONS
        ------------------------------------------------------------------------------------*/

        static void initialize(){
            if(!initialized){
                initialized=1;
                Cb=0x10203040;
                x=0x40302010; 
                id=0x1;
                HMAC_CTX_init(&ctx);
            }
        }

        static void HMAC_SHA1(unsigned char * result,uint32_t resultLen,void * message,uint32_t messageLength,void * key,uint32_t keyLength){
            HMAC_Init_ex(&ctx, key, keyLength, EVP_sha1(), NULL);
            HMAC_Update(&ctx, (unsigned char*) message, messageLength);
            HMAC_Final(&ctx, result, &resultLen);
            HMAC_CTX_cleanup(&ctx);
        }

        static void SaveDataToFile(){
            int i;
            char fileName[255] = "";
            sprintf(fileName, "/sdcard/SwissKnife/Test-%d.txt", fileNumber);
            fileNumber++;
            file = fopen(fileName,"w");
            if(file != NULL)
            {
                fprintf(file, "Received: %08x, Na: %08x , Nb: %08x, Z0: %08x, Z1: %08x, Tb: %08x \n", received, int_Na, int_Nb, Z[0], Z[1], int_Tb);  
                for(i=0;i<32;i++){
                    fprintf(file, "%lu,%lu, %lu, %lu, %lu\n",zeroMessage[i], firstMessage[i],data[i],startSend[i], timeSent[i]);
                }
            fclose(file);
            }
        }

        static void GenerateTagData(){
            time_t seed = GenerateTimeSeed();
            HMAC_SHA1(Nb, 20,&seed,8,&x,4);
            uint8_t * bytes_Nb = (uint8_t *) (Nb);
            int_Nb = (bytes_Nb[0] << 24) | (bytes_Nb[1] << 16) | (bytes_Nb[2] << 8) | bytes_Nb[3];  
            uint64_t seed1 = (uint64_t)( (uint64_t) int_Nb << 32) | Cb;
            HMAC_SHA1(A,20,&seed1,8,&x,4);
            uint8_t * bytes_a = (uint8_t *) (A);
            int_a = (bytes_a[0] << 24) | (bytes_a[1] << 16) | (bytes_a[2] << 8) | bytes_a[3];  
            Z[0] = int_a;
            Z[1] = int_a^x;
        }

        static void ProcessNa(uint8_t * message){
            received = 0;
            Na[0] = message[3];
            Na[1] = message[4];
            Na[2] = message[5];
            Na[3] = message[6];             
            uint8_t * bytes_Na = (uint8_t *) (Na);
            int_Na = (bytes_Na[0] << 24) | (bytes_Na[1] << 16) | (bytes_Na[2] << 8) | bytes_Na[3];  
            GenerateTagData();
            uint8_t * bytes_nb = (uint8_t *) Nb;
            uint8_t answer[8] = {0x10,0x00,0x00,0x04,bytes_nb[0],bytes_nb[1],bytes_nb[2],bytes_nb[3]};
            waitingForNa = 0;
            USERIAL_Write(USERIAL_NFC_PORT, answer, 8);
        }

        static void RapidBitExchange(uint8_t * message){
            struct timespec timeStart, timeEnd;
            uint8_t bitReceived = message[3] & 0x01;
            received |= (bitReceived) << (31-sentMessages) ; //Store the bit
            uint8_t bit = (Z[bitReceived] >> (31-sentMessages) ) & 0x01 ; //get the bit we want to send
            uint8_t answer[5] = {0x10,0x00,0x00,0x01,bit};
            clock_gettime(CLOCK_MONOTONIC, &timeStart);
            USERIAL_Write(USERIAL_NFC_PORT, answer, 5);
            clock_gettime(CLOCK_MONOTONIC, &timeEnd);
            data[sentMessages] = (timeData.tv_sec*1000000000)+timeData.tv_nsec; 
            startSend[sentMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
            timeSent[sentMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 
            sentMessages++;
        }

        static void VerifyPhase(){
            int i;
            uint32_t seedTb[5] = {int_Na, int_Nb, id,received, x};
            HMAC_SHA1(Tb, 20, &seedTb[0],20, &x, 4);
            uint8_t * bytes_Tb = (uint8_t *) Tb;
            int_Tb = (bytes_Tb[0] << 24) | (bytes_Tb[1] << 16) | (bytes_Tb[2] << 8) | bytes_Tb[3];  
            uint8_t * bytes_received = (uint8_t *) &received;
            uint8_t answer[12] = {0x10,0x00,0x00,0x08,bytes_received[0], bytes_received[1], bytes_received[2], bytes_received[3],bytes_Tb[0],bytes_Tb[1],bytes_Tb[2],bytes_Tb[3]};
            USERIAL_Write(USERIAL_NFC_PORT, answer, 12);
            sentMessages=0;
            SaveDataToFile();
            HAL_TRACE_ERROR0("Protocol ends Here");
        }        

        int SwissKnife1Bit(uint8_t * pbuf, uint32_t size){
            /* Start of my code CADU */
            int sent = 0;
            int i = 0;
            UINT8 * message = (UINT8 *) pbuf ;

            struct timespec time;
            clock_gettime(CLOCK_MONOTONIC, &timeData);

            #ifdef LOGSWISSKNIFE
                SwissKnife_Log(message,size,"Receiving Message:");
            #endif

            message++; //We ignore the first byte of the message because it does not matter to us.
            if(message[0] == 0x00){
                if(message[3] == 0x00 && message[4] == 0xA4 && message[5] == 0x04 && message[6] == 0x00 && message[7] == 0x07){
                    if(message[8] == 0x00 && message[9] == 0x01 && message[10] == 0x00 && message[11] == 0x01 && message[12] == 0x00 && message[13] == 0x01 && message[14] == 0x00){
                        ReceivedAID();
                    }
                    else
                        shortCircuit = 0; // It's not our AID, do not shortCircuit.
                }
                else if(message[3] == 0xF0 && message[4] == 0XDE && message[5] == 0X00){
                    return EndProtocol();    
                }
                else if(shortCircuit){
                    int length = 0;
                    if(waitingForNa){ //RECEIVED Na, Generate Nb, A, Z and send Nb
                        ProcessNa(message);
                    }
                    else if (sentMessages < 32){
                        RapidBitExchange(message);
                    }
                    else if(sentMessages == 32){
                        VerifyPhase();
                    }
                    else{ 
                        shortCircuit = 0; //Received invalid message stop shortCircuiting.
                    }
                    sent = 1; 
                    return 1;
                }            
            } 
            else if(shortCircuit && message[0] == 0x60 && sentMessages < 32) {
                zeroMessage[sentMessages] = (startOfReceiving.tv_sec*1000000000)+startOfReceiving.tv_nsec; 
                firstMessage[sentMessages] = (time.tv_sec*1000000000)+time.tv_nsec; 
            }
            return 0;
        }
    #endif
#endif