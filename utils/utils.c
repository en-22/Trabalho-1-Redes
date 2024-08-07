#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "utils.h"
#include <dirent.h>


void sendError(int sok, struct sockaddr_ll end, unsigned char seq, int errortype, struct frame *frame, unsigned char *buffer){
    unsigned char data[MAX_DATA_LENGTH] = {0};
	memcpy(data, &errortype, sizeof(int));//Indica tipo do erro nos dados

    mountPacket(frame, seq, ERROR, data, MAX_DATA_LENGTH);
    sendto(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, sizeof(end));//Não precisa da função sendPacket porque não necessita de ack
}

unsigned int crc8(unsigned char *data, int size){
    unsigned char crc = 0x00;
    for (int i = 0; i < size; i++){
        crc ^= data[i];
        for (int j = 0; j < 8; j++){
            if (crc & 0x80){
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void mountPacket(struct frame *frame, unsigned char seq, unsigned char type, unsigned char *data, __off_t size){
    memset(frame, 0, sizeof(struct frame));
    frame->start_marker = START_MARKER;
    frame->seq = seq;
    frame->type = type;
    memcpy(frame->data, data, size);
    frame->size = size;
    frame->crc = crc8((unsigned char*)frame, sizeof(struct frame)-1);
} 

void sendACK(int sok, struct sockaddr_ll end, unsigned char seq, struct frame *frame, unsigned char *buffer){
    unsigned char data[MAX_DATA_LENGTH] = {0};

    mountPacket(frame, seq, ACK, data, MAX_DATA_LENGTH);
    sendto(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, sizeof(end));
}

void sendNACK(int sok, struct sockaddr_ll end, unsigned char seq, struct frame *frame, unsigned char *buffer){
    unsigned char data[MAX_DATA_LENGTH] = {0};

    mountPacket(frame, seq, NACK, data, MAX_DATA_LENGTH);
    sendto(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, sizeof(end));
}

//tenta enviar pacote até conseguir enviar e receber ACK ou ERRO, ou da TIMEOUT na falta de resposta
int sendPacket(int sok, struct sockaddr_ll end, unsigned char *buffer){
	struct frame* awnserFrame;
    unsigned char* awnserBuffer = (unsigned char*) malloc(sizeof(struct frame));
    memset(awnserBuffer, 0, sizeof(struct frame));
    awnserFrame = (struct frame*) awnserBuffer;
    int type;

	do{
		sendto(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, sizeof(end));
        type = waitforACK(sok, end, awnserFrame, awnserBuffer);
        if(type == TIMEOUT){
            fprintf(stderr, "Timed out.\n");
            return TIMEOUT;
        }
        if(type == ERROR)
            return ERROR;
	} while (type != ACK);
        
    return ACK;
}

int receivePacket(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){
    socklen_t addr_len = sizeof(struct sockaddr);
    unsigned char* awnserBuffer = (unsigned char*) malloc(sizeof(struct frame));
    memset(awnserBuffer, 0, sizeof(struct frame));
    struct frame* awnserFrame = (struct frame*) awnserBuffer;
    int notSent;

    do{
        if ((recvfrom(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, &addr_len)) < 0){
            free(awnserBuffer);
            sendError(sok, end, frame->seq, TIMEOUT, awnserFrame, awnserBuffer);
            return 0;
        }

        if((frame->start_marker != START_MARKER)){//Se os Starter markers diferirem, recebeu lixo, tenta receber frame novamente
            notSent = 1;
        } else if(frame->crc != crc8((unsigned char*)frame, sizeof(struct frame) - 1)){//Se os crc8 diferirem, a mensagem veio com erro, envia nack e espera a próxima
            notSent = 1;
            sendNACK(sok, end, frame->seq, awnserFrame, awnserBuffer);
        } else {//Se estiver tudo como esperado, recebe o pacote e envia ack
            notSent = 0;
            sendACK(sok, end, frame->seq, awnserFrame, awnserBuffer);
        }
    }while(notSent);

    free(awnserBuffer);
    return 1;
}

//Retorna o tipo do frame em resposta ao pacote enviado
int waitforACK(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){
    socklen_t addr_len = sizeof(struct sockaddr);

    do{
        if(recvfrom(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, &addr_len) < 0)
            return TIMEOUT;
    } while(frame->start_marker != START_MARKER);//Se os Starter markers diferirem, recebeu lixo, tenta receber frame novamente

    return frame->type; 
}
