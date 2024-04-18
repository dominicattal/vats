#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

SOCKET server_socket;

int seen(struct sockaddr_in *clients, struct sockaddr_in client) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sin_addr.s_addr == client.sin_addr.s_addr)
            return i;
    }
    return -1;
}

int main() {
    WSADATA wsa;
    struct sockaddr_in server_addr, client_addr, connected_clients[MAX_CLIENTS];
    int recv_len, client_addr_len, connected = 0, lens[MAX_CLIENTS];
    char names[MAX_CLIENTS][BUFFER_SIZE];

    for (int i = 0; i < MAX_CLIENTS; i++)
        connected_clients[i].sin_family = 69;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR && WSAGetLastError() != 10054) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    while (1) {
        int idx;
        size_t message_size;
        char message[BUFFER_SIZE], buffer[BUFFER_SIZE];
        client_addr_len = sizeof(client_addr);
        recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Recv failed with error code : %d", WSAGetLastError());
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }
        buffer[recv_len] = '\0';
        if ((idx = seen(connected_clients, client_addr)) == -1) {
            connected_clients[connected] = client_addr;
            lens[connected] = client_addr_len;
            strcpy(names[connected], buffer);
            idx = connected;
            connected++;
            puts("Successfully added client");
        }
        printf("Received message from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
        strcat(strcat(strcpy(message, names[idx]), "> "), buffer);
        message_size = recv_len + strlen(names[idx]) + 3 * sizeof(char);
        message[message_size] = '\0';

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (connected_clients[i].sin_family == 69)
                continue;
            if (sendto(server_socket, message, message_size, 0, (struct sockaddr *)&connected_clients[i], lens[i]) == SOCKET_ERROR) {
                printf("Sendto failed with error code : %d", WSAGetLastError());
                closesocket(server_socket);
                WSACleanup();
                return 1;
            }
        }
    }

    return 0;
}
