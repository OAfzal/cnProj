// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <stdlib.h>

#define PORT 5000 
#define filename "vid.mp4"
#define IP_ADDR "10.7.27.107"

int main(int argc, char const *argv[]) 
{ 
    FILE *fp;
    fp = fopen(filename,"wb");
    if (fp == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

	int sock = 0, bytes_read;
	double size_rec = 0;
	struct sockaddr_in serv_addr; 
	char *hello = "Hello from client"; 
	char buffer[1024] = {0}; 
	
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
	int leng;
	sendto(sock , hello , strlen(hello) , 0,(struct sockaddr *)&serv_addr,sizeof(serv_addr)); 	
	// valread = read( sock , buffer, 1024); 

	double buff_size[1] = {0};
	bytes_read =  recvfrom(sock,buff_size,sizeof(buff_size),0,(struct sockaddr *)&serv_addr,&leng);
	printf("%f\n",buff_size[0]);

	// // will remain open until the server terminates the connection
	while ((bytes_read = recvfrom(sock, &buffer, sizeof(buffer), MSG_WAITALL,(struct sockaddr *)&serv_addr,&leng)) > 0) {
		
		size_rec += bytes_read;
		if(!strcmp(buffer,"-1")){
			printf("\nTransfer Complete");
			break;
		}
		printf("\rProgress: %f",(size_rec/buff_size[0])*100.0);
        fwrite(buffer,bytes_read,1,fp);
        bzero(buffer,sizeof(buffer));
		buffer[sizeof(buffer)] = '\0';
	}
	printf("%f",size_rec);
    fclose(fp);
	return 0; 
} 