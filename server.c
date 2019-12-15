#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include<fcntl.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/time.h>

#define MAX_LEN 500
#define ANSI_COLOR_GREEN "\x1b[32m"

struct CustomSegment {

    char payload[MAX_LEN + 1];
    int dataSize;
    int sequence;
};


//Reading FileSize
double filesize(FILE *file){
    
    int previous=ftell(file);
    fseek(file, 0L, SEEK_END);
    double size = ftell(file);
    fseek(file,previous,SEEK_SET);
    return size;
}


int main(int argc, char *argv[]){

    size_t bytesRead = 0;
    double totalBytes = 0, sizeFile = 0;
    struct sockaddr_in server, client;
    int sockParent, acked[5], seqNumCounter = 1,addlen = sizeof(server);

    char buffer[MAX_LEN +1];

    struct CustomSegment window[5];
    struct CustomSegment terminationPkt = {"-1",0,-1};
    struct CustomSegment emptyPKt = {"",0,0};
    FILE *fp;

    if(argc != 3){
        printf("\nPlease provide 2 argumnets in the format:");
        printf(ANSI_COLOR_GREEN"\n'./server FILENAME PORT'\n\n");
        exit(2);
    }

    char *filename = argv[1];
    int port = atoi(argv[2]);

    fp = fopen(filename,"rb");
    
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    sizeFile = filesize(fp);

    //Creating a socket 
    if ((sockParent = socket(AF_INET, SOCK_DGRAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    //binding Socket to the port
    if(bind(sockParent,(struct sockaddr*)&server,sizeof(server))<0){

        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }   


    bytesRead =  recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
    printf("%s\n",buffer);

    bzero(buffer,sizeof(buffer));

    sendto(sockParent,"Hello From Server",strlen("Hello From Server"),0,(struct sockaddr *)&client,addlen);

    bytesRead = 0;

    // THE CODE BELOW:
    // 1. READS CHUNKS OF FILES
    // 2. STRUCTURES THEM INTO CUSTOM SEGMENTS
    // 3. FILL UP SEGMENT WITH METADATA
    // 4. SEND 5 SEGMENTS
    // 5. WAIT FOR ACKS
    // 6. RESEND NACKED SEGMENTS
    // 7. CLOSES CONNECTION

    while(totalBytes < sizeFile){

        //INIT WINDOW WITH EMPTY SEGMETNS

        for (size_t i = 0; i < 5; i++)
        {
            window[i] = emptyPKt;
        }
        
        // READS CHUNKS OF FILES
        // STRUCTURES THEM INTO CUSTOM SEGMENTS
        // IF EOF PUT TERMINATION PACKET IN WINDOW
    
        int ic = 0;
        printf("\n\nSENDING:\n");
        while ((ic < 5))
        {
            window[ic].sequence = seqNumCounter;
            bytesRead = fread(window[ic].payload,1,sizeof(window[ic].payload),fp);

            if(bytesRead == 0){
                window[ic] = terminationPkt;
                break;
            }
            window[ic].dataSize = bytesRead;
            totalBytes += bytesRead;
            ic++;
            printf("%d\t",seqNumCounter);
            seqNumCounter++;
        }        
        
        // SEND SEGMENTS IN WINDOW
        // INIT ACKED ARRAY TO 0

        for (size_t j = 0; j < 5; j++)
        {
            sendto(sockParent,(struct CustomSegment*)&window[j],sizeof(window[j]),0,(struct sockaddr *)&client,addlen);
	        acked[j] = 0;
        }

        // WAIT FOR ACKS AND NACKS OF SEGMENTS SEND
        // PUT ACKS FOR SEGMENTS IN CORRESPONDING PLACE IN ACKED ARRAY

        printf("\nAcks:\n");
        for(size_t j = 0;j<ic;j++)
        {
            recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
            int ack = atoi(buffer);
            printf("%d\t",ack);
    	    acked[(ack - 1) % 5] = 1;
            bzero(buffer,sizeof(buffer));
        }

        printf("\n");

        // RESEND PACKETS FOR WHICH NACK WAS RECIEVE
        
         for (size_t i = 0; i < 5; i++)
         {
       	     if(acked[i] == 0){
                 sendto(sockParent,(struct CustomSegment*)&window[i],sizeof(window[i]),0,(struct sockaddr *)&client,addlen);
		         printf("\nRESENDING:%d",window[i].sequence);
             }
         }
        
    }
    // RESEND TERMINATION PACKET IN CASE THE ABOVE WAS DROPPED
    sendto(sockParent,(struct CustomSegment*)&terminationPkt,sizeof(terminationPkt),0,(struct sockaddr *)&client,addlen);   

    printf("\nTotalSize:%.2f\n",totalBytes);
    fclose(fp);
    return 0;
}
