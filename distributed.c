#include "banker.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CONNECTIONS 5

// Initialize socket for node communication
int init_socket(int node_id) {
    WSADATA wsaData;
    SOCKET server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("socket failed\n");
        WSACleanup();
        return -1;
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        printf("setsockopt failed\n");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT + node_id);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("bind failed\n");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }
    
    // Listen for connections
    if (listen(server_fd, MAX_CONNECTIONS) == SOCKET_ERROR) {
        printf("listen failed\n");
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }
    
    return (int)server_fd;
}

// Send message to another node
bool send_message(Node *source, Node *dest, Message *msg) {
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    WSADATA wsaData;
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return false;
    }
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation error\n");
        WSACleanup();
        return false;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT + dest->node_id);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        printf("Connection Failed\n");
        closesocket(sock);
        WSACleanup();
        return false;
    }
    
    if (send(sock, (char*)msg, sizeof(Message), 0) == SOCKET_ERROR) {
        printf("Send failed\n");
        closesocket(sock);
        WSACleanup();
        return false;
    }
    
    closesocket(sock);
    WSACleanup();
    return true;
}

// Process incoming message
void process_message(Node *node, Message *msg) {
    switch (msg->request_type) {
        case 0: // Resource request
            if (can_grant_request(node, msg->source_node, msg->resources)) {
                request_resources(node, msg->source_node, msg->resources);
            }
            break;
            
        case 1: // Resource release
            release_resources(node, msg->source_node, msg->resources);
            break;
            
        case 2: // Resource borrow
            process_borrow_request(node, msg);
            break;
    }
}

// Request to borrow resources from another node
bool request_borrow(Node *source_node, Node *dest_node, int *resources) {
    Message msg;
    msg.source_node = source_node->node_id;
    msg.dest_node = dest_node->node_id;
    msg.request_type = 2; // Borrow request
    
    for (int i = 0; i < source_node->num_resources; i++) {
        msg.resources[i] = resources[i];
    }
    
    return send_message(source_node, dest_node, &msg);
}

// Process borrow request from another node
bool process_borrow_request(Node *node, Message *request) {
    bool can_spare = true;
    for (int i = 0; i < node->num_resources; i++) {
        if (request->resources[i] > node->available[i]) {
            can_spare = false;
            break;
        }
    }
    
    if (can_spare) {
        for (int i = 0; i < node->num_resources; i++) {
            node->available[i] -= request->resources[i];
        }
        
        Message response;
        response.source_node = node->node_id;
        response.dest_node = request->source_node;
        response.request_type = 2;
        
        // Create a temporary node for the response
        Node temp_node;
        temp_node.node_id = request->source_node;
        return send_message(node, &temp_node, &response);
    }
    
    return false;
}

// Message handling thread
DWORD WINAPI message_handler(LPVOID arg) {
    Node *node = (Node *)arg;
    int server_fd = init_socket(node->node_id);
    
    if (server_fd < 0) {
        return 1;
    }
    
    while (1) {
        SOCKET new_socket;
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            printf("accept failed\n");
            continue;
        }
        
        Message msg;
        if (recv(new_socket, (char*)&msg, sizeof(Message), 0) > 0) {
            process_message(node, &msg);
        }
        
        closesocket(new_socket);
    }
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
} 
