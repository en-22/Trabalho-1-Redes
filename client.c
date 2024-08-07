#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <dirent.h>
#include "funcClient.h"
#include "rawSocket.h"
#include "utils.h"


int main(){

    int choice;
    __off_t size;
    int opt = 0;
    struct sockaddr_ll end = {0};
    char video[MAX_DATA_LENGTH] = {0};
    unsigned char data[MAX_DATA_LENGTH] = {0};
    int sock = createSocket("eth0", &end);

    unsigned char* buffer = (unsigned char*) malloc(sizeof(struct frame));
    memset(buffer, 0, sizeof(struct frame));
    struct frame* frame = (struct frame*) buffer;
    frame->seq = 0;
    while (opt != 2){
        opt = showMenu();
        switch(opt){
            case 1:
                mountPacket(frame, frame->seq++, LISTAR, data, MAX_DATA_LENGTH);//Pede para listar videos
                if(sendPacket(sock, end, buffer) == TIMEOUT)//Se não houver resposta, volta ao menu
                    continue;
                if((receiveNames(sock, end, frame, buffer)) == 0)//Se não há videos, volta ao menu
                    continue;

                if((choice = showDownloadMenu()) == 1){//Se deseja baixar um video
                    printf("Digite o nome do video que deseja baixar: \n");
                    scanf("%63s", video);
                    memcpy(data, video, strlen(video));
                    mountPacket(frame, frame->seq++, BAIXAR, data, strlen(video));
                    if(sendPacket(sock, end, buffer) == TIMEOUT)//Se não houver resposta, volta ao menu
                        continue;

                    size = receiveDescription(sock, end, frame, buffer);

                    if(!(size))//Se houve problema com a leitura do tamanho do arquivo
                        continue;

                    memcpy(&size, frame->data, sizeof(__off_t));

                    if(checkDiskUsage(size)){
                        sendACK(sock, end, 0, frame, buffer);//Aceita receber video
                        printf("\nBaixando...\n");
                        receiveVideo(sock, end, frame, video, size, buffer);
                    } else{
                        printf("Não há espaço suficiente no disco\n");
                        sendError(sock, end, 0, DISKFULL, frame, buffer); 
                    }
                }
                break;
            case 2:
                free(buffer);
                printf("Adeus.\n");//Fecha o client
                return 0;
            default:
                break;
        }
    }

  return 0;
}
