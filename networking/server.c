#include <stdio.h>
#include <winsock2.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    int recv_len, client_addr_len;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }

    // Prepare the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    // Receive and handle incoming messages
    while (1) {
        client_addr_len = sizeof(client_addr);
        recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len == SOCKET_ERROR) {
            printf("Recv failed with error code : %d", WSAGetLastError());
            break;
        }

        // Add null terminator to received data
        buffer[recv_len] = '\0';

        // Print received message and client address
        printf("Received message from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        // Send response back to client
        if (sendto(server_socket, buffer, recv_len, 0, (struct sockaddr *)&client_addr, client_addr_len) == SOCKET_ERROR) {
            printf("Sendto failed with error code : %d", WSAGetLastError());
            break;
        }
    }

    // Close socket and cleanup
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
