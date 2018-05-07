/* Send Multicast Datagram code example. */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char ip[50];
char fname[100];
char databuf[1024] = "Multicast test message lol!";
char data[1024];
int datalen = sizeof(databuf);
int fp;
int n;



int main (int argc, char *argv[ ])
{
/* Create a datagram socket on which to send. */
	strcpy(ip, argv[1]);
	strcpy(fname, argv[2]);
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  printf("Opening the datagram socket...OK.\n");
	 
	/* Initialize the group sockaddr structure with a */
	/* group address of 225.1.1.1 and port 5555. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(5555);
	 
	 
	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr(ip);
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
	  perror("Setting local interface error");
	  exit(1);
	}
	else
	  printf("Setting the local interface...OK\n");

	/* Send a message to the multicast group specified by the*/
	/* groupSock sockaddr structure. */
	/*int datalen = 1024;*/
/*	if(sendto(sd, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
	{
		perror("Sending datagram message error");
	}
	else
	{
	  printf("Sending datagram message...OK\n");
	}*/
	
	sendto(sd, "1.jpg", 1024, 0, (struct sockaddr *) &groupSock, sizeof(groupSock));
	n = recvfrom(sd, data, 1024, 0, NULL, NULL);
	/*if (!strncmp(buf, "ok", 2)) {
		printf("Filename sent.\n");
	}*/

	fp = open(fname, O_RDONLY);
	while ((n = read(fp, data, 1024)) > 0) {
		sendto(sd, data, n, 0, (struct sockaddr *) &groupSock, sizeof(groupSock));
	}
	sendto(sd, "=======END", strlen("=======END"), 0, (struct sockaddr *) &groupSock, sizeof(groupSock));
	
	 
	return 0;
}
