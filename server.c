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
#define filename "file2.mp4"
#define MAX_LEN 500

struct CustomSegment {

    char payload[MAX_LEN + 1];
    int dataSize;
    int sequence;
};

double filesize(FILE *f){
    
    int prev=ftell(f);
    fseek(f, 0L, SEEK_END);
    double sz=ftell(f);
    fseek(f,prev,SEEK_SET);
    return sz;
}


int main(int argc, char *argv[]){

    size_t bytesRead = 0;
    double totalBytes = 0, sizeFile = 0;
    struct sockaddr_in server, client;
    int sockParent, acked[5], seqNumCounter = 1, expectedSeqNum = seqNumCounter ,addlen = sizeof(server);

    char buffer[MAX_LEN +1];


    struct CustomSegment window[5];
    struct CustomSegment terminationPkt = {"-1",0,-1};
    struct CustomSegment emptyPKt = {"",0,0};
    FILE *fp;

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
    server.sin_port = htons(PORT);

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
    
    while(totalBytes < sizeFile){

        for (size_t i = 0; i < 5; i++)
        {
            window[i] = emptyPKt;
        }
        

        int ic = 0;
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
            printf("\nSeqNum:%d\nData Size:%ld\n",seqNumCounter,bytesRead);
            seqNumCounter++;
        }        
        


        for (size_t j = 0; j < 5; j++)
        {
            sendto(sockParent,(struct CustomSegment*)&window[j],sizeof(window[j]),0,(struct sockaddr *)&client,addlen);
	        acked[j] = 0;
        }

        int data = 0;

        for(size_t j = 0;j<ic;j++)
        {
            data = recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
            int ack = atoi(buffer);
            printf("%d\t",ack);
    	    acked[(ack - 1) % 5] = 1;
            bzero(buffer,sizeof(buffer));
        }
        printf("\n");

        
         for (size_t i = 0; i < 5; i++)
         {
       	     if(acked[i] == 0){
                 sendto(sockParent,(struct CustomSegment*)&window[i],sizeof(window[i]),0,(struct sockaddr *)&client,addlen);
		         printf("\nRE:%d",window[i].sequence);
             }
         }
        
    }
    printf("\nTotalSize:%.2f\n",totalBytes);
    sendto(sockParent,(struct CustomSegment*)&terminationPkt,sizeof(terminationPkt),0,(struct sockaddr *)&client,addlen);   

    fclose(fp);

    return 0;
}
