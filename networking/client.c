#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

#define NAME_LENGTH 20
#define MESSAGE_LENGTH 100

struct thread_args {
    SOCKET client_socket;
    struct sockaddr_in server_addr;
};

void *listener(void *vargp) {
    struct thread_args *args = vargp;
    int recv_len;
    char buffer[BUFFER_SIZE];
    while (1) {
        recv_len = recvfrom(args->client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (recv_len == SOCKET_ERROR) {
            printf("Recv failed : %d", WSAGetLastError());
            closesocket(args->client_socket);
            WSACleanup();
            exit(1);
        }
        buffer[BUFFER_SIZE - 1] = '\0';
        puts(buffer);
    }
}

int read_name(char *name) {
    size_t len;
    printf("Enter username: ");
    fgets(name, NAME_LENGTH, stdin);
    len = strlen(name);
    if (len > 0 && name[len - 1] == '\n') {
        name[len - 1] = '\0';
        len = strlen(name);
    }
    return strlen(name) > 0;
}

int read_message(char *message, char *name) {
    fgets(message, MESSAGE_LENGTH, stdin);
    int len = strlen(message);
    if (len > 0 && message[len - 1] == '\n')
        message[len - 1] = '\0';
    return strlen(message) > 0;
}

int main(int argc, char **argv) {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    int recv_len;
    char buffer[BUFFER_SIZE], name[NAME_LENGTH], message[MESSAGE_LENGTH + NAME_LENGTH];
    pthread_t thread_id;

    if (argc != 3) {
        puts("Missing server IP address or port number. client [IP_address] [Port #]");
        return 1;
    }

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    if (!(server_addr.sin_addr.s_addr = inet_addr(argv[1]))) {
        printf("Invalid IP address");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    if (!(server_addr.sin_port = htons(atoi(argv[2])))) {
        printf("Invalid port number");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    };

    while (!read_name(name))
        puts("Name too short.");

    if (sendto(client_socket, name, strlen(name), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Send failed : %d", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    puts("A");
    struct thread_args args;
    args.client_socket = client_socket;
    args.server_addr = server_addr;
    pthread_create(&thread_id, NULL, listener, &args);
    puts("B");
    recv_len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == SOCKET_ERROR) {
        printf("Recv failed : %d", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    
    while (!read_message(message, name))
        ;

    while (strcmp(message, "exit")) {
        if (sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            printf("Send failed from here : %d", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }

        while (!read_message(message, name))
            ;
    }

    char disconnected[] = " disconnected";
    char disconnect_message[strlen(name) + strlen(disconnected)];
    strcpy(disconnect_message, name);
    strcat(disconnect_message, disconnected);

    if (sendto(client_socket, disconnect_message, strlen(disconnect_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Send failed from here : %d", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    recv_len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == SOCKET_ERROR) {
        printf("Recv failed : %d", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    closesocket(client_socket);
    WSACleanup();

    return 0;
}
