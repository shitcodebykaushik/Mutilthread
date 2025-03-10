#include "proxy_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <pthread.h> // As we are using the p thread  in the code 
#include <semaphore.h>
// One hr 105 line 
#define MAX_CLIENTS 10  // Means we have the availibility to accept the request of  max to max 10 client in the . 
#define MAX_BYTES 4096
typedef struct  cache_element cache_element;
 struct cache_element
 {
    char*data;
    int len;
    char*url;
    time_t lru_time_track;
    cache_element*next;
 };
 // function declaration 
 cache_element*find(char*url);
 int add_cache_element(char*data,int size,char*url);
 void  remove_cache_element();

 int port_number = 8080;  // our server will run here 
 // Whenever any one want to conncet to our server then we need to open our socket 
 // Socket is useful for the communication over the network 
 // Every socket is connected to our proxy server , and for each client we have seprated thread and in every single thread we have their own socket .
 int proxy_socketId;
 pthread_t tid[MAX_CLIENTS];
 sem_t semaphore; // This is lock but having the multiple value .
 pthread_mutex_t lock;
 
 // Defining the head of the cache 
 cache_element*head;
 int cache_size;
// thread function 
void *thread_fn (void *socketNew){
    sem_wait(&semaphore);
    int p;
    sem_getvalue(&semaphore,p);
    printf('semaphore value is: %d\n',p);
    int *t = (int*)socketNew;
    int socket = *t;
    int bytes_send_client,len;
    char *buffer = (char*)callock(MAX_BYTES,sizeof(char));
    bzero(buffer, MAX_BYTES);
    bytes_send_client = recv(socket,buffer,MAX_BYTES,0);
    while (bytes_send_client>0){
        len = strlen(buffer);
        if (strstr(buffer,"\r\n\r\n")==NULL){
            bytes_send_client = recv(socket,buffer+len, MAX_BYTES-len,0);
        }else {
            break;
        }
    }
    char *tempReq =(char *)malloc(strlen(buffer)*sizeof(char)+1);
    for (int i =0;i<strlen(buffer);i++){
        tempReq[i] = buffer[i];
    }
    struct cache_element*temp = find(tempReq):
    if (temp!=NULL){
        int size = temp =>len/sizeof(char);
        int pos = 0;
        char response [MAX_BYTES];
        while(pos<size){
            bzero(response,MAX_BYTES);
            for(int i = 0; i<MAX_BYTES; i++){
                response[i]=temp->data[i]   //here we broke the data into small small piece for the cache 
                pos++;
            }
            send(socket,response,MAX_BYTES,0);
        }
        printf("Data retrived from the cache\n");
        printf("%s\n\n",response);
    }else if(bytes_send_client>0) {
     len = strlen(buffer);
     ParsedRequest*request = ParsedRequest_create();
     if (ParsedRequest_parse(request,buffer,len)<=0){
        printf("parsing failed\n");
     }else {
        bzero(buffer,MAX_BYTES);
        if(!strcmp(request->method,"GET")){
            if (request->host && request->path && checkHTTPversion(request->version)==1){
                bytes_send_client = handle_request(socket,request,tempReq);
                if(bytes_send_client == -1){
                    sendErrorMessage(socket,500);
                }
                }else{
                    sendErrorMessage(socket,500);
                }
            }else {
                printf("This code doesnot support any other method other than GET \n");
            }
        }
        ParsedRequest_destroy(request);
     }
    }



int main (int argc,char*argv[]){
    int client_socketId,client_len;
    struct sockaddr_in server_addr,client_addr;
    sem_init(&semaphore,0,MAX_CLIENTS);
    pthread_mutex_init(&lock,NULL);
    if(argv == 2) {
        port_number = atoi(argv[1]);
    }else {
     printf("Too few arguments\n");
     exit(1);
    }
    printf("Starting the Proxy server at port: %d\n", port_number);
    proxy_socketId = socket(AF_INET, SOCK_STREAM,0);
    if (proxy_socketId<0){
        perror("Failed to create socket\n");
        exit(1);
    }
    int reuse =1;
    if(setsockopt(proxy_socketId,SOL_SOCKET,SO_REUSEADDR,(const char*)&reuse,sizeof(reuse))<0){
     perror("setSocket failed\n");
    }
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // binding 
    if (bind(proxy_socketId,(struct sockaddr*)&server_addr,sizeof(server_addr)<0)) {
        perror ("Port is not available \n");
        exit(1);
    }
    printf ("Binding on port %d\n",port_number);
    int listen_status = listen (proxy_socketId,MAX_CLIENTS);
    if (listen_status<0){
     perror ("Error in listening\n");
     exit(1);
    }
    int i = 0;
    int Connected_socketId[MAX_CLIENTS];
    while (1){
        bzero ((char*)&client_addr,sizeof(client_addr));
        client_len = sizeof(client_addr);
        client_socketId = accept(proxy_socketId, (struct sockaddr*) &client_addr,(socklen_t*)&client_len);
        if (client_socketId<0){
            print("Not able to connect");
            exit(1);
        }else {
            Connected_socketId[i]=client_socketId;
        }
        struct sockaddr_in *client_pt =(struct sockaddr_in*)&client_addr;
        struct in_addr ip_addr = client_pt->sin_addr;
        char str [INET_ADDRSTRLEN];
        inet_netop(AF_INET,&ip_addr,str,INET_ADDRSTRLEN);
        printf("Client is connected with the port number %d and ip address is %s\n",nthos(client_addr));
        pthread_create(&tid[i],NULL, thread_fn,(void *)&Connected_socketId[i]);
        i++;
    }
    close (proxy_socketId); 
    return 0;
}