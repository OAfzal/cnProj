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

    int sockParent, opt = 1, acked[5],seqNumCounter = 0;
    struct sockaddr_in server, client;
    char buffer[500] = {0};
    int addlen = sizeof(server);
    struct CustomSegment window[5];
    struct CustomSegment pkt = {"-1",0};
    
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

    while(1){

        for (size_t i = 1; i <= 5; i++)
        {
            acked[i] = 0;
            window[i-1].sequence = seqNumCounter;
            bytes_read = fread(window[i-1].payload,1,sizeof(window[i-1].payload),fp);
            seqNumCounter++;
            // bzero(buffer,sizeof(buffer));
            // printf("Sequence:%s\n",buffer);
        }

        for (size_t i = 1; i <= 5; i++)
        {   
            sendto(sockParent,(struct CustomSegment*)&window[i-1],sizeof(window[i-1]),0,(struct sockaddr *)&client,addlen);
            recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
            // while(1){
                
            // }
            // acked[atoi(buffer)] = 1;
            printf("Sequence:%s\n",buffer);
            
            char data[1000];
            sprintf(data,"acked,%s\n",buffer);
            fputs(data,fp2);

            bzero(data,sizeof(data));
            bzero(buffer,sizeof(buffer));
        }

        // for (size_t i = 0; i < 5; i++)
        // {
        //     char data[50];
        //     // printf("acked[%ld] = %d\n",i,acked[i]);
        //     sprintf(data,"acked[%ld] = %d\n",i,acked[i]);
        //     fputs(data,fp2);
        // }
        
        if (bytes_read == 0){
            printf("\nhere");
            int val = sendto(sockParent,(struct CustomSegment*)&pkt,sizeof(pkt),0,(struct sockaddr *)&client,addlen);   
            printf("%d",val);
            fclose(fp2);
            break;
        }
        seqNumCounter++;

        // while((recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen))>0){
        //     printf("Sequence:%s\n",buffer);
        //     bzero(buffer,sizeof(buffer));
        // }

        // char *p = (window[0].payload);   

        // sendto(sockParent,p,sizeof(window[0].payload),0,(struct sockaddr *)&client,addlen);

        // int bytes_written =  recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);

        // printf("\r%s",buffer);

    }   
    printf("Done");    
    return 0;

}
