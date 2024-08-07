#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct __attribute__((packed)) frame{
    unsigned int start_marker: 8;
    unsigned int size: 6;
    unsigned int seq: 5;
    unsigned int type: 5;
    unsigned char data[63];
    unsigned int crc: 8;
};

#define ACK 0 //00000
#define NACK 1 //00001
#define LISTAR 10 //01010
#define BAIXAR 11 //01011 
#define PRINTAR 16 //10000
#define DESCRITOR 17 //10001
#define DADOS 18 //10010
#define FIMTX 30 //11110
#define ERROR 31 //11111

#define NEGACESS 1
#define NOTFOUND 2
#define DISKFULL 3
#define TIMEOUT 4

#define START_MARKER 0x7E
#define MAX_DATA_LENGTH 63

void sendError(int sok, struct sockaddr_ll end, unsigned char seq, int errortype, struct frame* frame, unsigned char* buffer);

unsigned int crc8(unsigned char* data, int size);

void mountPacket(struct frame* frame, unsigned char seq, unsigned char type, unsigned char* data, __off_t size);

void sendACK(int sok, struct sockaddr_ll end, unsigned char seq, struct frame* frame, unsigned char* buffer);

void sendNACK(int sok, struct sockaddr_ll end, unsigned char seq, struct frame* frame, unsigned char* buffer);

//Retorna tipo de resposta recebida para o envio do pacote (ACK, ERROR OU TIMEOUT)
int sendPacket(int sok, struct sockaddr_ll end, unsigned char *buffer);

//Retorna 1 no sucesso de envio e 0 no timeout
int receivePacket(int sok, struct sockaddr_ll end, struct frame* frame, unsigned char* buffer);

//Retorna tipo de resposta recebida (ACK, ERROR OU TIMEOUT)
int waitforACK(int sok, struct sockaddr_ll end, struct frame* frame, unsigned char* buffer);

#endif // _UTILS_H_