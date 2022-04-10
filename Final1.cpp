#include <iostream>
#include <stdlib.h>
#include <queue>
#include <stdio.h>
#include <mysql/mysql.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define PORT 4444
#define MAX 1024

using namespace std;

class BST{
    int ID;
    string IP;
    BST *left, *right, *back;
public:
    BST();
    BST(int,string);
    BST* insertnode(BST*, int,string);
    void inorder(BST*);
    BST* fndusr(BST*,int,int,int);
    BST* delnode(BST*,int);
    BST* minvalnode(BST*);
    BST* inittree();
};

BST* BST:: minvalnode(BST* root){
    BST* temp=root;
    while(temp && temp->left!=NULL){
        temp=temp->left;
    }
    return temp;

}

BST* BST::delnode(BST* root, int val){
    if(root == NULL){
        return root;
    }

    if(root->ID>val)
        root->left=delnode(root->left,val);
    else if(root->ID < val)
        root->right=delnode(root->right,val);
    else{
        if(root->left==NULL && root->right==NULL){
            return NULL;
        }
        else if (root->left == NULL) {
			BST* temp = root->right;
			free(root);
			return temp;
		}
		else if (root->right == NULL) {
			BST* temp = root->left;
			free(root);
			return temp;
		}
        BST* temp = minvalnode(root->right); 
        root->ID=temp->ID;
        root->back=temp->back;
        root->right=delnode(root->right,temp->ID);
    }
    
    return root;

}

BST::BST(){
    ID=0;
    left=right=back=NULL;
}
BST::BST(int val,string IP){
    ID=val;
    this->IP=IP;
    left=right=back=NULL;
}

BST* BST :: insertnode(BST* root, int val,string IP=string()){
    if(!root){
        return new BST(val,IP);
    }

    if(val>root->ID){
        root->right=insertnode(root->right,val,IP);
    }
    else{
        root->left=insertnode(root->left,val,IP);
    }
    return root;
}

void BST::inorder(BST* root){
    if(!root)
        return;
    inorder(root->left);
    cout<<root->ID<< " ";
    inorder(root->right);
}

BST* BST::fndusr(BST* root, int UID,int sig,int FID=0 ){
 
  if(root==NULL){
      return root;
    }
   queue<BST*>q;
   BST* temp =NULL;
   q.push(root);
   while(!q.empty()){
       temp=q.front();
       q.pop();
       

      if(temp->ID != UID && (!temp->left) && (!temp->right) ){
            cout<<"USER DOESNT EXISTS\n";
            return root;
        } 
        if(temp->ID==UID){
           cout<<"Usr Found\n";
           break;
        }
        if(temp->ID > UID){
            
            q.push(temp->left);
        }
        if(temp->ID < UID ){
            
            q.push(temp->right);
       
        }
    }
    
        

    switch(sig){
        case 0 : cout<<"Inserting File\n";
                temp->back=insertnode(temp->back,FID);
                break;
        
        case 1 : cout<<"Deleting File\n";
                temp->back=delnode(temp->back,FID);
                 break;
        case 2 : cout<<"Traversing\n";
                inorder(temp->back);
                 break;
        default : break; 
    }
    return root;
}

MYSQL  *conn; 
MYSQL_ROW row;
MYSQL_RES *res;

BST* BST::inittree(){
    //initializes tree from db
    
    BST* temp = NULL;

    int UID,FID;
    conn =mysql_init(0);
    if((conn = mysql_real_connect(conn,"localhost","root","83bb9542","storage",3306,NULL,0))==NULL){
        perror("Connection can't be established");
        return NULL;
    }

    mysql_query(conn,"Select * FROM User ORDER by Joined ASC");
    res = mysql_use_result(conn);
    string s;
    while(row=mysql_fetch_row(res)){
        UID = atoi(row[0]);
        s = row[1];
        temp = insertnode(temp,UID,s);
    }
    mysql_query(conn,"Select FID,UserID FROM Files ORDER by AddedAt ASC");
    res = mysql_use_result(conn);
    while(row=mysql_fetch_row(res)){
        FID=atoi(row[0]);
        cout<<FID<<" ";
        UID = atoi(row[1]);
        temp = fndusr(temp,UID,0,FID);
    }    

    
    return temp;


}

int main()
{
    int sig,ch;
	BST b;
    //initialize tree
    BST* root = b.inittree();
    cout<<endl;
    b.inorder(root);
    cout<<endl;

    //server 

    
    int opt =true;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients =30, activity, i ,valread, sd,max_sd;
    struct sockaddr_in address;

    char buffer[1024];

    fd_set readfs;
    //initialize client socket
    string msg = "HENLO \r\n";
    char *message;
    message=&msg[0];

    for(i=0;i < max_clients;i++ ){
        client_socket[i]=0;
    }
    //create a master socket
    if((master_socket=socket(AF_INET,SOCK_STREAM,0))==0){
        perror("Socket Failed\n");
        exit(EXIT_FAILURE);
    }
    //allow multiple connection
    if(setsockopt(master_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&opt,sizeof(opt)) < 0 ){
        perror("Setsockopt");
        exit(EXIT_FAILURE   );
    }

    address.sin_family=AF_INET;
    address.sin_addr.s_addr = inet_addr("10.0.30.4");
    address.sin_port=htons(PORT);

    //binding the socket

    if(bind(master_socket, (struct sockaddr*)&address,sizeof(address))<0){
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }
    cout<<"Listening on port"<<PORT<<endl;
    if(listen(master_socket,3)<0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    addrlen=sizeof(address);
    cout<<"Waiting for connection...\n"<<endl;

    while(true){
        //clear socket set
        FD_ZERO(&readfs);

        //add master socketto set
        FD_SET(master_socket,&readfs);
        max_sd = master_socket;

        //add childern socket to set
        for(i=0;i<max_clients;i++){
            //socket descriptor
            sd = client_socket[i];
            //add valid socket descriptor to read list
            if(sd>0){
                FD_SET(sd,&readfs);
            }
            if (max_sd < sd){
                max_sd =sd;
            }
        }

        activity = select(max_sd+1,&readfs,NULL,NULL,NULL);
        if((activity<0) && (errno!=EINTR)){
            perror("select error");
        }

        if(FD_ISSET(master_socket,&readfs)){
            if ((new_socket=accept(master_socket,(struct sockaddr*)&address,(socklen_t*)&addrlen))<0){
                perror("Accept failure");
                exit(EXIT_FAILURE);
            }
            cout<<"New Connection\n socket fd: "<<new_socket<<"\n IP: "<<inet_ntoa(address.sin_addr)<<"\n port: "<< ntohs(address.sin_port)<<endl;
            //send new connection greeting
            if(send(new_socket,message,strlen(message),0)!= strlen(message)){
                perror("send failure");
            }
	    else
                cout<<"Welcome Message sent successfully"<<endl;
            //Add function to generate random UID and insert the UID and IP here:
            string ss,sg;
            char sgc[MAX],ssc[MAX];
            ss="NEW USER?";
            strcpy(ssc,ss.c_str());
            if((send(new_socket,ssc,sizeof(ssc),0))==sizeof(ssc)){
                perror("Send failed");
            }
            if((read(new_socket,sgc,sizeof(sgc))==0)){
                    perror("something is wrong");
            }
            if(strcmp(sgc,"yes")==0){
                int random = 1+(rand()%100);
                explicit_bzero(ssc,sizeof(ssc));
                ss='\0';
                ss= to_string(random);
                strcpy(ssc,ss.c_str());

                if((send(new_socket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                    perror("Send Failed@!");
                }
            }
            else{
                explicit_bzero(sgc,sizeof(sgc));
                explicit_bzero(ssc,sizeof(ssc));
                ss="User ID WHat?";
                strcpy(ssc,ss.c_str());
                if((send(new_socket,ssc,sizeof(ssc),0))!=sizeof(ssc)){
                    perror("Send Failed@!");
                }
                if((read(new_socket,sgc,sizeof(sgc))==0)){
                    perror("something is wrong");
                }

                cout<<"User Joined with UserID: "<<sgc<<endl;
            }

            //add new socket to list
            for(i=0;i<max_clients;i++){
                if(client_socket[i]==0){
                    client_socket[i]=new_socket;
                    cout<<"Adding to the list of socket as "<<i<<endl;
                    break;
                }
            }
        }
        //else I/O on other sockets
        for(i=0;i<max_clients;i++){
            sd=client_socket[i];
            if(FD_ISSET(sd,&readfs)){
                //check for closing and read the input
		valread=read(sd,buffer,1024);
		buffer[valread]='\0';
                if((strcmp(buffer,"exit")==0)||(strcmp(buffer,"Exit")==0)){
                //someone is disconnecting
		            send(sd,buffer,strlen(buffer),0);
                    getpeername(sd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
                    cout<<"Client Disconnected!\n IP: "<<inet_ntoa(address.sin_addr)<<"\tPORT: "<<ntohs(address.sin_port)<<endl;

                    close(sd);
                    client_socket[i]=0 ;
                }
                else{
                    //whatever gonna happen or i have to add do it here

                    
                    send(sd,buffer,strlen(buffer),0);
                }
            }
        }
    }


 
    
    return 0;
}   
