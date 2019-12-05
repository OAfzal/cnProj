// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include<fcntl.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/time.h>

#define PORT 5000 
#define filename "haris.mp4"

struct CustomSegment {

    char payload[500];
    int sequence;
};


int main(int argc, char *argv[]){

    int sockParent, opt = 1, acked[5],seqNumCounter = 1;
    struct sockaddr_in server, client;
    char buffer[500] = {0};
    int addlen = sizeof(server);
    struct CustomSegment window[5];
    struct CustomSegment pkt = {"-1",0};
    struct CustomSegment emptyPKt = {"",0};

    size_t bytes_read = 0;

    FILE *fp, *fp2;
    fp2 = fopen("data.txt","w");
    fp = fopen(filename,"rb");
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }


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


    bzero(buffer,sizeof(buffer));

    sendto(sockParent,"Hello From Server",strlen("Hello From Server"),0,(struct sockaddr *)&client,addlen);

    bytes_read = 0;
    double dataRead = 0;

    while(1){

        for (size_t i = 0; i < 5; i++)
        {
            acked[i] = 0;
            window[i].sequence = seqNumCounter;
            bytes_read = fread(window[i].payload,1,sizeof(window[i].payload),fp);

            if(bytes_read == 0){
                window[i] = emptyPKt;
                break;
            }

            printf("\nSeqNum:%d\nData Size:%ld\n",seqNumCounter,strlen(window[i].payload));

            sprintf(buffer,"%s",window[i].payload);
            fputs(buffer,fp2);
            seqNumCounter++;
            bzero(buffer,sizeof(buffer));
        }
        

        for (size_t i = 0; i < 5; i++)
        {
            if(window[i].sequence == 0){
                break;
            }   
            sendto(sockParent,(struct CustomSegment*)&window[i],sizeof(window[i]),0,(struct sockaddr *)&client,addlen);
            
            char data[1000];
            sprintf(data,"acked,%s\n",buffer);
            fputs(data,fp2);

            bzero(data,sizeof(data));

            bzero(buffer,sizeof(buffer));
        }

        for (size_t i = 0; i < 5; i++)
        {
            recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
        }
        

        if (bytes_read == 0){
            printf("\n");
            int val = sendto(sockParent,(struct CustomSegment*)&pkt,sizeof(pkt),0,(struct sockaddr *)&client,addlen);   
            break;
        }

    }
    
    sprintf(pkt.payload,"%d",seqNumCounter);    
    printf("%s",pkt.payload);
    sendto(sockParent,(struct CustomSegment*)&pkt,sizeof(pkt),0,(struct sockaddr *)&client,addlen);   
    printf("\nSeqNumCounter:%d",seqNumCounter);

    fclose(fp);
    fclose(fp2);

    printf("Done");    
    return 0;

}
