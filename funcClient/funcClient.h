#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "utils.h"

//Retorna 1 se o arquivo couber e 0 caso não
int checkDiskUsage(__off_t size);
//Retorna tamanho do arquivo
long int receiveDescription(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer);
//Retorna 1 no sucesso de receber o video e 0 na falha (erro por TIMEOUT)
int receiveVideo(int sok, struct sockaddr_ll end, struct frame *frame, char* videoName, size_t size, unsigned char *buffer);
//Retorna 1 se recebeu nomes e 0 se não havia nenhum
int receiveNames(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer);
//Retorna a escolha do usuario em baixar o video (1-Sim 2-Não)
int showDownloadMenu();
//Retorna escolha do usuario em relação ao uso do client (1-Listar 2-Sair)
int showMenu();

#endif // __CLIENT_H__