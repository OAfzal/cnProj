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
#define filename "file.mp4"
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
    FILE *fp, *fp2;

    fp = fopen(filename,"rb");
    fp2 = fopen("data.mp4","wb");
    
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

    // while (totalBytes < sizeFile)
    // {
    //     int i = 0;
    //     while ((i < 5))
    //     {
    //         bytesRead = fread(window[i].payload,1,sizeof(window[i].payload),fp);
    //         if(bytesRead == 0){
    //             break;
    //         }
    //         window[i].dataSize = bytesRead;
    //         totalBytes += bytesRead;
    //         i++;
    //     }
    //     // printf("\nj:%d\tTotalBytes:%.2f",j,totalBytes);
    // }

    // printf("\nTotalBytes:%.2f",totalBytes);
    
    while(totalBytes < sizeFile){

        int ic = 0;
        while ((ic < 5))
        {
            window[ic].sequence = seqNumCounter;
            bytesRead = fread(window[ic].payload,1,sizeof(window[ic].payload),fp);
            if(bytesRead == 0){
                window[ic] = emptyPKt;
                // printf("\ntoota");
                // break;
            }
            window[ic].dataSize = bytesRead;
            totalBytes += bytesRead;
            ic++;
            printf("\nSeqNum:%d\nData Size:%ld\n",seqNumCounter,bytesRead);
            seqNumCounter++;
        }        

        for (size_t j = 0; j < 5; j++)
        {
            // if(window[j].sequence == 0){
            //     continue;
            // }   
            sendto(sockParent,(struct CustomSegment*)&window[j],sizeof(window[j]),0,(struct sockaddr *)&client,addlen);
	        acked[j] = 0;
        }

        int data = 0;

        for(size_t j = 0;j<5;j++)
        {
            data = recvfrom(sockParent,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&addlen);
            int ack = atoi(buffer);
            printf("%d\t",ack);
	        acked[(ack - 1) % 5] = 1;
            bzero(buffer,sizeof(buffer));
        }
        printf("\n");

        // for (size_t j = 0; j < 5; j++)
        // {
        //     if(acked[i] == 0){
        //         sprintf(buffer,"\n%d",window[i].sequence);
        //         fputs(buffer,fp2);
        //     }
        // }
        
         for (size_t i = 0; i < 5; i++)
         {
       	     if(acked[i] == 0){
                sendto(sockParent,(struct CustomSegment*)&window[i],sizeof(window[i]),0,(struct sockaddr *)&client,addlen);
		        printf("\nRE:%d",window[i].sequence);
             }
         }
        
    }
    printf("\nTOtalSIze:%.2f\n",totalBytes);
    // // sprintf(pkt.payload,"%d",seqNumCounter);    
    // // printf("%s",pkt.payload);
    for (size_t i = 0; i < 2; i++)
    {
        sendto(sockParent,(struct CustomSegment*)&terminationPkt,sizeof(terminationPkt),0,(struct sockaddr *)&client,addlen);   
    }
    // // printf("\nSeqNumCounter:%d",seqNumCounter);

    fclose(fp);
    fclose(fp2);

    // printf("\nTotalBYtes:%d\n",totalBytes);
    printf("Done");    
    return 0;

}
