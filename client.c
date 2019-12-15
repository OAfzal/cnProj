#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>

#define PORT 5000 
#define filename "vid3.mp4"
#define IP_ADDR "10.7.26.180"
#define MAX_LEN 500

struct CustomSegment {

    char payload[MAX_LEN + 1];
	int dataSize;
    int sequence;
};


int main(int argc, char const *argv[]) 
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
	serv_addr.sin_port = htons(PORT); 
	

	if(inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<=0) 
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

			if(pkt->sequence == -1){
				finished =1;
				printf("\nTransfer Complete");		
				printf("\nSeqNum:%d\tFinished:%d\n",pkt->sequence,finished);
				window[i] = *pkt;
				break;
			}

			window[(pkt->sequence - 1) % 5] = *pkt;

			if(pkt->sequence < seqNumCounter){
				window[(pkt->sequence - 1) % 5] = emptyPKt;
				continue;
			}

			seqNumCounter++;
			i++;


			// if (((pkt->sequence - 1) % 5) >= 0)
			// {

			// }
		}

		//SENDING ACKNOWLEDGEMENTS		


		
		int notAcked = 0;

		for (size_t j = 0; j < 5; j++)
		{
			char buff[10];

			if((window[j].sequence == -1)){
				break;
			}

			if(window[j].sequence == 0){
				notAcked++;
			}

			sprintf(buff,"%d",window[j].sequence);
			sendto(sock ,buff , sizeof(buffer) ,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
			bzero(buff,sizeof(buff));
		}

		// if(finished == 1){
		// 	notAcked = 0;
		// 	for (size_t m = 0; m < 5; m++)
		// 	{
		// 		printf("\nnotAcked:%d",notAcked);
		// 		printf("\nSeq:%d",window[m].sequence);
		// 		if(window[m].sequence != 0 && window[m].sequence != -1){
		// 			notAcked++;
		// 		}
		// 	}
		// }


		//RECIEVEING MISSED SEGMENTS

		while (notAcked != 0){

			recvfrom(sock, pkt, sizeof(*pkt),0,(struct sockaddr *)&serv_addr,&leng);

			if(window[(pkt->sequence - 1)%5].sequence == -1 ){
				break;
			}

			if (((pkt->sequence - 1) % 5) >= 0)
			{
				window[(pkt->sequence - 1) % 5] = *pkt;

				notAcked--;
			}
		}

		

		//WRITING TO DESTINATION FILE

		for (size_t j = 0; j < 5; j++)
		{
			totalSize += window[j].dataSize;

			if(window[j].sequence == -1){
				break;
			}
			else if(window[j].sequence == 0){
				continue;
			}

			printf("\nSeqNum:%d\nData Size:%d\n",window[j].sequence,window[j].dataSize);
			fwrite(window[j].payload,1,window[j].dataSize,fp);
		}

	}
	printf("\nTotalSize:%d\n",totalSize);
    fclose(fp);
	return 0; 
} 
