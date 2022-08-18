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

int internet_socket; //Made global so both of my threads can see it.

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );

//thread used to send messages.
void* sendThread() {
	while (1)
	{
   		int lenghtOfContentPacketToSend;
		char contentPacketToSend[180]; 
   	
		gets(contentPacketToSend);

		//Adds /0 terminator to the end of the stings and gets the bytes for sendto function
		lenghtOfContentPacketToSend = strlen(contentPacketToSend);
		contentPacketToSend[lenghtOfContentPacketToSend] = '\r';


		int number_of_bytes_send = 0;
		number_of_bytes_send = send( internet_socket, contentPacketToSend, lenghtOfContentPacketToSend, 0 );
		if( number_of_bytes_send == -1 )
		{
			perror( "send" );
		}
	}
}

//thread used to receive messages.
void* receiveThread() {
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

	printf("========================================================================================\n\n");
			printf("  /$$$$$$  /$$ /$$                       /$$                         \n");
			printf(" /$$__  $$| $$|__/                      | $$                         \n");
			printf("| $$  \\__/| $$ /$$  /$$$$$$  /$$$$$$$  /$$$$$$                       \n");
			printf("| $$      | $$| $$ /$$__  $$| $$__  $$|_  $$_/                       \n");
			printf("| $$      | $$| $$| $$$$$$$$| $$  \\ $$  | $$                         \n");
			printf("| $$    $$| $$| $$| $$_____/| $$  | $$  | $$ /$$                     \n");
			printf("|  $$$$$$/| $$| $$|  $$$$$$$| $$  | $$  |  $$$$/                     \n");
			printf(" \\______/ |__/|__/ \\_______/|__/  |__/   \\___/                       \n");
			printf("\n");
			printf("\n");
			printf("\n");
			printf("  /$$$$$$   /$$                           /$$                     /$$\n");
			printf(" /$$__  $$ | $$                          | $$                    | $$\n");
			printf("| $$  \\__//$$$$$$    /$$$$$$   /$$$$$$  /$$$$$$    /$$$$$$   /$$$$$$$\n");
			printf("|  $$$$$$|_  $$_/   |____  $$ /$$__  $$|_  $$_/   /$$__  $$ /$$__  $$\n");
			printf(" \\____  $$ | $$      /$$$$$$$| $$  \\__/  | $$    | $$$$$$$$| $$  | $$\n");
			printf(" /$$  \\ $$ | $$ /$$ /$$__  $$| $$        | $$ /$$| $$_____/| $$  | $$\n");
			printf("|  $$$$$$/ |  $$$$/|  $$$$$$$| $$        |  $$$$/|  $$$$$$$|  $$$$$$$\n");
			printf(" \\______/   \\___/   \\_______/|__/         \\___/   \\_______/ \\_______/\n\n");
	printf("========================================================================================\n\n\n\n");
	printf("You can now type and receive messages. Have fun!\n");
	
	//Create my threads and run those suckers.
	pthread_t p1, p2;
    if (pthread_create(&p1, NULL, &sendThread, NULL) != 0) {
        return 1;
    }
    if (pthread_create(&p2, NULL, &receiveThread, NULL) != 0) {
        return 2;
    }

    if (pthread_join(p1, NULL) != 0) {
        return 3;
    }
    if (pthread_join(p2, NULL) != 0) {
        return 4;
    }

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int initialization()
{
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "127.0.0.1", "24050", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
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


	printf("... waiting for a message from the server.");
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