#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>


struct sockaddr_in c_addr;
char fname[100];
char protocol[10];
char sr[10];
char ip[50];
int port;

void gotoxy(int x,int y)
 {
 printf("%c[%d;%df",0x1B,y,x);
 }

void* SendFileToClient(int *arg)
{
      int connfd=(int)*arg;
      printf("Connection accepted and id: %d\n",connfd);
      printf("Connected to Client: %s:%d\n",inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
       write(connfd, fname,256);

        FILE *fp = fopen(fname,"rb");
        if(fp==NULL)
        {
            printf("File opern error");
            return;   
        }   

        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);
            //printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
            if(nread > 0)
            {
                //printf("Sending \n");
                write(connfd, buff, nread);
            }
            if (nread < 1024)
            {
                if (feof(fp))
		{
                    printf("End of file\n");
		    printf("File transfer completed for id: %d\n",connfd);
		}
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }
printf("Closing Connection for id: %d\n",connfd);
close(connfd);
shutdown(connfd,SHUT_WR);
sleep(2);
}

void tcp_send() 
{
	int connfd = 0,err;
    pthread_t tid; 
    struct sockaddr_in serv_addr;
    int listenfd = 0,ret;
    char sendBuff[1025];
    int numrv;
    size_t clen=0;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd<0)
	{
	  printf("Error in socket creation\n");
	  exit(2);
	}

    printf("Socket retrieve success\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    ret=bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(ret<0)
    {
      printf("Error in bind\n");
      exit(2);
    }

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return;
    }

    while(1)
    {
        clen=sizeof(c_addr);
        printf("Waiting...\n");
        connfd = accept(listenfd, (struct sockaddr*)&c_addr,&clen);
        if(connfd<0)
        {
	  printf("Error in accept\n");
	  continue;	
	}
        err = pthread_create(&tid, NULL, &SendFileToClient, &connfd);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
   }
    close(connfd);
    return;
}

void tcp_receive()
{
	system("clear");
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    /* Create a socket first */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); // port

    serv_addr.sin_addr.s_addr = inet_addr(ip);

    /* Attempt a connection */
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return;
    }

    printf("Connected to ip: %s : %d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));

   	 /* Create file where data will be stored */
    	FILE *fp;
	char fname[100];
	read(sockfd, fname, 256);
	//strcat(fname,"AK");
	printf("File Name: %s\n",fname);
	printf("Receiving file...");
   	 fp = fopen(fname, "ab"); 
    	if(NULL == fp)
    	{
       	 printf("Error opening file");
         return;
    	}
    long double sz=1;
    /* Receive data in chunks of 256 bytes */
    while((bytesReceived = read(sockfd, recvBuff, 1024)) > 0)
    { 
        sz++;
        gotoxy(0,4);
        printf("Received: %llf Mb",(sz/1024));
	fflush(stdout);
        // recvBuff[n] = 0;
        fwrite(recvBuff, 1,bytesReceived,fp);
        // printf("%s \n", recvBuff);
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
    printf("\nFile OK....Completed\n");
    return;
}

void udp_send()
{
    int sockfd, n, fd;
    struct sockaddr_in servaddr;
    char buf[1024];

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    sendto(sockfd, ip, strlen(ip), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, NULL);
    if (!strncmp(buf, "ok", 2)) {
        printf("Filename sent.\n");
    }
    
    fd = open(fname, O_RDONLY);
    while ((n = read(fd, buf, MAXLINE)) > 0) {
        sendto(sockfd, buf, n, 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
    }
    sendto(sockfd, "END", strlen("END"), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
}

void udp_receive()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    run(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

}

void run(int sockfd, struct sockaddr *cliaddr, socklen_t clilen)
{
    int n, fd;
    socklen_t len;
    char buf[1024];

    len = clilen;
    n = recvfrom(sockfd, buf, MAXLINE, 0, cliaddr, &len);
    buf[n] = 0;
    printf("Received from client: [%s]\n", buf);
    sendto(sockfd, "ok", strlen("ok"), 0, cliaddr, len);
    fd = open(buf, O_RDWR | O_CREAT, 0666);

    while ((n = recvfrom(sockfd, buf, MAXLINE, 0, cliaddr, &len))) {
        buf[n] = 0;
        printf("%s", buf);
        if (!(strcmp(buf, END_FLAG))) {
            break;
        }
        write(fd, buf, n);
    }
    close(fd);
}

int main(int argc, char *argv[])
{
	
	strcpy(protocol, argv[1]);
	strcpy(sr, argv[2]);
	strcpy(ip, argv[3]);
    port = atoi(argv[4]);
	
	if(strcmp("tcp", protocol) == 0)
	{
		if(strcmp("send", sr) == 0)
        {
            strcpy(fname, argv[5]);
			tcp_send();
        }
		else
			tcp_receive();
	}
/*	else 
	{
		if(strcmp("send", sr) == 0)
			udp_send();
		else
			udp_receive();
	}*/
		
    return 0;
}
