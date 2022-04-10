#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

using namespace std;

#define PORT 4444
#define MAX 1024

int main(){

	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);


		   //Add function to generate random UID and insert the UID and IP here:
            string ss,sg;
            char sgc[MAX],ssc[MAX];
            ss="NEW USER?";
            strcpy(ssc,ss.c_str());
            if((send(newSocket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                perror("Send failed1");
            }
            if((read(newSocket,sgc,sizeof(sgc))==0)){
                    perror("something is wrong1");
            }
            if(strcmp(sgc,"yes")==0){
                int random = 1+(rand()%100);
                explicit_bzero(ssc,sizeof(ssc));
                ss='\0';
                ss= to_string(random);
                strcpy(ssc,ss.c_str());

                if((send(newSocket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                    perror("Send Failed2");
                }
            }
            else{
                explicit_bzero(sgc,sizeof(sgc));
                explicit_bzero(ssc,sizeof(ssc));
                ss="User ID WHat?";
                strcpy(ssc,ss.c_str());
                if((send(newSocket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                    perror("Send Failed3");
                }
                if((read(newSocket,sgc,sizeof(sgc))==0)){
                    perror("something is wrong2");
                }

                cout<<"User Joined with UserID: "<<sgc<<endl;
				explicit_bzero(ssc,sizeof(ssc));
				string tmp;
				tmp =&sgc[0];
                ss="Welcome User "+tmp;
                strcpy(ssc,ss.c_str());
                if((send(newSocket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                    perror("Send Failed4");
                }
            }



			while(1){
				explicit_bzero(buffer,1024);
				recv(newSocket, buffer, 1024, 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}else{
					printf("Client: %s\n", buffer);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
		}

	}

	close(newSocket);


	return 0;
}
