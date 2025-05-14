#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define SERVER_PORT 12345

void initializeSock(){
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData)!=0){
        printf("WsaStartup failed with error %ld\n",WSAGetLastError());
        exit(1);
    }
}

int createServerSocket(){
    SOCKET serverSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if  (serverSocket == INVALID_SOCKET){
        printf("Socket failed with error %ld\n",WSAGetLastError());
        WSACleanup();
        exit(1);
    }
    return serverSocket;
}

void bindSocket(SOCKET serverSocket){
  struct sockaddr_in seraddr;
  memset(&seraddr, 0,sizeof(seraddr));
  seraddr.sin_family = AF_INET;
  seraddr.sin_addr.s_addr = INADDR_ANY;
  seraddr.sin_port = htons(SERVER_PORT);

  if(bind(serverSocket,(struct sockaddr*)&seraddr,sizeof(seraddr))==SOCKET_ERROR){
    printf("Bind failed with error %d\n",WSAGetLastError());
    closesocket(serverSocket);
    WSACleanup();
    exit(1);
  }

}

void listenClients(SOCKET serverSocket){
    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR){
        printf("Listen failed with error %dld\n",WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }
    printf("Server listening on port %d..\n",SERVER_PORT);
}

void handleClient(SOCKET serverSocket) {
    SOCKET client = accept(serverSocket, 0, 0);
    if (client == INVALID_SOCKET) {
        printf("Accept failed with error %ld\n", WSAGetLastError());
        return;
    }
    char request[256] = {0};
    int bytesReceived = recv(client, request, sizeof(request) - 1, 0);
    printf("Received request:\n%s\n", request);

    if (bytesReceived > 0) {
        if (memcmp(request, "GET /", 5) == 0) {
            FILE* f = fopen("index.html", "r");
            if (!f) {
                const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\nConnection: close\r\n\r\n404 Not Found";
                send(client, notFound, strlen(notFound), 0);
            } else {
                fseek(f, 0, SEEK_END);
                long fileSize = ftell(f);
                fseek(f, 0, SEEK_SET);
                char* fileData = malloc(fileSize + 1);
                fread(fileData, 1, fileSize, f);
                fclose(f);
                fileData[fileSize] = '\0';
                char headers[128];
              snprintf(headers, sizeof(headers),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n\r\n", fileSize);
                send(client, headers, strlen(headers), 0);
                send(client, fileData, fileSize, 0);
                free(fileData);
            }
        } else {
           const char* notImplemented = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 15\r\nConnection: close\r\n\r\n501 Not Implemented";
         send(client, notImplemented, strlen(notImplemented), 0);
        }
    } else {
       printf("recv failed\n");
    }
    Sleep(100);
    closesocket(client); 
}

int main(){
    initializeSock();
    SOCKET serverSocket = createServerSocket();
    bindSocket(serverSocket);
    listenClients(serverSocket);
    while(1){
      handleClient(serverSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    
}