#include<netinet/in.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include<stdio.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include<fcntl.h> 
#include<stdlib.h>
#include <time.h>

#include<errno.h>

#define BUF_SIZE 1000
#define RECEIVE_SIZE 256
#define PORT_NO 1101	// used if port_no is hardcoded
#define MAX_ITERS 1000

// This assignment belongs to 12775, Utsav Sinha

void error ( char *msg )	// error displaying, a handy function
{
    perror( msg );
    exit(1);
}

void create_file ( int server, char *file_name, char *file_contents );

int main ( ) 
{ 
	int s1, s2, if_s1_connected, if_s2_connected; 	// s1 and s2 corresponds to the 2 servers the client is connecting to
	int port_no1, port_no2;
	char server1_ip[100] = "", server2_ip[100] = "";

	printf("Enter port number for 1st and 2nd servers\n");
	scanf("%d %d", &port_no1, &port_no2);
	if ( port_no1 < 0 || port_no2 < 0 )
		error("negative port numbers ");

	printf("Enter the ip addresses of the 2 servers\n");
	scanf("%s %s",server1_ip, server2_ip);

	struct sockaddr_in server1, server2; 
	s1 = socket( AF_INET, SOCK_STREAM, 0 ); 
	s2 = socket( AF_INET, SOCK_STREAM, 0 ); 
	if( s1 < 0 ) 
		error("Socket not created "); 
	if( s2 < 0 )
		error("Socket not created ");

	bzero( &server1, sizeof(struct sockaddr_in) ); 
	server1.sin_family = AF_INET; 
	server1.sin_port = htons( port_no1 ); 
	if ( !inet_aton( server1_ip, &server1.sin_addr ) ) // "172.31.4.210" can a valid address which can be hard coded here
	{
		error("Server1's ip is invalid ");
		// The inet_aton() function returns 1 if the address is successfully converted, or 0 if the conversion failed. 
	}	

	bzero( &server2, sizeof(struct sockaddr_in) ); 
	server2.sin_family = AF_INET; 
	server2.sin_port = htons( port_no2 ); 
	if ( !inet_aton( server2_ip, &server2.sin_addr ) )	// "127.0.0.1" can be a valid string for 1st argument
	{
		error("Server2's ip is invalid ");
	}

	if_s1_connected = connect( s1, (struct sockaddr *)&server1, sizeof(server1) ); // If the connection or binding succeeds, zero is returned.  On error, -1 is returned, and errno is set appropriately.
	if ( if_s1_connected != 0 )
	{
		error("server 1 not connected ");
	}

	char buf[BUF_SIZE] = "";
	char buf1[RECEIVE_SIZE] = "";	// contains data received from server 1
	char buf2[RECEIVE_SIZE] = "";	// contains data received from server 2
 
	printf("Enter file Name: "); 
	scanf("%s", buf);  


	write( s1, buf, strlen(buf) ); 
	read( s1, buf1, RECEIVE_SIZE );	// receives the size of the file or -1 id file does not exist

	if ( strcmp( buf1, "-1" ) == 0 )	// means file was not found in this server, so lets try to connect to second server
	{
		if_s2_connected = connect( s2, (struct sockaddr *)&server2, sizeof(server2) ); 
		if ( if_s2_connected != 0 )
		{
			printf("File not found on server 1\n");
			error("server 2 not connected ");
		}
		write( s2, buf, strlen(buf) ); 
		read( s2, buf2, RECEIVE_SIZE );
		if ( strcmp( buf2, "-1" ) == 0 )	// server sends -1 if file is not found, otherwise it sends the size of the file in bytes
		{
			printf("RECEIVED FROM SERVERS:\tFile not found\n"); 
		}
		else	// file found on the second server
		{
			//printf("RECEIVED FROM SERVER 2\n%s\n", buf2);
			create_file( s2, buf, buf2 );
		}
		close( s2 );
	}
	else
	{
		//printf("RECEIVED FROM SERVER 1\n%s\n", buf1);
		create_file( s1, buf, buf1 ); 
	}
	close( s1 );
	return 0;
}


void create_file ( int server, char *file_name, char *first_chunk )	// it creates the file on the client side. This file is the same file which the client requested from the server
{
	FILE * fp;
	int make_file_name_unique = (int)time(NULL);	
	// appending timestamps could also make the file name unique but looks ugly, so appending unique_string

	char unique_string[1000] = "uutsav_";
	char recvBuff[RECEIVE_SIZE] = "";
	int bytesReceived = 0, bytes_read = 0, counter = 0;
	int bytesToRead = RECEIVE_SIZE;
	int byte_written;	// bytes written on the newly created file in the latest chunk read from buffer

	strcat( unique_string, file_name );
	strcpy( file_name, unique_string );

	//printf("%s\n",file_name);

	fp = fopen ( file_name, "ab" );

	long file_size = atoi(first_chunk);
	long file_read = 0;	// thus variable stores the file size which has already been read by the client. On complete transwer, this must be equal to file_size
	printf("Size of file transferred %ld\n\n",file_size);
	
	if ( file_size == 0 )	// we transferred an empty file from server
	{
		printf("file transfered\n");
		fclose(fp);
		return ;
	}
	bytes_read = read( server, recvBuff, RECEIVE_SIZE );
	file_read += (long)bytes_read;
	//printf("%d bytes read Packet: %d\n",bytes_read, ++counter);

	int bytes_read_this_time = 0;	// bytes read till now is used when server sent RECEIVE_SIZE packets but somehow less number of bytes are present on the buffer. Then we keep on looping till bytes_to_read ( = RECEIVE_SIZE ) becomes equal to bytes_read. Then, till variable stores the additional bytes read

	int iters_elapsed;

	while( bytes_read > 0 && file_read < file_size )	 
	{
		
		while (bytesToRead != bytes_read )
		{
			//printf("\t\t\t\tCompleting 256 bytes\n");
			//printf("file read till now %ld should be %d\n",file_read, counter*RECEIVE_SIZE);
			iters_elapsed = 0;
			do
			{
			//	printf("iters elapsed is %d\n",iters_elapsed++);	
				bytes_read_this_time = read(server, recvBuff + bytes_read, (bytesToRead - bytes_read));
			//	printf("%d\n", bytes_read_this_time);		
			}
			while((bytes_read_this_time == -1) && (errno == EINTR) );
			//printf("out of while\n");
			if (bytes_read_this_time == -1)
			{
				printf("\t\t\t\tinside error file transfered\n"); 
				//return;
			}
			bytes_read += bytes_read_this_time;
			file_read += (long)bytes_read_this_time;
		//	printf("\t\t\t\tread this time: %d\n",bytes_read_this_time);
		} 

		byte_written = fwrite( recvBuff, 1, bytes_read, fp ); // fwrite function returns the total number of elements successfully written is returned as a size_t object, which is an integral data type. If this number differs from the nmemb parameter, it will show an error.
		// printf("%d bytes written to file\n",temp); 
		
		bytes_read = read( server, recvBuff, RECEIVE_SIZE );
		file_read += (long)bytes_read;
	//	printf("%d bytes read Packet: %d\n",bytes_read, ++counter);
	//	printf("file read till now %ld should be %d\n",file_read, counter*RECEIVE_SIZE);
		printf("file read till now %ld B %.2lf %% of download complete\r",file_read, (100.0*(double)file_read)/(double)file_size );
	}	
	byte_written = fwrite( recvBuff, 1, bytes_read, fp );
	//printf("%d bytes written to file\n", byte_written);	// writing the last chunk of bytes to the created file
	printf("file transfered\n");

	fclose(fp);
	return ;
}









