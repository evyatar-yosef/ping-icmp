#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>

int main()
{   
    printf("im the Watchdog");
    int server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    char buffer[1];
    struct timeval currentFirstTime ,lastFirstTime;
    
    if (server_sock < 0)
    {
        printf("Could not create socket\n");
        exit(1);
    }
        printf("Created socket succesfully\n");
    

    struct sockaddr_in serverAddress; // new struct decleration of sockaddr_in type
    struct sockaddr_in * pserver_addr = &serverAddress; // Pointer Decleration
    memset(&serverAddress,0, sizeof(serverAddress)); // zerod the last 8 bits so it will match the suckaddr struct

    serverAddress.sin_family = AF_INET; // value = AF_INET. match to the sa_family of suckaddr struct
    serverAddress.sin_port = htons(3001); // switch from host byte order to network(Big endian) byte order.
    int convert = inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	if (convert <= 0)
	{
		printf("inet_pton() failed\n");
		exit(1);
	}

    int bindNum = bind(server_sock, (struct sockaddr *)&serverAddress , sizeof(serverAddress));
    if (bindNum == -1)
    {
        printf("Could not bind\n");
        perror("Error binding socket");
        exit(1);
    }
    else if (bindNum == 0)
    {
        printf("Binding succesfully to port number: %d\n", 3000);
    }

    if (listen(server_sock, 400) == -1)
	{
	printf("listen() failed with error code\n");
    exit(1);
    }
    printf("Listening...\n");

    
    //### struct for the client address, size and the accept function which makes the first connection with the sender ###//

    struct sockaddr_in clientAddress;  
    socklen_t clientAddressLen = sizeof(clientAddress);

    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(server_sock, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
    	{
           printf("listen failed with error code\n");
           return -1;
    	}
    	    printf("A new client connection accepted\n");

    int i = 0;
    int recieve2;
    int stay = 1;
    recieve2 = 0;
    int stayHere = 1;
    
    // while(i != 10 && stayHere == 1)
    // {
    //     i++;
    //     sleep(1); 
    //     recieve2 = recv(clientSocket, buffer, sizeof(buffer), MSG_DONTWAIT);
    //     if(recieve2 > 0 )
    //     {
    //         stay = 0;
    //     }
    //     printf("is:%d", recieve2);
    //     if (i == 9)
    //     {
    //         "break";
    //         stay = 0;
    //         kill(0, SIGKILL);

    //     }
    // }

    while(stayHere == 1)
    {
        printf("aa!!!!!!!!");
        fflush(stdout);
        recieve2 = recv(clientSocket, buffer, sizeof(buffer), MSG_DONTWAIT);
        printf("aa");
        fflush(stdout);
        printf("%d", recieve2);
        fflush(stdout);
        if (recieve2 <= 0)
        {
            i++;
            sleep(1);
            if( i == 9)
            printf("break");
            kill(0, SIGKILL);

        }
    }
        close(server_sock);
        close(clientSocket);
}