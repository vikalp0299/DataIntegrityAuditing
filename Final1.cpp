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
}