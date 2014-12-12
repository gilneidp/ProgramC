//
// SNMP Client
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // for struct hostent

#define MAX 100
#define MAX_REQUEST 1024
#define MAX_REPLY 1024
int getLine(int fd, char line[], int max);

main(int argc, char **argv)
{
    int                 sockfd, sockfd2;
    struct sockaddr_in  cli_addr, serv_addr, serv_addr2;
    unsigned char       s[MAX];      /* array to hold output */
    int                 response;    /* user response        */
    int                 nread;       /* number of characters */ 
    int                 servlen;     /* length of server addr*/
    int			i,j;
    unsigned long long	counterSENT, counterRCVD, counterFINAL;
    char                request[MAX_REQUEST+1];
    char                reply[MAX_REPLY+1];
    unsigned int	server_port = 25;
    struct hostent	*hostptr;
    struct in_addr      *ptr;
    unsigned short	port_number;
    char userinput[801]; //MAX_IN is initialized to 800
    char from[MAX];
    char to[MAX];
    char data[MAX];

    /* Set up the address of the server to be contacted. */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    if (argc==1)
      serv_addr.sin_addr.s_addr = inet_addr("129.130.10.48");
    else
      serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port        = htons(161);

    /* Set up the address of the client. */
    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family      = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(0);
    cli_addr.sin_port        = htons(0);

    /* Create a socket (an endpoint for communication). */
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client: can't open datagram socket");
        exit(1);
    }

    /* Bind the client's socket to the client's address */
    if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("client: can't bind local address");
        exit(1);
    }

    counterSENT = counterRCVD = counterFINAL = 0;

    for (i=0; i<43; i++)
      scanf("%2X",&s[i]);
    servlen = sizeof(serv_addr);
    sendto (sockfd, (char *) s, 43, 0,
                 (struct sockaddr *) &serv_addr, servlen);
    printf("SENT:\n       ");
    for (i=0; i<43; i++)
    {
      printf("%2X ",s[i]);
      if ((i+1)%16 == 0)
        printf("\n       ");
      counterSENT *= 256;
      counterSENT += s[i];
    }

    printf("\nCounter SENT = %u \n", counterSENT);
    printf("\n\nWAITING FOR REPLY...\n\n");
    
    nread = recvfrom(sockfd, s, MAX, 0,
             (struct sockaddr *) &serv_addr, &servlen);
    if (nread > 0) {
      printf("RCVD:\n       ");
      for (i=0; i<nread; i++)
      {
        printf("%2X ",s[i]);
        if ((i+1)%16 == 0)
          printf("\n       ");
      }
    } else {
	printf("      Nothing read. \n");
    }

//  if (s[40]==0x43)
//    j = 42;
//  else
//    j = 43;
    j = nread-5; // the last four bytes are the counter

    for (i=j; i<nread; i++)
    {
      counterRCVD *= 256;
      counterRCVD += s[i];
    }
    printf("\nCounter RCVD = %u \n", counterRCVD);

	counterFINAL = (counterRCVD-counterSENT)/30;

    printf("\nCounter = %u \n", counterFINAL);
	
	if(counterFINAL > 100){
	        port_number = 25;
			char arg[MAX];
			strcpy(arg, "smtp.cis.ksu.edu");
	    	if ( (hostptr = (struct hostent *) gethostbyname(arg)) == NULL) {
    	    	perror("gethostbyname error for host");
				return(1);
	    	}
	    	ptr = (struct in_addr *) *(hostptr->h_addr_list);
		    printf ("DEBUG: server address: %u %s\n",ptr->s_addr,inet_ntoa(*ptr));
	    	memset((char *) &serv_addr2, 0, sizeof(serv_addr2));
	    	serv_addr2.sin_family      = AF_INET;
		    serv_addr2.sin_addr.s_addr = ptr->s_addr;
		    serv_addr2.sin_port        = htons(port_number);

	    	// Create communication endpoint.
	    	if ( (sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    		    perror("server: can't open stream socket");
	        	return(1);
    		}

	    	// Connect to the server.
    		if (connect(sockfd2, (struct sockaddr *) &serv_addr2, sizeof(serv_addr2)) < 0) {
        		perror("client: can't connect to server");
		        return(1);
	    	}
    		getLine(sockfd2, reply, MAX_REPLY);
		    puts(reply);
	    	printf("SENDING HELO %s\n", arg);
		    sprintf(request,"HELO %s\r\n", arg);
		    write(sockfd2, request, strlen(request));
	    	getLine(sockfd2, reply, MAX_REPLY);
    		puts(reply);
    		strcpy(from, "gilvanveras@cis.ksu.edu");
	    	sprintf(request,"MAIL FROM:<%s>\r\n", from);
		    write(sockfd2, request, strlen(request));
		    getLine(sockfd2, reply, MAX_REPLY);
    		puts(reply);
    		strcpy(to, "gilvanveras@cis.ksu.edu");
	    	sprintf(request,"RCPT TO:<%s>\r\n", to);
	    	write(sockfd2, request, strlen(request));
    		getLine(sockfd2, reply, MAX_REPLY);
		    puts(reply);
    		write(sockfd2, "DATA", strlen(request));
		    getLine(sockfd2, reply, MAX_REPLY);
	    	puts(reply);
			strcpy(data, "Rate exceeds limit!");
		    sprintf(request,"%s\r\n.\r\n", data);
	    	write(sockfd2, request, strlen(request));
		    getLine(sockfd2, reply, MAX_REPLY);
	    	puts(reply);
	    	close(sockfd2);
		}
		
    exit(0);
}

//
// get a line of data from fd
//
int getLine(int fd, char* line, int lim)
{
    int i;
    char c;

    i =  0;
    while (--lim > 0 && read(fd,&c,1)>0 && c!='\n' && c!='\0')
    {
        line[i++] = c;
    }
    if (c=='\n')
        line[i++] = c;
    line[i] = '\0';
    return i;
}
