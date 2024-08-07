#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/time.h> 
#include "rawSocket.h"

int createSocket(char* device, struct sockaddr_ll *end){
    int soc;
    struct packet_mreq mreq = {0};

    if ((soc = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1){ //cria socket
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root\n");
        return -1;
    }

    int ifindex = if_nametoindex(device);

    end->sll_family = AF_PACKET;
    end->sll_protocol = htons(ETH_P_ALL);
    end->sll_ifindex = ifindex;

    if (bind(soc, (struct sockaddr*) & *end, sizeof(*end)) == -1){// Inicializa socket
        fprintf(stderr, "Erro no bind do socket\n");
        return -1;
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.mr_ifindex = ifindex;
    mreq.mr_type = PACKET_MR_PROMISC;
    
    if (setsockopt(soc, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1){// Não joga fora o que identifica como lixo: Modo promíscuo
        fprintf(stderr, "Erro ao fazer setsockopt\n");
        return -1;
    }

    const int timeoutMillis = 5000; // 5000 milisegundos de timeout 
    struct timeval timeout = { .tv_sec = timeoutMillis / 1000, .tv_usec = (timeoutMillis % 1000) * 1000 };
    if (setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)) == -1){//tiemout
        fprintf(stderr, "Erro ao fazer setsockopt\n");
        return -1;
    }

    return soc;
}