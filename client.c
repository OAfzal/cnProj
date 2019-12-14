#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>

#define PORT 5000 
#define filename "vid.mp4"
#define IP_ADDR "40.121.137.163"
#define MAX_LEN 500

struct CustomSegment {

    char payload[MAX_LEN + 1];
	int dataSize;
    int sequence;
};

void customSort(struct CustomSegment list[5])
{
    int i, j;
    struct CustomSegment temp;
    
    for (i = 0; i < 5 - 1; i++)
    {
        for (j = 0; j < (5 - 1-i); j++)
        {
            if (list[j].sequence > list[j + 1].sequence)
            {
                temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            } 
        }
    }
}


int main(int argc, char const *argv[]) 
{ 

	int seqNumCounter = 1;
	int sock = 0, bytes_read,leng;
	double size_rec = 0;

	struct sockaddr_in serv_addr; 

	char *hello = "Hello from client"; 
	char buffer[500] = {0}; 

	struct CustomSegment* pkt = malloc(sizeof(struct CustomSegment));
	struct CustomSegment window[5];
	// for (size_t i = 0; i < 5; i++)
	// {
	// 	window[i] = malloc(sizeof(struct CustomSegment));
	// }
	
	struct CustomSegment emptyPKt = {"",0};

    FILE *fp,*fp2;
    fp2 = fopen("dataC.txt","w");
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
	int flagMissing = 0;
	int totalSize = 0;
	int packetsRecieved = 0;

	while (1) {
		
		if(finished){
			break;
		}

		bzero(buffer,sizeof(buffer));
		bzero(pkt,sizeof(struct CustomSegment));

		//Emptying the array of previous remains
		for (size_t i = 0; i < 5; i++)
		{
			window[i] = emptyPKt;
		}
		
		size_t i = 0;
		while(i<5){

			bytes_read = recvfrom(sock, pkt, sizeof(*pkt),MSG_DONTWAIT,(struct sockaddr *)&serv_addr,&leng);
			char buffAll[50];

			if(pkt->sequence < seqNumCounter){
				window[(pkt->sequence - 1) % 5] = emptyPKt;
				continue;
			}

			totalSize += pkt->dataSize;

			window[(pkt->sequence - 1) % 5] = *pkt;

			if(!strcmp(window[i].payload,"-1")){
				finished =1;
				printf("\nTransfer Complete");	
				break;
			}

			// if(window[i].sequence > seqNumCounter){
			// 	for (size_t iter = seqNumCounter; iter <= window[i].sequence; iter++)
			// 	{
			// 		sprintf(buffAll,"\nSeqNum:%d\tExpected:%ld",window[i].sequence,iter);
			// 		fputs(buffAll,fp2);
			// 	}
			// 	seqNumCounter = window[i].sequence;
			// }

			seqNumCounter++;
			// window[i].sequence = pkt->sequence;
			// strcpy(window[i]->payload,pkt->payload);
			// fwrite(window[i]->payload,1,window[i]->dataSize,fp);
			// printf("\nSeqNum:%d\nData Size:%d\n",window[i].sequence,window[i].dataSize);
			i++;
		}
		
		int notAcked = 0;

		for (size_t j = 0; j < 5; j++)
		{
			char buff[10];

			if(!strcmp(window[j].payload,"-1")){
				break;
			}

			if(window[j].sequence == 0){
				notAcked++;
			}

			// printf("%d\n",window[j].sequence);
			sprintf(buff,"%d",window[j].sequence);
			sendto(sock ,buff , sizeof(buffer) ,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
			bzero(buff,sizeof(buff));
			// seqNumCounter++;
		}

		//Receiving Missed Packets

		int bytea = 0;
		
		// printf("\nnotAcked=%d",notAcked);

		while (notAcked != 0){

			bytea =recvfrom(sock, pkt, sizeof(*pkt),0,(struct sockaddr *)&serv_addr,&leng);
			window[(pkt->sequence - 1) % 5] = *pkt;


			if(!strcmp(window[(pkt->sequence - 1)%5].payload,"-1")){
				finished =1;
				printf("\nTransfer Complete");	
				break;
			}

			totalSize += pkt->dataSize;

			// printf("\nRE:SeqNum:%d\nData Size:%d\nSeqNumCounter:%d\n",window[(pkt->sequence - 1) %5].sequence,window[(pkt->sequence - 1) %5].dataSize,seqNumCounter);

			notAcked--;
		}

		printf("\nSTART:%d",seqNumCounter);
		for (size_t j = 0; j < 5; j++)
		{
			if(!strcmp(window[j].payload,"-1")){
				continue;
			}
			else if(window[j].sequence == 0){
				continue;
			}

			printf("\nSeqNum:%d\nData Size:%d\n",window[j].sequence,window[j].dataSize);
			// printf("Seq%d (%ld)\t",window[i].sequence,strlen(window[i].payload));			
			fwrite(window[j].payload,1,window[j].dataSize,fp);
		}
		

	}
	printf("\nTotalSize:%d\n",totalSize);
	printf("\n%s",pkt->payload);
	// recvfrom(sock,pkt,sizeof(*pkt),0,(struct sockaddr*)&serv_addr,&leng);
	// printf("\nSeqNumCounter:%.2f\n",((double)seqNumCounter/(double)(atoi(pkt->payload)))*100.0);
	// printf("%f",size_rec);
    fclose(fp);
	fclose(fp2);
	return 0; 
} 
