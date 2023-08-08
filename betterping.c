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
#include<sys/wait.h>
#include <unistd.h>

#define ICMP_HDRLEN 8
#define IP4_HDRLEN 20
#define DESTINATION_IP "8.8.8.8"
#define SOURCE_IP "127.0.0.1" //127.0.0.1

// Checksum algo
unsigned short calculate_checksum(unsigned short *paddress, int len);
float time_diff(struct timeval *start, struct timeval *end);

int main(int argc, char *argv[])
{
    int timeCounter = 0;
    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;
    int status;
    int pid = fork();
    if (pid == 0)
    {
        printf("in child \n");
        execvp(args[0], args);
    }
    else{
        sleep(2);
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            printf("Could not create socket\n");       // Vailidation of the Socket creations..
        }
        else if (sockfd == 0)
        {
            printf("Created socket succesfully\n");
        }

        struct sockaddr_in serverAddress; // new struct decleration of sockaddr_in type
        struct sockaddr_in * pserver_addr = &serverAddress; // Pointer Decleration
        memset(&serverAddress,0, sizeof(serverAddress)); // zerod the last 8 bits so it will match the suckaddr struct

        serverAddress.sin_family = AF_INET; // value = AF_INET. match to the sa_family of suckaddr struct
        serverAddress.sin_port = htons(3001); // switch from host byte order to network(Big endian) byte order.
        int convert = inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr); // converting the IPv4 from text to binary
        if (convert <= 0)                                                       // Checking if the conversion worked
        {
            printf("inet_pton() failed\n"); 
            return -1;
        }
        
        //### CONNECTING TO THE SERVER ###///
        
        int sockcon = connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)); 

        if (sockcon == -1)                                                                      // Vailidation of the connections between
        {                                                                                         //  theserver      
            printf("connect() failed with error code\n\n");
        }
        else if (sockcon == 0)
        {
            printf("Connected to server succesfully!\n\n");
        }

        struct icmp icmphdr; // ICMP-header
        char data[IP_MAXPACKET] = "This is the ping.\n";

        int datalen = strlen(data) + 1;

    //########## Setting the parametes of the icmp struct ##########//

        icmphdr.icmp_type = ICMP_ECHO;
        icmphdr.icmp_code = 0;
        icmphdr.icmp_id = 18;
        icmphdr.icmp_seq = 0;
        icmphdr.icmp_cksum = 0;

        char packet[IP_MAXPACKET];

        // Next, ICMP header
        memcpy((packet), &icmphdr, ICMP_HDRLEN);

        // After ICMP header, add the ICMP data.
        memcpy(packet + ICMP_HDRLEN, data, datalen);

            // Calculate the ICMP header checksume
        icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
        memcpy((packet), &icmphdr, ICMP_HDRLEN);

        struct sockaddr_in dest_in;
        memset(&dest_in, 0, sizeof(struct sockaddr_in));
        dest_in.sin_family = AF_INET;

        // The port is irrelant for Networking and therefore was zeroed.
        // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
        dest_in.sin_addr.s_addr = inet_addr(DESTINATION_IP);
        inet_pton(AF_INET, DESTINATION_IP, &(dest_in.sin_addr.s_addr));

        // Create raw socket for IP-RAW (make IP-header by yourself)
        int sockPing = -1;
        if ((sockPing = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
        {
            fprintf(stderr, "socket() failed with error: %d", errno);
            fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
            return -1;
        }
        int counter = 0;
        struct timeval start, end;
        
        while (1)
        {
            icmphdr.icmp_type = ICMP_ECHO;
            icmphdr.icmp_code = 0;
            icmphdr.icmp_id = 18;
            // icmphdr.icmp_seq = 0;
            icmphdr.icmp_cksum = 0;
                // Next, ICMP header
            memcpy((packet), &icmphdr, ICMP_HDRLEN);

            // After ICMP header, add the ICMP data.
            memcpy(packet + ICMP_HDRLEN, data, datalen);

                // Calculate the ICMP header checksume
            icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
            memcpy((packet), &icmphdr, ICMP_HDRLEN);

            struct sockaddr_in dest_in;
            memset(&dest_in, 0, sizeof(struct sockaddr_in));
            dest_in.sin_family = AF_INET;

            // The port is irrelant for Networking and therefore was zeroed.
            // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
            dest_in.sin_addr.s_addr = inet_addr(DESTINATION_IP);
            inet_pton(AF_INET, DESTINATION_IP, &(dest_in.sin_addr.s_addr));
            gettimeofday(&start, 0);
            timeCounter++;
            // Send the packet using sendto() for sending datagrams.
            int bytes_sent = sendto(sockPing, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
                if (bytes_sent == -1)
                {
                    fprintf(stderr, "sendto() failed with error: %d", errno);
                    return -1;
                }
            // if(timeCounter == 10)
            // {
            //     sleep(8);
            // }

            // if(timeCounter == 250)
            // {
            //     sleep(11);
            // }
            // printf("Successfuly sent one packet : ICMP HEADER : %d bytes, data length : %d , icmp header : %d \n", bytes_sent, datalen, ICMP_HDRLEN);
            // Get the ping response
        bzero(packet, IP_MAXPACKET);
        socklen_t len = sizeof(dest_in);
        ssize_t bytes_received = -1;
        while ((bytes_received = recvfrom(sockPing, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
            {
                if (bytes_received > 0)
                {
                    send(sockfd, "0", 1, 0);
                    printf("send");
                    // Check the IP header
                    struct iphdr *iphdr = (struct iphdr *)packet;
                    struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
                    // printf("%ld bytes from %s\n", bytes_received, inet_ntoa(dest_in.sin_addr));
                    // icmphdr->type

                    // printf("Successfuly received one packet with %ld bytes : data length : %d , icmp header : %d , ip header : %d \n", bytes_received, datalen, ICMP_HDRLEN, IP4_HDRLEN);
                    
                    break;
                }
            }
            int x = IP_RECVTTL;
            gettimeofday(&end, 0);
            char reply[IP_MAXPACKET];
            memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, datalen);
            printf("%ld bytes from %s: icmp_seq=%d time=%f\n", bytes_received, argv[1], counter, time_diff(&start, &end));
            counter++;
        }

            // Close the raw socket descriptor.
            close(sockfd);
            close(sockPing);
    }

    wait(&status); // Waiting for the children
    printf("child exit status is: %d\n", status);
    return 0;
}
    // Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}

float time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}
