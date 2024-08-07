#ifndef _SERVER_H_
#define _SERVER_H_

#include "utils.h"
//Retorna 0 se a lista estiver vazia ou o diretorio nao abrir e 1 no sucesso de envio
int sendVidList(int socket, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer);
//Retorna 1 no sucesso de envio e 0 no fracasso (TIMEOUT)
int sendVideo(int socket, struct sockaddr_ll end, char* video, struct frame *frame, unsigned char *buffer);
//Retorna tipo de resposta recebida para o envio do pacote (ACK, ERROR OU TIMEOUT)
int sendDescription(int socket, struct sockaddr_ll end, char* video, struct frame *frame, unsigned char *buffer);
//Retorna 1 se o video existe e 0 caso contrário
int videoExists(char* video);
#endif // _SERVER_H_