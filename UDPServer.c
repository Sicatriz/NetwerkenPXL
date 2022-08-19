// gcc UDPServer.c -l ws2_32 -o runS

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
	#include <string.h> //for memset
	#include <time.h> //for clock
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
	int OSInit( void ) {}
	int OSCleanup( void ) {}
#endif

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );
void intro (void);

int qPakket;

int main( int argc, char * argv[] )
{

	//////////////////
	//Initialization//
	//////////////////

	OSInit();
	intro();

	int internet_socket = initialization();

	/////////////
	//Execution//
	/////////////

	printf("Hoeveel pakketten wil je ontvangen?\n");
	scanf("%d", &qPakket);
	
	execution( internet_socket );

	////////////
	//Clean up//
	////////////

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
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24080", &internet_address_setup, &internet_address_result );
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
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				close( internet_socket );
				perror( "bind" );
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
	int timeout = 10000;

	printf("Hoeveel seconden moet de timeout bedragen?\n\n");
	scanf("%d", &timeout);
	timeout = timeout *1000;

	float percentPakket = 0;
    if (setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
        perror("Error");
    }
	FILE *output;
	output = fopen("output.csv", "w+");
	fprintf(output, "De gebruiker heeft gekozen om %d pakketten te ontvangen\n\n", qPakket);
	int totPakket;
	totPakket = 0;

	//Step 2.1
	int number_of_bytes_received = 0;
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;

	clock_t begin = clock();
	for (size_t i = 0; i < qPakket; i++)
	{
			number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
			if( number_of_bytes_received == -1 )
			{
				perror( "recvfrom" );
			}
			else
			{
				buffer[number_of_bytes_received] = '\0';
				printf( "Received : %s\n", buffer );
				fprintf(output, "pakket %d van %d : %s\n", i+1, qPakket, buffer);
				totPakket = totPakket +1;
			}

	}
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\ncycle time was %.2f seconds\n", time_spent);
	printf( "\nTotal packets received : %d\n", totPakket );
	printf("Total packets missed : %d\n", qPakket - totPakket);
	percentPakket = (float) totPakket / qPakket * 100;
	printf("accuracy rate is : %.2lf percent\n", percentPakket);
	fprintf(output, "Totaal tijd tussen eerste en laatste pakket is %.2f seconden.\n\n", time_spent);
	fprintf(output, "\nTotal packets received : %d\n", totPakket);
	fprintf(output, "Total packets missed : %d\n", qPakket - totPakket);
	fprintf(output, "accuracy rate is : %.2lf percent\n", percentPakket);

	//Step 2.2
	int number_of_bytes_send = 0;
	number_of_bytes_send = sendto( internet_socket, "Hello UDP world!", 16, 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
	if( number_of_bytes_send == -1 )
	{
		perror( "sendto" );
	}

		fclose(output);
}

void cleanup( int internet_socket )
{
	//Step 3.1
	close( internet_socket );
}

void intro(void)
{
    system("cls");
	printf("\n\n\n*********************************************************\n");
	printf(" - - - - - - - - -  UDP SERVER launched  - - - - - - - -\n");
	printf("*********************************************************\n\n\n");
}