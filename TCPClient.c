 // gcc TCPClient.c -l ws2_32 -o tcpC

 /*
    Bronnen: BJ guide, Bart Stukken (docent) en Axel Vanherle (studybuddy)
*/
 
 #ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <pthread.h>
	#include <string.h> //for memset
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

int internet_socket;

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );

void* sendItem()
{
	while (1)
	{
		int lengteItem;
		char inhoudItem[180];

		gets(inhoudItem);
		lengteItem = strlen(inhoudItem);
		inhoudItem[lengteItem] = '\r';

		int number_of_bytes_send = 0;
		number_of_bytes_send = send( internet_socket, inhoudItem, lengteItem, 0 );
		if( number_of_bytes_send == -1 )
		{
			perror( "send" );
		}
	}
	
}

void* receiveItem()
{

	while (1)
	{
		int number_of_bytes_received = 0;
		char buffer[10000];
		number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
		if( number_of_bytes_received == -1 )
		{
			perror( "recv" );
		}
		else
		{	
			buffer[number_of_bytes_received] = '\0';
			printf( "Received : %s\n", buffer );
		}
	}
	
}

int main( int argc, char * argv[] )
{

	OSInit();

	internet_socket = initialization();
	system("cls");
	printf("\n\n\n**************************************************\n");
	printf(" - - - - - - - - -  SPT launched  - - - - - - - -\n");
	printf("**************************************************\n\n\n");

	pthread_t id1, id2;
    	if (pthread_create(&id1, NULL, &sendItem, NULL) != 0) 
		{
    	    return 1;
    	}
    	if (pthread_create(&id2, NULL, &receiveItem, NULL) != 0) 
		{
        	return 2;
    	}

    	if (pthread_join(id1, NULL) != 0) 
		{
        	return 3;
    	}
    	if (pthread_join(id2, NULL) != 0) 
		{
        	return 4;
    	}

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "127.0.0.1", "9034", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

void execution( int internet_socket )
{
	printf("waiting for servermessage...");
}

void cleanup( int internet_socket )
{
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}