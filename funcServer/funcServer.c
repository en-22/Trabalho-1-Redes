
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
#include <unistd.h>
#include "funcServer.h"
#include "rawSocket.h"
#include "utils.h"

int receivePacketNT(int sok, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){//No timeout
    socklen_t addr_len = sizeof(struct sockaddr);
    unsigned char* awnserBuffer = (unsigned char*) malloc(sizeof(struct frame));
    memset(awnserBuffer, 0, sizeof(struct frame));
    struct frame* awnserFrame = (struct frame*) awnserBuffer;
    int notSent;

    do{
        recvfrom(sok, buffer, sizeof(struct frame), 0, (struct sockaddr*)&end, &addr_len);
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

int sendVidList(int socket, struct sockaddr_ll end, struct frame *frame, unsigned char *buffer){
    struct dirent* in_file;
    DIR* dir;
    unsigned char data[MAX_DATA_LENGTH] = {0};

    if (!(dir = opendir ("videos"))){
        fprintf(stderr, "Erro ao abrir diretório de videos\n");
		mountPacket(frame, frame->seq++, FIMTX, data, MAX_DATA_LENGTH);
		sendPacket(socket, end, buffer);
        return 0;
    }

    if (!(in_file = readdir(dir))){
        fprintf(stderr, "Erro ao enviar nome da lista: diretório vazio\n");
		mountPacket(frame, frame->seq++, FIMTX, data, MAX_DATA_LENGTH);
		sendPacket(socket, end, buffer);
        return 0;
    }

    do{
        if(!(strcmp (in_file->d_name, ".")) || !(strcmp (in_file->d_name, "..")))
            continue;
        if (strlen(in_file->d_name) > MAX_DATA_LENGTH){
                fprintf(stderr, "%s não enviado: nome maior que MAX_DATA_LENGTH bytes\n", in_file->d_name);//Impede o envio de nomes maiores que 63 bytes
                continue;
        }
        mountPacket(frame, frame->seq++, PRINTAR, (unsigned char*)in_file->d_name, strlen(in_file->d_name));
        sendPacket(socket, end, buffer);
    } while((in_file = readdir(dir)));

    
	mountPacket(frame, frame->seq++, FIMTX, data, MAX_DATA_LENGTH);
	sendPacket(socket, end, buffer);

    return 1;
}

int sendVideo(int socket, struct sockaddr_ll end, char* video, struct frame *frame, unsigned char *buffer){
    FILE *file;
    struct stat filestat;
    char path[74] = "videos/";
    strcat(path, video);
    stat(path, &filestat);

    file = fopen(path, "rb");
    if (file == NULL){
        sendError(socket, end, 0, NOTFOUND, frame, buffer);
        return ERROR;
    }

    unsigned char* data = malloc(sizeof(unsigned char)*MAX_DATA_LENGTH);
    memset(data, 0, sizeof(unsigned char)*MAX_DATA_LENGTH);

    int aux = filestat.st_size / MAX_DATA_LENGTH;//Frames com data cheia
    int j;
    for (int i = 0; i < aux; i++){
        j = 0;
        while(j < MAX_DATA_LENGTH){
            fread(&data[j], 1, 1, file);
            if(((data[j] == 0x88) || (data[j] == 0x81)) && j+1 < MAX_DATA_LENGTH){// adicionar 0xff impede o pacote de ser perdido
                data[j+1] = 0xff;
                j++;
            }
            j++;
        }
        mountPacket(frame, frame->seq++, DADOS, data, MAX_DATA_LENGTH);
        if(sendPacket(socket, end, buffer) == TIMEOUT){//Em caso de time out, desiste do envio
            return TIMEOUT;
        }
    }

    memset(data, 0, sizeof(unsigned char)*MAX_DATA_LENGTH);//Frame com data menor que 63 bytes
    while(j < filestat.st_size % MAX_DATA_LENGTH){
        fread(&data[j], 1, 1, file);
        if(((data[j] == 0x88) || (data[j] == 0x81)) && j < MAX_DATA_LENGTH){
            data[j+1] = 0xff;
            j++;
        }
        j++;
    }
    mountPacket(frame, frame->seq++, DADOS, data, filestat.st_size % MAX_DATA_LENGTH);
    if(sendPacket(socket, end, buffer) == TIMEOUT){
        return TIMEOUT;
    }
	
    fclose(file);

    mountPacket(frame, frame->seq++, FIMTX, data, MAX_DATA_LENGTH);
    if(sendPacket(socket, end, buffer) == TIMEOUT){
        return TIMEOUT;
    }

    free(data);
    return 1;
}

int sendDescription(int socket, struct sockaddr_ll end, char* video, struct frame *frame, unsigned char *buffer){
    uint8_t data[MAX_DATA_LENGTH] = {0};
    FILE *file;
    struct stat filestat;

    char path[70] = "videos/";
    strcat(path, video);
    stat(path, &filestat);

    file = fopen(path, "r");
    if (file == NULL){
        sendError(socket, end, 0, NOTFOUND, frame, buffer);
        return ERROR;
    }

    memcpy(data, &filestat.st_size, sizeof(__off_t)); 
    mountPacket(frame, frame->seq++, DESCRITOR, data, sizeof(__off_t));//envia o tamanho do arquivo na data
    int receivedType = sendPacket(socket, end, buffer);//Diz se o video pode ser enviado (ACK) ou se não espaço em disco(ERRO)

    fclose(file);

    return receivedType;
}

int videoExists(char* video){
    char path[70] = "videos/";
    strcat(path, video);
    if (access (path, F_OK) != -1)
        return 1;
    else
        return 0;
}







