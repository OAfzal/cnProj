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


struct CustomSegment {

    char payload[500];
    int sequence;
};

int main(int argc, char const *argv[]) 
{ 

	int seqNumCounter = 1;
	int sock = 0, bytes_read,leng;
	double size_rec = 0;
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
	char buffer[500] = {0}; 
	struct CustomSegment* pkt = malloc(sizeof(struct CustomSegment));


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
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, IP_ADDR, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 	
		return -1; 
	}

	sendto(sock , hello , strlen(hello) , 0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	


	bytes_read = recvfrom(sock, &buffer, sizeof(buffer), 0,(struct sockaddr *)&serv_addr,&leng);

	printf("%s\n",buffer);

	// setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(int *)4,sizeof(int));

	// struct timeval timeout;      
    // timeout.tv_sec = 10;
    // timeout.tv_usec = 0;

    // if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
    //             sizeof(timeout)) < 0)
    //     error("setsockopt failed\n");

    // if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
    //             sizeof(timeout)) < 0)
    //     error("setsockopt failed\n");

	bzero(buffer,sizeof(buffer));

		
	while ((bytes_read = recvfrom(sock, pkt, sizeof(*pkt), 0,(struct sockaddr *)&serv_addr,&leng)) > 0) {

		char *missingSeqNum;
		// size_rec += sizeof(pkt->payload);
		printf("\nSeqNum:%d\nServer Seq Num:%d\nData ecieved:%ld\n",seqNumCounter,pkt->sequence,strlen(pkt->payload));
		// printf("%s\n----------------------",pkt->payload);
		if(!strcmp(pkt->payload,"-1")){
			printf("\nTransfer Complete");
			break;
		}

		sprintf(missingSeqNum,"%d\t%d\n",seqNumCounter,pkt->sequence);
		fputs(missingSeqNum,fp2);

		// if(seqNumCounter != pkt->sequence){
		// 	sprintf(missingSeqNum,"%d\t%d\n",seqNumCounter,pkt->sequence);
		// 	fputs(missingSeqNum,fp2);
		// 	bzero(missingSeqNum,sizeof(missingSeqNum));
		// }
		seqNumCounter++;
		// printf("Sequence: %d\n",pkt->sequence);
		sprintf(buffer,"%ld",sizeof(pkt->payload));
		// fputs(buffer,fp2);

		sendto(sock ,buffer , sizeof(buffer) ,0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
        fwrite(pkt->payload,sizeof(pkt->payload),1,fp);
        bzero(buffer,sizeof(buffer));
		bzero(pkt,sizeof(struct CustomSegment));
	}

	recvfrom(sock,pkt,sizeof(*pkt),0,(struct sockaddr*)&serv_addr,&leng);
	printf("\nSeqNumCounter:%.2f\n",((double)seqNumCounter/(double)(atoi(pkt->payload)))*100.0);
	// printf("%f",size_rec);
    fclose(fp);
	fclose(fp2);
	return 0; 
} 