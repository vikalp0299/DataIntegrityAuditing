#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;

#define PORT 4444

int getRes(){
	int i;
	cout<<"1.Insert File\n2.Delete File\n";
	cin>>i;
	return i;
}
int main(){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	bzero(buffer,1024);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Socket failed.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("10.0.30.4");
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	while(1){
		bzero(buffer,1024);
		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: \t%s\n", buffer);
		}
		//int i =	getRes();

		
		printf("Client: \t");
		scanf("%s", &buffer[0]);
		send(clientSocket, buffer, strlen(buffer), 0);

		if(strcmp(buffer, ":exit") == 0){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}

		
	}

	return 0;
}
