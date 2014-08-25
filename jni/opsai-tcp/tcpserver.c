#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <assert.h>
#include "tcpserver.h"

#define SERVER "10.0.0.17"
#define RCVBUFSIZE 500   /* Size of receive buffer */

/* This is TCP server daemon for communication channel */
int main (int argc, char const *argv[]) {
    printf("running\n"); /* communicate with commandLineUtil in Java */
    char ip[16];
    getIP(ip); // get its own IP addr

    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */

    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() 
                                        and total bytes read */
    servIP = "10.0.0.17";             /* First arg: server IP address (dotted quad) */
    echoString = "LIST";         /* Request for list */
    echoServPort = 35901;  /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    int request = -1;
    if (argc > 1)    /* Test for correct number of arguments */
        request = atoi(argv[1]);
    totalBytesRcvd = 0;

    if (request == -1) {
        echoStringLen = strlen(echoString);          /* Determine input length */

        /* Send the string to the server */
        if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
            DieWithError("send() sent a different number of bytes than expected");

        /* Receive the same string back from the server */
        printf("NUM_RESOURCE=");                /* Setup to print the echoed string */
        while (totalBytesRcvd < echoStringLen)
        {
            /* Receive up to the buffer size (minus 1 to leave space for
               a null terminator) bytes from the sender */
            if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed or connection closed prematurely");
            totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
            echoBuffer[bytesRcvd] = '\0';  /* Terminate the string! */
            printf("%s", echoBuffer);      /* Print the echo buffer */

        }


        printf("\n");    /* Print a final linefeed */

        close(sock);
        exit(0);
        return 0;
    } else  {
        printf("sending request\n");
        char creq[2];
        sprintf(creq, "%d", request);
        echoStringLen = strlen(creq);
        send(sock, creq, strlen(creq), 0);
        printf("Sent\n");
    }
    return 0;
}

void getIP(char *ip) {
	struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) 
        perror("getifaddrs");

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;  

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
        				host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if((strcmp(ifa->ifa_name,"wlan0")==0)&&(ifa->ifa_addr->sa_family==AF_INET)){
            if (s != 0)
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
            // printf("\t  Address : <%s>\n", host); 
            freeifaddrs(ifaddr);
            strcpy(ip, host);
            return;
        }
    }

    freeifaddrs(ifaddr);
    strcpy(ip, "10.0.0.2");
    return ; // just for demo purpose and be safe :) 
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

int HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize; /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
    DieWithError("recv() failed");

    if (strcmp(echoBuffer, "ECHO"))
    {
        printf("%s\n", "Authentication successful");
        sprintf(echoBuffer, "ACK");
        printf("Sending %s\n", echoBuffer);
    }

    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) /* zero indicates end of transmission */
    {
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
        DieWithError("send() failed");

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
          DieWithError("recv() failed");
        } else {

        }
    }
    // close(clntSocket); /* Close client socket */
    return 1;
}