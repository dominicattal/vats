#include <stdio.h>
#include <winsock2.h>

#define SERVER_IP "169.254.176.170"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    int recv_len;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send data to server
    if (sendto(client_socket, "Hello from client!", strlen("Hello from client!"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Send failed : %d", WSAGetLastError());
        return 1;
    }

    // Receive response from server
    recv_len = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (recv_len == SOCKET_ERROR) {
        printf("Recv failed : %d", WSAGetLastError());
        return 1;
    }
    
    // Add null terminator to received data
    buffer[recv_len] = '\0';
    
    // Print server response
    printf("Server response: %s\n", buffer);

    // Close socket and cleanup
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
