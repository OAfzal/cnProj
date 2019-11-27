// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include<fcntl.h> 
#include <netinet/in.h> 
#include <string.h> 

#define PORT 5000 
#define filename "haris.mp4"

struct CustomSegment {

    char buffer[500];
    int sequence;
}


double filesize(FILE *f){
    
    int prev=ftell(f);
    fseek(f, 0L, SEEK_END);
    double sz=ftell(f);
    fseek(f,prev,SEEK_SET);
    return sz;
}


int seqNumCounter = 0;

int main(int argc, char *argv[]){

    int sockParent, bytes_read, opt = 1;
    struct sockaddr_in server, client;
    char buffer[500] = {0};
    double buff_filesize[1] = {0};
    int addlen = sizeof(server);

    FILE *fp;
    fp = fopen(filename,"rb");
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    buff_filesize[0] = filesize(fp);

    //Creating a socket 
    if ((sockParent = socket(AF_INET, SOCK_DGRAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    //binding Socket to the port
    if(bind(sockParent,(struct sockaddr*)&server,sizeof(server))<0){

        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }   

    bytes_read =  recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
    printf("%s\n",buffer);
    
    sendto(sockParent , buff_filesize , sizeof(buff_filesize) , 0,(struct sockaddr *)&client,addlen); 

    bzero(buffer,sizeof(buffer));

    double sizeho = 0.0;
    while(1){

        size_t bytes_read = fread(buffer,1,sizeof(buffer),fp);
        if (bytes_read == 0){
            int val = sendto(sockParent,"-1",sizeof("-1"),0,(struct sockaddr *)&client,addlen);   
            printf("%d",val);
            break;
        }
        char *p = buffer;   

        while(bytes_read > 0){
            int bytes_written = sendto(sockParent,p,bytes_read,MSG_CONFIRM,(struct sockaddr *)&client,addlen);
            bytes_read -= bytes_written;
            sizeho += bytes_written;
            p += bytes_written;
        } 
    }
    printf("\n%f",sizeho);    
    return 0;

}
