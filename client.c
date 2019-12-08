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
#define IP_ADDR "10.7.27.171"
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

	while (1) {
		
		if(finished){
			break;
		}

		bzero(buffer,sizeof(buffer));
		bzero(pkt,sizeof(struct CustomSegment));
		
		for(size_t i = 0 ;i < 5; i++){

			bytes_read = recvfrom(sock, pkt, sizeof(*pkt),0,(struct sockaddr *)&serv_addr,&leng);
			totalSize += pkt->dataSize;

			// if(bytes_read == -1){	
			// 	window[i] = emptyPKt;
			// }
			if(!strcmp(pkt->payload,"-1")){
				finished =1;
				printf("\nTransfer Complete");
				break;
			}
			
			window[i].sequence = pkt->sequence;
			sprintf(window[i].payload,"%s",pkt->payload);
			fwrite(pkt->payload,1,pkt->dataSize,fp);
			printf("\nSeqNum:%d\nData Size:%d\n",pkt->sequence,pkt->dataSize);
		}

		printf("\n");

		int acked = 0;

		// while (acked != 1){

		// 	for (size_t i = 0; i < 5; i++)
		// 	{
		// 		char buff[10];
		// 		if(!strcmp(pkt->payload,"-1")){
		// 			break;
		// 		}
		// 		if(window[i].sequence != seqNumCounter){

		// 			flagMissing++;
		// 			sprintf(buff,"%d\n",seqNumCounter);
		// 			fputs(buff,fp2);
		// 			continue;
		// 		}
		// 		// sendto(sock ,buffer , sizeof(buffer) ,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
		// 		seqNumCounter++;
		// 	}
		// 	acked = 1;
		// }


		// while(flagMissing){
			
		// 	bytes_read = recvfrom(sock, pkt, sizeof(*pkt), 0,(struct sockaddr *)&serv_addr,&leng);
		// 	printf("%d\n",pkt->sequence);
		// 	for (size_t i = 0; i < 5; i++)
		// 	{
		// 		if(window[i].sequence == 0){
		// 			window[i].sequence = pkt->sequence;
		// 			sprintf(window[i].payload,"%s",pkt->payload);
		// 		}
		// 	}
		// 	flagMissing--;			
		// }
	

		// customSort(window);

		// for (size_t i = 0; i < 5; i++)
		// {
		// 	if(!strcmp(pkt->payload,"-1")){
		// 			continue;
		// 	}
		// 	// printf("%d (%ld)\t",window[i].sequence,strlen(window[i].payload));			
		// 	fwrite(window[i].payload,1,window[i].dataSize,fp);
		// }
		

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