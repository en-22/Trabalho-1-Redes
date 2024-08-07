#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/statvfs.h>
#include "funcClient.h"
#include "rawSocket.h"
#include "utils.h"

int checkDiskUsage(__off_t size){
    struct statvfs stat;

    if (statvfs("videos/", &stat) == -1){
        fprintf(stderr, "statvfs() error\n");
        sleep(3);
        return 0;
    }
    
    if(((__off_t)(stat.f_bsize * stat.f_bavail) + (__off_t)(5000000)) > size)//impede de encher totalmente o disco
        return 1;
    
    return 0;
}

long int receiveDescription(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){

    receivePacket(sok, end, frame, buffer);

    if(frame->type == ERROR){
        printf("Erro ao baixar video\n");
        return 0;
    }

    long int size;
    memcpy(&size, frame->data, sizeof(long int));
    printf("Espaço necessário: %ld", size);
    
    return size;
}

int receiveVideo(int sok, struct sockaddr_ll end, struct frame *frame, char* videoName, size_t size, unsigned char *buffer){
    FILE *file;
    int i;
    char path[74] = "videos/";//Nome da pasta+maior nome possivel
    strcat(path, videoName);
    file = fopen(path, "wb");
    if (file == NULL){
        fprintf(stderr, "Erro ao abrir arquivo\n");
        return 0;
    }

    int aux = (int)size/MAX_DATA_LENGTH;//frames com data totalmente preenchida

    while (frame->type != FIMTX && aux--){
        receivePacket(sok, end, frame, buffer);
        if(frame->type == ERROR){//Em caso de TIMEOUT, apaga a file e desiste do download
            printf("Erro ao baixar video\n");
            fclose(file);
            remove(path);
            return 0;
        }
        i = 0;
        while(i < MAX_DATA_LENGTH){
            fwrite(&frame->data[i], 1, 1, file);
            if((frame->data[i] == 0x88) || (frame->data[i] == 0x81))//Vem acompanhado de 0xff para evitar perda do pacote, i++ o impede de ser escrito
                i++;
            i++;
        }
    }

    receivePacket(sok, end, frame, buffer);
    if(frame->type == ERROR){//Timeout
        printf("Erro ao baixar video\n");
        fclose(file);
        remove(path);
        return 0;
    }
    i = 0;
    while(i < size%MAX_DATA_LENGTH){//Recebe a data menor que 63 bytes
        fwrite(&frame->data[i], 1, 1, file);
        if((frame->data[i] == 0x88) || (frame->data[i] == 0x81))
            i++;
        i++;
    }
    
    fclose(file);

    char command[90];
    snprintf(command, sizeof(command), "xdg -open %s", path);
    system(command);

    return 1;
}

int receiveNames(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){

    printf("\e[1;1H\e[2J");
    printf("\033[1;33;40m+----------------------------------------------------------+\033[0m");
    receivePacket(sok, end, frame, buffer);
    if(frame->type == FIMTX){//Se não vier nenhum nome de video
        printf("\n\033[1;33;40m|%-58s\033[1;33m|\033[0m", "Nenhum video disponivel");
        printf("\n\033[1;33;40m+----------------------------------------------------------+\033[0m\n");
        return 0;
    }
    while (frame->type != FIMTX){
        printf("\n\033[1;33;40m|%-58s\033[1;33m|\033[0m", frame->data);
        receivePacket(sok, end, frame, buffer);
    }
    printf("\n\033[1;33;40m+----------------------------------------------------------+\033[0m\n");
    return 1;
}

int showDownloadMenu(){
    int choice;
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[1;37m| \033[1;33mDeseja Baixar Um Video?                                  \033[1;37m|\033[0m\n");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[1;37m| \033[1;37m1. Sim                                                   \033[1;37m|\033[0m\n");
    printf("\033[1;37m| \033[1;31m2. Não                                                   \033[1;37m|\033[0m\n");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[6m Digite sua escolha:                                       \n\033[0m");
    scanf("%d", &choice);

    return choice;
}

int showMenu(){
    int choice;
    printf("\e[1;1H\e[2J");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[1;37m|                            Menu                          |\033[0m\n");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[1;37m| \033[1;33mComandos:                                                \033[1;37m|\033[0m\n");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[1;37m| \033[1;37m1. Listar Videos                                         \033[1;37m|\033[0m\n");
    printf("\033[1;37m| \033[1;31m2. Sair                                                  \033[1;37m|\033[0m\n");
    printf("\033[1;37m+----------------------------------------------------------+\033[0m\n");
    printf("\033[6m Digite sua escolha:                                       \n\033[0m");
    scanf("%d", &choice);

    return choice;
}