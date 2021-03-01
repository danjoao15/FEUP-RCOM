/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

int main(int argc, char** argv){

	int	sockfd;
	struct	sockaddr_in server_addr;
	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";  
	int	bytes;
	
//################################################################################################	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;                       /* address family: AF_INET */
	//$man 3 inet_addr
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);	/* internet address => 32   bit address Internet network byte ordered*/
	//$man 3 htons
	server_addr.sin_port = htons(SERVER_PORT);		/*port in network byte order =>server TCP port must be network byte ordered */
  
  //sin_addr  is  the  IP host address
  
  
  /*  FROM $ man 7 ip 
         struct sockaddr_in {
               sa_family_t    sin_family; // address family: AF_INET 
               in_port_t      sin_port;   // port in network byte order
               struct in_addr sin_addr;   // internet address 
           };

           // Internet address.
           struct in_addr {
               uint32_t       s_addr;     // address in network byte order
           };
 */   
    
//##############################################################################################    
    
    
	/*open an TCP socket*/
	// man 2 socket
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	/*connect to the server*/
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}
    	/*send a string to the server*/
	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);

	close(sockfd);
	exit(0);
}


