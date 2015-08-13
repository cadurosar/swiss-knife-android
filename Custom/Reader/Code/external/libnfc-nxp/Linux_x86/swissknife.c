#ifdef SWISSKNIFEVERSION
    #include <string.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <time.h>
    #include <phDal4Nfc_uart.h> 
    #include "openssl/hmac.h"
    #include "swissknife.h"
    #include <cutils/log.h>
    
    /*-----------------------------------------------------------------------------------
                                    COMMON GLOBAL VARIABLES
    ------------------------------------------------------------------------------------*/

    extern uint8_t last_ns, last_nr;
    static uint8_t ns1, nr1, waitingForNb = 0;
    static uint8_t id, sent,fileNumber,executionsThisBatch,waitingForTheEnd = 0;
    static uint8_t errC, errR, errT,errTb, lastPass = 0;
    static HMAC_CTX ctx;


    /*-----------------------------------------------------------------------------------
                                    COMMON FUNCTION NAMES
    ------------------------------------------------------------------------------------*/

    static void GenerateTagData();
    static void VerifyErrors();
    static void SaveDataToFile();
    static int VerificationPhase(uint8_t * message);
    static int RapidBitExchange(uint8_t * message);


    #if SWISSKNIFEVERSION == 8

        /*-----------------------------------------------------------------------------------
                                    GLOBAL VARIABLES 8 BITS
        ------------------------------------------------------------------------------------*/

        static uint8_t receivedLastMessage = 0,receivedNb;
        static uint8_t Na[32], Nb[32],A[32],C[32],Tb[32],Capostrophe[32],calculatedTb[32];
        static uint8_t received[32],Cb[32],x[32],Z[2][32], PreCalc[32][256];   
    
    #else

        /*-----------------------------------------------------------------------------------
                                    GLOBAL VARIABLES 1 BIT
        ------------------------------------------------------------------------------------*/

        static uint8_t Na[20], Nb[20], a[20], C[20],Capostrophe[20],Tb[20],calculatedTb[20];
        static uint32_t Z[2];
        static uint32_t Cb,x,int_Na, int_a, int_Nb, int_C, int_Capostrophe, int_Tb, int_CalculatedTb;
        static uint32_t received;
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

    static int EndPhase(uint8_t * message,uint32_t size){
        uint32_t i = 0;
        uint8_t calculaCRC[15];
        waitingForTheEnd = 0;

        //Generate a byteLLC that would be the normal response for the first message we sent.
        uint8_t byteLLC = 0x80 | ( (nr1+1)%8) | ((ns1) << 3);        

        //Generate the response that goes to the upper layers.
        message[0] = byteLLC;
        message[3] = lastPass;
        message[4] = id;
        message[5] = errC;
        message[6] = errR;
        message[7] = errT;
        message[8] = 0x00;
        message[9] = 0x00;
        message[10] = 0x00;
        for(i=0;i<size;i++)
            calculaCRC[i+1] = message[i];
        calculaCRC[0] = 0x0e; 
        phLlcNfc_H_ComputeCrc(calculaCRC,13,&message[12],&message[13]);

        //Tries to tell the chip that the protocol is over and we have received all the messages we need.
        for(i=0;i<8;i++){
            uint8_t byteLLC2 = 0xC0 | ( i%8 ) ;
            uint8_t send[4] = {0x03, byteLLC2, 0x00,0x00};                  
            phLlcNfc_H_ComputeCrc(send,2,&send[2],&send[3]);
            phDal4Nfc_uart_write(send, 4);
        }
        return 0;
    }

    static int ContinueOrFinish(){
        if(executionsThisBatch < EXECUTIONSPERBATCH){
            last_ns = (last_ns + 1) % 8;
            last_nr = (last_nr + 1) % 8;
            uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
            uint8_t send[19] = {0x12,byteLLC,0x85,0x10,0x00,0x00, 0xA4, 0x04, 0x00,
                    0x07, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,0x00};
            phLlcNfc_H_ComputeCrc(send,17,&send[17],&send[18]);
            phDal4Nfc_uart_write(send, 19);
            return 4;
        }
        else{
            executionsThisBatch = 0;
            last_ns = (last_ns + 1) % 8;
            last_nr = (last_nr + 1) % 8;
            uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
            uint8_t send[10] = {0x09,byteLLC,0x85,0x10,0x00,0xF0,0xDE,0x00, 0x00,0x00};
            phLlcNfc_H_ComputeCrc(send,8,&send[8],&send[9]);
            phDal4Nfc_uart_write(send, 10);
            waitingForTheEnd = 1;
            return 4;
        }
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
        ALOGE("%s",buf[posicao2]);
    }

    #if SWISSKNIFEVERSION == 8

        /*-----------------------------------------------------------------------------------
                                    8 BITS FUNCTIONS
        ------------------------------------------------------------------------------------*/
        static void SaveDataToFile(){
            int i, countMsg3 = 0;
            char fileName[255] = "";
            sprintf(fileName, "/sdcard/SwissKnife/Test-%d.txt", fileNumber);
            fileNumber++;
            executionsThisBatch++;
            FILE* file = fopen(fileName,"w");        
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
                fprintf(file, "\nCapostrophe: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", Capostrophe[i]);
                }
                fprintf(file, "\nC: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", C[i]);
                }
                fprintf(file, "\nReceived: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", received[i]);
                }
                fprintf(file, "\nCalculatedSent: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", PreCalc[i][C[i]]);
                }
                fprintf(file, "\nTb: ");
                    for(i=0;i<32;i++){
                        fprintf(file,"%02x", Tb[i]);
                }
                fprintf(file, "\nSeedTb: ");
                for(i=0;i<97;i++){
                    fprintf(file,"%02x", seedTb[i]);
                }
                fprintf(file, "\nCalculated Tb: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", calculatedTb[i]);
                }
                fprintf(file, "\nA: ");
                for(i=0;i<32;i++){
                    fprintf(file,"%02x", A[i]);
                }

                fprintf(file,"\n");
            */  
                for(i=0;i<32;i++){
                    if(msg3[i] == 0){
                        countMsg3++;
                    }
                    fprintf(file, "%lu, %lu, %lu, %lu, %lu, %lu\n", startSend[i],timeSent[i],msg3[i], receiving[i],msgsize[i],data[i]);
                }
                fprintf(file, "Polluted messages : %d, errC = %d, errR = %d, errT = %d, Pass=%s \n", 32-countMsg3,errC,errR,errT, lastPass ?"true":"false" );
                ALOGE("Polluted messages : %d, errC = %d, errR = %d, errT = %d, Pass=%s \n", 32-countMsg3,errC,errR,errT, lastPass ?"true":"false");
                fclose(file);
                }
        }


        static void HMAC_SHA256(unsigned char * result,unsigned int resultLen,void * message,unsigned int messageLength,void * key,unsigned int keyLength){
            HMAC_Init_ex(&ctx, key, keyLength, EVP_sha256(), NULL);
            HMAC_Update(&ctx, (unsigned char*) message, messageLength);
            HMAC_Final(&ctx, result, &resultLen);
            HMAC_CTX_cleanup(&ctx);
        }

        static void GenerateTagData(){
            uint8_t seedTb[97];
            memcpy ( &seedTb[0], Na, 32 );
            memcpy ( &seedTb[32], Nb, 32 );
            memcpy ( &seedTb[64], &id, 1 );
            memcpy ( &seedTb[65], Capostrophe, 32 );
            HMAC_SHA256(calculatedTb, 32, seedTb,97, &x, 32);

            int i,j = 0;

            //Verify Tb
            for(i=0;i<32;i++){
                if(Tb[i] != calculatedTb[i]){
                   ALOGE("Tb did not match");
                   errTb = 32;
                   break;
                }
            }

            uint8_t messageA[64];
            memcpy ( &messageA[0], Nb, 32 );
            memcpy ( &messageA[32], Cb, 32 );    
            HMAC_SHA256(A,32,messageA,64,&x,32);

            for(j=0;j<32;j++)
                Z[0][j] = A[j];
            for(j=0;j<32;j++)
                Z[1][j] = A[j] ^ x[j];            

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

        static void VerifyErrors(){
            GenerateTagData();
            errC = 0;
            errR = 0;
            errT = 0;
            int i =0;
            uint8_t Cresult[32];
            uint8_t expectedReceived[32];
            for(i=0;i<32;i++){
                if( C[i] != Capostrophe[i]){
                    errC++;    
                } else if(PreCalc[i][C[i]] != received[i] ){
                    errR++;
                } else if ( (msgsize[i] -  startSend[i]) > TIMEHIGH || (msgsize[i] -  startSend[i]) < TIMELOW){
                    errT++;
                }
            }
            lastPass = (errT+errR+errC+errTb <= THRESHOLD);
        }

        static int InitializationPhasePart1(){

            ALOGE("Protocol Starts Here");

            //Zero variables
            int i = 0;
            for (i=0;i<32;i++){
                startSend[i] = 0;
                timeSent[i] = 0;
                msg3[i] = 0;
                receiving[i] = 0; 
                msgsize[i] = 0;
                data[i] = 0;
                received[i] = 0;
                Na[i] = 0;
                Nb[i] = 0;
            }
            receivedNb = 0;        
            receivedLastMessage = 0;
            receivedMessages = 0;
            errTb = 0;
            waitingForNb = 1;

            //Start random and generate C, Na,Cb,x and id
            HMAC_CTX_init(&ctx);
            time_t seed = GenerateTimeSeed();
            HMAC_SHA256(C,32,&Cb,8,&seed,8);
            HMAC_SHA256(Na,32,&seed,8,&Cb,32);
            HMAC_SHA256(Cb,32,"This is our shared secret please don't tell anyone",50, "This is my secret powerful master key",37);
            HMAC_SHA256(x,32,"Carlos Eduardo Rosar Kos Lassance",33, "This is my secret powerful master key",37);
            id=0x1;

            //Prepare control byte
            if(executionsThisBatch == 0){
                ns1 = last_ns;
                nr1 = last_nr;
            }
            last_ns = (last_ns + 1) % 8;
            last_nr = (last_nr + 1) % 8;
            uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);

            //Prepare the bytes to send and send it.
            uint8_t send[23] = {0x16,byteLLC,0x85,0x10,0x00};
            memcpy ( &send[5], &Na[0], 16 );
            phLlcNfc_H_ComputeCrc(send,21,&send[21],&send[22]);        
            phDal4Nfc_uart_write(send, 23);        

            return 1;

        }

        static int ReceivingNbPhase(uint8_t * message){
            int index1;
            struct timespec timeStart,timeEnd;
            memcpy ( &Nb[16*receivedNb], &message[3], 16 );
            receivedNb++;

            if(receivedNb == 2){ //receivedTheFullNb send the first bit       

                rapidBitExchange = 1;
                waitingForNb = 0;
                //Prepare message
                last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t byte = C[0];
                uint8_t send[8] = {0x07,byteLLC,0x85,0x10,0x0e,byte, 0x73,0xcb};
                phLlcNfc_H_ComputeCrc(send,6,&send[6],&send[7]);


                //Send the first byte from C    
                clock_gettime(CLOCK_MONOTONIC, &timeStart);
                phDal4Nfc_uart_write(send, 8);
                clock_gettime(CLOCK_MONOTONIC, &timeEnd);
                startSend[receivedMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
                timeSent[receivedMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 

            }

            else{

                //Send the second part of Na
                last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t send[23] = {0x16,byteLLC,0x85,0x10,0x00};
                memcpy ( &send[5], &Na[16], 16 );
                phLlcNfc_H_ComputeCrc(send,21,&send[21],&send[22]);        
                phDal4Nfc_uart_write(send, 23);        

            }

            return 2;
        }

        static int RapidBitExchange(uint8_t * message){
            struct timespec timeStart,timeEnd;

            received[receivedMessages] = message[3]; 
            receivedMessages++;

            uint8_t byte = C[receivedMessages];
            last_ns = (last_ns + 1) % 8;
            last_nr = (last_nr + 1) % 8;
            uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
            uint8_t send[8] = {0x07,byteLLC,0x85,0x10,0x00,byte, 0x00,0x00};
            phLlcNfc_H_ComputeCrc(send,6,&send[6],&send[7]);

            clock_gettime(CLOCK_MONOTONIC, &timeStart);
            phDal4Nfc_uart_write(send, 8);
            clock_gettime(CLOCK_MONOTONIC, &timeEnd);
            startSend[receivedMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
            timeSent[receivedMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 
            return 3;

        }

        static int VerificationPhase(uint8_t * message){
            int i;
            if(receivedLastMessage < 2){ //receiving C'
                memcpy (&Capostrophe[16*receivedLastMessage], &message[3],16 );
            }
            else{//Receiving Tb
                memcpy (&Tb[16*(receivedLastMessage-2)], &message[3],16 );
            }
            receivedLastMessage++;
            if(receivedLastMessage < 4){ //Still receiving data
                last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t send[8] = {0x07,byteLLC,0x85,0x10,0x00,0x00, 0x00,0x00};
                phLlcNfc_H_ComputeCrc(send,6,&send[6],&send[7]);
                phDal4Nfc_uart_write(send, 8);
                return 10;
            } else if(receivedLastMessage == 4){ //Received everything now let's verify.

                ALOGE("Protocol Ends Here");
                rapidBitExchange = 0;

                VerifyErrors();
                SaveDataToFile();
                return ContinueOrFinish();
            }
            return 11;
        }

        int SwissKnife8Bits(uint8_t * message, uint32_t size){       
            if(size == 14 && message[3] == 0x48 && message[4] == 0x65 && message[5] == 0x6c && message[6] == 0x6c && message[7] == 0x6f &&
            message[8] == 0x20 && message[9] == 0x50 && message[10] == 0x43) // Received Hello Pc -> Start
            { 
                return InitializationPhasePart1();
            }
            else if(size == 14 && message[3] == 0x48 && message[4] == 0x65 && message[5] == 0x6c && message[6] == 0x6c && message[7] == 0x6f &&
            message[8] == 0x20 && message[9] == 0x50 && message[10] == 0x44 && waitingForTheEnd){ // Received Hello PD -> Clean
                return EndPhase(message,size);
            }
            else if(waitingForNb && size == 22 && receivedNb < 2){ // We are waiting for Nb and we received it
                return ReceivingNbPhase(message);
            }
            
            else if(rapidBitExchange && size == 7 && receivedMessages < 32){ //Rapid Bit Exchange
                return RapidBitExchange(message);
            }
            else if(rapidBitExchange && size == 22 && receivedMessages == 32){ //Verification Phase
                return VerificationPhase(message);
            }   
            return 0;
        }
    #else

        /*-----------------------------------------------------------------------------------
                                    1 BIT FUNCTIONS
        ------------------------------------------------------------------------------------*/


        static void HMAC_SHA1(unsigned char * result,unsigned int resultLen,void * message,unsigned int messageLength,void * key,unsigned int keyLength){
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
            executionsThisBatch++;
            FILE * file = fopen(fileName,"w");        
            VerifyErrors();
            if(file != NULL)
            {
                int countMsg3 = 0;
                for(i=0;i<32;i++){
                    if(msg3[i] == 0){
                        countMsg3++;
                    }
                    fprintf(file, "%lu, %lu, %lu, %lu, %lu, %lu\n", startSend[i],timeSent[i],msg3[i], receiving[i],msgsize[i],data[i]);
                }
                fprintf(file, "Received: %08x, Na: %08x , Nb: %08x, C: %08x, Capostrophe: %08x, Tb: %08x, Calculated Z[0]: %08x, Calculated Z[1]: %08x, Calculated Tb: %08x\n", received, int_Na, int_Nb, int_C, int_Capostrophe, int_Tb,Z[0],Z[1],int_CalculatedTb);  
                fprintf(file, "Polluted messages : %d, errC = %d, errR = %d, errT = %d, Pass=%s \n", 32-countMsg3,errC,errR,errT, lastPass ?"true":"false" );
                ALOGE("Polluted messages : %d, errC = %d, errR = %d, errT = %d, Pass=%s \n", 32-countMsg3,errC,errR,errT, lastPass ?"true":"false");
                fclose(file);
            }
        }

        static void VerifyErrors(){
            errC = 0;
            errR = 0;
            errT = 0;
            int i =0;
            uint32_t Cresult = int_C ^ int_Capostrophe;
            uint32_t expectedReceived = ( int_C & Z[1] ) | (  ~(int_C) & Z[0]);
            uint32_t receivedResult = expectedReceived ^ received;
            for(i=0;i<32;i++){
                if( ((Cresult >> i) & 0x01) == 0x01){
                    errC++;    
                } else if(((receivedResult >> i) & 0x01) == 0x01 ){
                    errR++;
                } else if ( (msgsize[i] -  startSend[i]) > TIMEHIGH || (msgsize[i] -  startSend[i]) < TIMELOW){
                    errT++;
                }
            }
            lastPass = (errT+errR+errC+errTb <= THRESHOLD);
        }

        static int InitializationPhase(){
                
                ALOGE("Protocol Starts Here");
                HMAC_CTX_init(&ctx);
                time_t seed = GenerateTimeSeed();
                
                x= 0x40302010;
                Cb = 0x10203040;
                id = 0x1;
                
                HMAC_SHA1(Na,20,&x,4,&seed,8);
                uint8_t * bytes_Na = (uint8_t *) Na;
                int_Na = (bytes_Na[0] << 24) | (bytes_Na[1] << 16) | (bytes_Na[2] << 8) | bytes_Na[3];  
                seed = GenerateTimeSeed();
                HMAC_SHA1(C,20,&seed,8,&x,4);
                uint8_t * bytes_C = (uint8_t *) (C);        
                int_C = (bytes_C[0] << 24) | (bytes_C[1] << 16) | (bytes_C[2] << 8) | bytes_C[3];  
                if(executionsThisBatch == 0){
                    ns1 = last_ns;
                    nr1 = last_nr;
                }
               last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t * NaByte = (uint8_t *) Na;
                uint8_t send[11] = {0x0a,byteLLC,0x85,0x10,0x00,NaByte[0],NaByte[1],NaByte[2],NaByte[3], 0x73,0xcb};
                phLlcNfc_H_ComputeCrc(send,9,&send[9],&send[10]);
                phDal4Nfc_uart_write(send, 11);        
                waitingForNb = 1;
                int i = 0;
                for (i=0;i<32;i++){
                    startSend[i] = 0;
                    timeSent[i] = 0;
                    msg3[i] = 0;
                    receiving[i] = 0; 
                    msgsize[i] = 0;
                    data[i] = 0;
                }
                receivedMessages= 0;
                received = 0;
                return 1;
        } 

        static int TreatNb(uint8_t * message){
                struct timespec timeStart,timeEnd;

                uint8_t * byte = (uint8_t *) Nb;
                byte[0] = message[3];
                byte[1] = message[4];
                byte[2] = message[5];
                byte[3] = message[6];

                waitingForNb = 0; 
                rapidBitExchange = 1;
                last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t bit = (  (int_C) >> 31) & 0x01;
                uint8_t send[8] = {0x07,byteLLC,0x85,0x10,0x0e,bit, 0x73,0xcb};
                
                phLlcNfc_H_ComputeCrc(send,6,&send[6],&send[7]);
                clock_gettime(CLOCK_MONOTONIC, &timeStart);
                phDal4Nfc_uart_write(send, 8);
                clock_gettime(CLOCK_MONOTONIC, &timeEnd);
                startSend[receivedMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
                timeSent[receivedMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 
                
                return 2;

        }

        static int RapidBitExchange(uint8_t * message){
                struct timespec timeStart,timeEnd;

                received |= (message[3] & 0x01) << (31-receivedMessages) ; 
                uint8_t bit = ( (int_C) >> (31-(receivedMessages+1)) ) & 0x01;
                last_ns = (last_ns + 1) % 8;
                last_nr = (last_nr + 1) % 8;
                uint8_t byteLLC = 0x80 | (last_ns) | ((last_nr) << 3);
                uint8_t send[8] = {0x07,byteLLC,0x85,0x10,0x00,bit, 0x00,0x00};
                phLlcNfc_H_ComputeCrc(send,6,&send[6],&send[7]);
                clock_gettime(CLOCK_MONOTONIC, &timeStart);
                phDal4Nfc_uart_write(send, 8);
                clock_gettime(CLOCK_MONOTONIC, &timeEnd);
                receivedMessages++;
                if(receivedMessages < 32){
                    startSend[receivedMessages] = (timeStart.tv_sec*1000000000)+timeStart.tv_nsec; 
                    timeSent[receivedMessages] = (timeEnd.tv_sec*1000000000)+timeEnd.tv_nsec; 
                }
                return 3;
        }

        static void GenerateTagData(){
            uint32_t seedTb[5] = {int_Na, int_Nb, id, int_Capostrophe, x};
            HMAC_SHA1(calculatedTb, 20, seedTb,20, &x, 4);
            uint8_t * bytes_Tb = (uint8_t *) Tb;
            int_Tb = (bytes_Tb[0] << 24) | (bytes_Tb[1] << 16) | (bytes_Tb[2] << 8) | bytes_Tb[3];  
            uint8_t * bytes_calculatedTb = (uint8_t *) calculatedTb;
            int_CalculatedTb = (bytes_calculatedTb[0] << 24) | (bytes_calculatedTb[1] << 16) | (bytes_calculatedTb[2] << 8) | bytes_calculatedTb[3];  

            uint64_t seed1 = (uint64_t)( (uint64_t) int_Nb << 32) | Cb;                
            HMAC_SHA1(a,20,&seed1,8,&x,4);
            uint8_t * bytes_a = (uint8_t *) (a);
            int_a = (bytes_a[0] << 24) | (bytes_a[1] << 16) | (bytes_a[2] << 8) | bytes_a[3];  
            Z[0] = (int_a);
            Z[1] = (int_a)^x;
        }

        static int VerificationPhase(uint8_t * message){
                int i;
                ALOGE("Protocol Ends Here");
                rapidBitExchange = 0;
                waitingForNb = 0;

                uint8_t * byte = (uint8_t *) Capostrophe;
                byte[0] = message[6];
                byte[1] = message[5];
                byte[2] = message[4];
                byte[3] = message[3];
                uint8_t * bytes_Capostrophe = (uint8_t *) (Capostrophe);
                int_Capostrophe = (bytes_Capostrophe[0] << 24) | (bytes_Capostrophe[1] << 16) | (bytes_Capostrophe[2] << 8) | bytes_Capostrophe[3];  

                byte = (uint8_t *) Tb;
                byte[0] = message[7];
                byte[1] = message[8];
                byte[2] = message[9];
                byte[3] = message[10];
                uint8_t * bytes_Nb = (uint8_t *) (Nb);
                int_Nb = (bytes_Nb[0] << 24) | (bytes_Nb[1] << 16) | (bytes_Nb[2] << 8) | bytes_Nb[3];  

                GenerateTagData();
                VerifyErrors();
                SaveDataToFile();
                return ContinueOrFinish();                
        }


        int SwissKnife1Bit(uint8_t * message, uint32_t size){       
            if(size == 14 && message[3] == 0x48 && message[4] == 0x65 && message[5] == 0x6c && message[6] == 0x6c && message[7] == 0x6f &&
            message[8] == 0x20 && message[9] == 0x50 && message[10] == 0x43){ // Received Hello send Na
                return InitializationPhase();
            }
            else if(size == 14 && message[3] == 0x48 && message[4] == 0x65 && message[5] == 0x6c && message[6] == 0x6c && message[7] == 0x6f &&
            message[8] == 0x20 && message[9] == 0x50 && message[10] == 0x44 && waitingForTheEnd){ // Received Hello send Na
                return EndPhase(message,size);
            }
            else if(waitingForNb && size == 10){ // Received NB send first bit
                return TreatNb(message);
            }
            else if(rapidBitExchange && size == 7 && receivedMessages < 32){
                return RapidBitExchange(message);
            }
            else if(rapidBitExchange && size == 14 && receivedMessages == 32){
                return VerificationPhase(message);
            }   
            return 0;
        }
    #endif
#endif
