#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zmq.h"
#include "zmqserver.h"
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <assert.h>

#define SERVER "10.0.0.17"

/* This is ZeroMQ daemon for communication channel */
int main (int argc, char const *argv[]) {
	const char *DNS_ADDR = "tcp://10.0.0.17:35901";
	const char *REQUEST_STATEMENT = "address of server";
	const char *REQUEST_ECHO = "ECHO"; //password to reqeust list of remote resources
	char buf[500];
    int port, rc, k;
  
	int s_port;
	s_port = 30332;

	// printf("Sending echo to %s\n", (char*)server_addr);
	struct sockaddr_in si_server;
	int sockfd;

    if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	printf("Error opening the socket\n");

    bzero((char*) &si_server, sizeof(si_server));

    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(s_port);
    si_server.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &si_server, sizeof(si_server)) < 0)
		printf("ERROR on binding\n");

	// listen(sockfd,5);
	
	if(sendto(sockfd, REQUEST_ECHO, strlen(REQUEST_ECHO), 
		0, (struct sockaddr *)&si_server, sizeof(si_server)) <0)
		printf("ERROR on sendto");
	while(1){
		// if (recvfrom(sockfd, buf, 500, 0, NULL,NULL) < 0)
			// printf("ERROR on recvfrom");
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
    strcpy(ip, "10.0.0.8");
    return ; // just for demo purpose
}
