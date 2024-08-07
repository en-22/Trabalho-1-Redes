#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "funcServer.h"
#include "rawSocket.h"
#include "utils.h"

int main(){
    struct sockaddr_ll end = {0};
    char videoName[MAX_DATA_LENGTH] = {0};
    int received;
    int sock = createSocket("eth0", &end);

    unsigned char* buffer = (unsigned char*) malloc(sizeof(struct frame));
    memset(buffer, 0, sizeof(struct frame));
    struct frame* frame = (struct frame*) buffer;
    frame->seq = 0;
    for (;;){//Sempre recebendo
        receivePacketNT(sock, end, frame, buffer);

        switch(frame->type){
            case LISTAR:
                if (!(sendVidList(sock, end, frame, buffer)))
                   fprintf(stderr, "Não há vídeos disponíveis.\n");
                break;

            case BAIXAR:
                memcpy(videoName, frame->data, MAX_DATA_LENGTH);

                if (videoExists(videoName)){
                    printf("Enviando tamanho de %s...\n", videoName);
                    received = sendDescription(sock, end, videoName, frame, buffer);

                    if (received == ACK){//Se há espaço para enviar o vídeo
                        printf("Enviando %s...\n", videoName);
                        received = sendVideo(sock, end, videoName, frame, buffer);
                        
                        if (received == ERROR)
                            fprintf(stderr, "Erro ao abrir vídeo\n");
                    }
                    else if (received == ERROR)
                        fprintf(stderr, "Video não encontrado ou recusado.\n");
                    
                    if (received == TIMEOUT)
                        printf("Vídeo não enviado.\n");

                } else
                    sendError(sock, end, 0, NOTFOUND, frame, buffer);//Avisa que o video não foi encontrado

                break;

            default:
                break;
        }
    }

    free(buffer);
    return 0;
}
