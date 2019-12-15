#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_LEN 500
#define ANSI_COLOR_GREEN "\x1b[32m"

struct CustomSegment {

    char payload[MAX_LEN + 1];
	int dataSize;
    int sequence;
};


int main(int argc, char *argv[]) 
{ 

	int seqNumCounter = 1;
	int sock = 0, bytes_read,leng;

	struct sockaddr_in serv_addr; 

	char *hello = "Hello from client"; 
	char buffer[500] = {0}; 

	struct CustomSegment* pkt = malloc(sizeof(struct CustomSegment));
	struct CustomSegment window[5];	
	struct CustomSegment emptyPKt = {"",0};

    FILE *fp;

	if(argc != 4){
        printf("\nPlease provide 3 argumnets in the format:");
        printf(ANSI_COLOR_GREEN"\n'./server FILENAME PORT IP_ADDRESS'\n\n");
        exit(2);
    }

    char *filename = argv[1];
    int port = atoi(argv[2]);
	char *ipAddr = argv[3];
	
    fp = fopen(filename,"wb");

    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
	
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port); 
	

	if(inet_pton(AF_INET, ipAddr, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 	
		return -1; 
	}

	sendto(sock , hello , strlen(hello) , 0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	


	bytes_read = recvfrom(sock, &buffer, sizeof(buffer), 0,(struct sockaddr *)&serv_addr,&leng);

	printf("%s\n",buffer);

	bzero(buffer,sizeof(buffer));

	int finished = 0;
	int totalSize = 0;

	while (1) {

		if(finished){
			break;
		}

		bzero(buffer,sizeof(buffer));
		bzero(pkt,sizeof(struct CustomSegment));

		//INIT WITH EMPTY SEGMENTS

		for (size_t j = 0; j < 5; j++)
		{
			window[j] = emptyPKt;
		}
		
		size_t i = 0;
		while(i<5){
			
			bytes_read = recvfrom(sock, pkt, sizeof(*pkt),MSG_DONTWAIT,(struct sockaddr *)&serv_addr,&leng);

			//IF TERMINATION SEGMENT ADD TO WINDOW AND BREAK
			if(pkt->sequence == -1){
				finished =1;
				printf("\nTransfer Complete");		
				printf("\nSeqNum:%d\tFinished:%d\n",pkt->sequence,finished);
				window[i] = *pkt;
				break;
			}

			//PLACE SEGMENT REC INTO WINDOW
			window[(pkt->sequence - 1) % 5] = *pkt;

			//IF SEGMENT DUPLICATE DISCARD THEM
			if(pkt->sequence < seqNumCounter){
				window[(pkt->sequence - 1) % 5] = emptyPKt;
				continue;
			}

			seqNumCounter++;
			i++;
		}

		//SENDING ACKNOWLEDGEMENTS		

		int notAcked = 0;

		for (size_t j = 0; j < 5; j++)
		{
			char buff[10];

			if((window[j].sequence == -1)){
				break;
			}

			//COUNT BECAUSE NOT RECIEVED

			if(window[j].sequence == 0){
				notAcked++;
			}

			//ADD TO BUFFER AND SEND
			sprintf(buff,"%d",window[j].sequence);
			sendto(sock ,buff , sizeof(buffer) ,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
			bzero(buff,sizeof(buff));
		}

		//RECIEVEING MISSED SEGMENTS

		while (notAcked != 0){

			recvfrom(sock, pkt, sizeof(*pkt),0,(struct sockaddr *)&serv_addr,&leng);
			//IF TERMINATION PACKET BREAK
			if(window[(pkt->sequence - 1)%5].sequence == -1 ){
				break;
			}
			//PLACE PACKET INTO WINDOW ARRAY
			if (((pkt->sequence - 1) % 5) >= 0)
			{
				window[(pkt->sequence - 1) % 5] = *pkt;

				notAcked--;
			}
		}

		//WRITING TO DESTINATION FILE

		for (size_t j = 0; j < 5; j++)
		{
			//ADD TO SIZE
			totalSize += window[j].dataSize;

			//IF TERMINATION PACKET BREAK
			if(window[j].sequence == -1){
				break;
			}
			//IF EMPTY PACKET CONTINUE TO NEXT
			else if(window[j].sequence == 0){
				continue;
			}
			//WRITE SEGMENT TO FILE
			printf("\nSeqNum:%d\nData Size:%d\n",window[j].sequence,window[j].dataSize);
			fwrite(window[j].payload,1,window[j].dataSize,fp);
		}
	}
	printf("\nTotalSize:%d\n",totalSize);
    fclose(fp);
	return 0; 
} 
