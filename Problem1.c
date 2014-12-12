//
// smtpClient.c - example SMTP Client shell to send HELO to SMTP server
//
//   compile using: gcc -o smtpClient smtpClient.c
//   run using: ./smtpClient <mail host> [ port ]
//   example:   ./smtpClient mailhost.ksu.edu 25
//
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // for struct hostent

#define MAX_REQUEST 1024
#define MAX_REPLY 1024
#define MAX 256
int getLine(int fd, char line[], int max);

main(int argc, char **argv)
{
    int                 sockfd;
    struct sockaddr_in  serv_addr;
    char                s[MAX_REQUEST+1];
    char                reply[MAX_REPLY+1];
	char 				to[MAX];
	char				from[MAX];
	char				message[MAX];
    unsigned int	server_port = 25;
    struct hostent	*hostptr;
    struct in_addr      *ptr;
    unsigned short	port_number;
    char userinput[801]; //MAX_IN is initialized to 800

    // Read hostname and port from the command-line
    if (argc < 2)
    {
        printf("Usage: smtpClient <hostname> [port-number]\n");
        return(1);
    }
    if (argc > 2)
        port_number = atoi(argv[2]);
    else
        port_number = 25;

    if ( (hostptr = (struct hostent *) gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname error for host");
	return(1);
    }
    ptr = (struct in_addr *) *(hostptr->h_addr_list);
    printf ("DEBUG: server address: %u %s\n",ptr->s_addr,inet_ntoa(*ptr));
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = ptr->s_addr;
    serv_addr.sin_port        = htons(port_number);

    // Create communication endpoint.
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: can't open stream socket");
        return(1);
    }

    // Connect to the server.
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("client: can't connect to server");
        return(1);
    }
  /* Get To, From, and Message from the user */
    printf("TO: ");
    scanf("%s[255]",to);
    printf("FROM: ");
    scanf("%s[255]",from);
    printf("MESSAGE: ");
    scanf("%s[255]",message);
    
    /* Read the server's reply. */  
   getLine(sockfd, s, MAX);
   puts(s);
     
    /* Say HELO. */
    sprintf(s,"HELO %s \r\n",argv[1]);
    write (sockfd, s, strlen(s));
    
    /* Read the server's reply. */
      //printf("ADD_Hello!");
    getLine(sockfd, s, MAX);
    puts(s);
    
    /* Mail from: */
    memset(s,0,MAX);
    sprintf(s,"MAIL from: %s \r\n",from);
    write(sockfd, s, strlen(s));
    
    /* Read the server's reply. */
   // printf("MAIL_from!");
    getLine(sockfd, s, MAX);
    puts(s);
    
    /* RCPT To: */
    memset(s,0,MAX);
    sprintf(s,"RCPT To: %s \r\n",to);
    write (sockfd, s, strlen(s));
    
    /* Read the server's reply. */
       getLine(sockfd, s, MAX);
       puts(s);

    /* DATA */
    memset(s,0,MAX);
    sprintf(s,"DATA\r\n");
    write (sockfd, s, strlen(s));
    
    /* Read the server's reply. */
     
  getLine(sockfd, s, MAX);
   puts(s);
    
    /* OK to send */
    memset(s,0,MAX);
    sprintf(s,"%s \r\n",message);
    write (sockfd, s, strlen(s));

    /* Send terminator */
    memset(s,0,MAX);
    sprintf(s,"\r\n.\r\n");
    write (sockfd, s, strlen(s));
    
    /* Read the server's reply. */
  
    getLine(sockfd, s, MAX);
    puts(s);
    
    /* QUIT */
    memset(s,0,MAX);
    sprintf(s,"QUIT\r\n");
    write (sockfd, s, strlen(s));
    
    /* Read the server's reply. */
    
    getLine(sockfd, s, MAX);
    puts(s);
	
	
    printf("Message accepted for delivery\n");
    
	
	close(sockfd);

    return(0);
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
