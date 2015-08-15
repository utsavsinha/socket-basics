#include<netinet/in.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include<stdio.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include<fcntl.h> 

#include <dirent.h> 


#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include<stdlib.h>

#include<errno.h>

#define BUF_SIZE 1000
#define PORT_NO 1101	// used if port_no is hardcoded
#define SEND_SIZE 256


// This assignment belongs to 12775, Utsav Sinha

void error ( char *msg )
{
    perror( msg );
    exit(1);
}

void get_file_contents( int cd, char *file_with_path );

int file_finder( char *searching, char *result, char *directory_to_search ) // returns 101 if file is found and the full file path is in "result"
{
	DIR *d;
	struct dirent *dir;
	char buff[MAXPATHLEN];	// MAXPATHLEN is typically 4096
	//printf("directory_to_search is : %s\n",directory_to_search);
	d = opendir( directory_to_search );
	if( d == NULL ) 
	{
		printf("Opening invalid directory\n");
		return 1;
	}
	while( ( dir = readdir( d ) ) ) 
	{
		if( strcmp( dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) 
		{
			continue;
		}
		if( dir->d_name[0] == '.' )	// no need to search directories which begin with "."
		{
			continue;
		}
		//printf("At %s\n",dir->d_name);
		if( dir->d_type == DT_DIR ) 
		{
			strcpy( buff, directory_to_search );	// directory_to_search acts as the current directory here
			strcat( buff, "/" );
			strcat( buff, dir->d_name );
			//printf("planning to go to %s\n",buff);
			if ( file_finder( searching, result, buff ) == 101 )
				return 101;
		} 
		else 
		{
			if( strcmp( dir->d_name, searching ) == 0 ) 
			{
				int  len;
				getcwd( result, MAXPATHLEN );
				len = strlen( result );
				strcpy( buff, directory_to_search );	// directory_to_search acts as the current directory here
				strcat( buff, "/" );
				strcat( buff, dir->d_name );
				strcpy( result, buff );
				// printf("Found: %s\n", buff );
				return 101;
			}
		}
	}
	closedir( d );
	return *result == 0;
}




int main ( ) 
{ 
	int s , cd, pid, port_no; 
	printf("Enter port number for the server\n");
	scanf("%d", &port_no);
	if ( port_no < 0 )
	{
		error("negative port numbers ");
	}

	char start_directory_to_find_file[MAXPATHLEN];	// this can be hardcoded to "/home" for example if there are no permission problems to search files in that directory. The safest option would be to search in the current directory where server is running

    getcwd( start_directory_to_find_file, MAXPATHLEN );
	printf("current working directory is %s\n", start_directory_to_find_file);
	
	struct sockaddr_in ser; 
	/*
		struct sockaddr_in 
		{
			short sin_family;	// AF_INET 
			u short sin_port;	// 16-bit port number 
			struct in_addr sin_addr;	// 32-bit netid/hostid 
			char sin_zero[8];	// unused 	
		};
	*/
	s = socket( AF_INET,SOCK_STREAM, 0 ); 	
	/* 
		socket(domain, type, protocol); 
		Protocol ’0’ means use of default protocol. 
		Socket domains
			AF_UNIX 				Unix Domain
			AF_INET 				Internet Domain
			AF_NS 					Xerox Protocols Domain
		Socket types
			SOCK_STREAM				reliable, no record boundaries
			SOCK_DGRAM				Datagram unreliable, record boundaries
			SOCK_RAW				Raw Access underlying protocol directly
			SOCK_SEQPACKET 			Sequenced reliable, record boundaries
		Return value: Upon successful completion, socket() shall return a non-negative integer, the socket file descriptor. Otherwise, a 			value of -1 shall be returned and errno set to indicate the error.
	*/

	if( s < 0 ) 
	{
		error("Socket not created "); 
	}	
	bzero( &ser, sizeof(struct sockaddr_in) ); // bzero (dest, nbytes); bzero writes null bytes to the string

	ser.sin_family = AF_INET; 

	ser.sin_port = htons( port_no ); 	// htons convert host byte order to network byte order, it is used to make sure that endianess of bytes is correct as desired by network bytes

	inet_aton("localhost", &ser.sin_addr); 	// The inet_aton() function converts the specified string, in the Internet standard dot notation, to a network address, and stores the address in the structure provided in the second parameter

	if ( bind( s, (struct sockaddr *)&ser, sizeof(ser)) < 0 ) // The bind() function shall assign a local socket address ("ser" here) to a socket identified by descriptor "s" that has no local socket address assigned. Returns 0 if no error occurs
	{
		error("Bind Error ");
	} 

	listen( s, 5 ); // int listen (int usocket, int n): The listen function enables the socket "usocket" to accept connections, thus making it a server socket. The argument "n" specifies the length of the queue for pending connections. When the queue fills, new clients attempting to connect fail with ECONNREFUSED until the server calls accept to accept a connection from the queue. 
	
	//cd=accept( s, NULL, NULL );

	while ( 1 ) 
	{  
		printf("Ready to accept requests\n");
		cd=accept( s, NULL, NULL );
		if ( cd == -1 )
			error("connection refused ");
		/*
			int accept (int usocket, struct sockaddr *addr, socklen_t *length_ptr);
			This function is used to accept a connection request on the server socket "usocket". 
			The "addr" and "length-ptr" arguments are used to return information about the name of the client socket that initiated the 			connection. 
			Accepting a connection does not make "usocket" part of the connection. Instead, it creates a new socket which becomes connected. 				The normal RETURN value of accept is the file descriptor for the new socket.

			After accept, the original socket socket remains open and unconnected, and continues listening until you close it. You can 				accept further connections with socket by calling accept again.

			If an error occurs, accept returns -1.
		*/

		char buf[BUF_SIZE] = ""; 
		char result[1000];
		pid = fork( ); 
		if( pid == 0 ) 
		{  
			printf("accept value %d\n",cd); 

			read( cd, buf, BUF_SIZE ); 	// read(int fildes, void *buf, size_t nbyte); The read() function shall attempt to read nbyte bytes from the file associated with the open file descriptor, fildes, into the buffer pointed to by buf.
			printf("Message from client: %s\n", buf); 

			if( file_finder( buf, result, start_directory_to_find_file ) == 101 ) 
			{
				printf( "File Found: %s\nSending to client\n", result );
				get_file_contents (cd, result );	// getting the file's content and sending it to client in chunks of SEND_SIZE bytes
			} 
			else 
			{
				printf("File requested by client not found\n");
				strcpy(result, "-1");	// file not found is indicated by -1 
				write( cd, result, strlen(result) ); 
			}
			
			//write( cd, result, strlen(result) ); 

			close( cd ); 
			//printf("closed cd\n");
			//exit(0);
		}
		//printf("after pid condition\n");
	} 
	close( s ); 
	return 0;
} 

void get_file_contents( int cd, char *file_with_path )
{
	FILE *fp;
	long file_size;
	int bytes_read;
	long file_read = 0;	// used to count the number of bytes read by the server
	int byte_written;

	fp = fopen( file_with_path, "rb" );
	if( !fp ) 	// this should ideally never happen since we checked the existence of file in file_finder, nevertheless it is good to check it here too in case file_finder fails !!
		error("file not found");

	fseek( fp , 0L , SEEK_END );
	file_size = ftell( fp );	// the file size which has to be send to the client
	rewind( fp );
	
	unsigned char buffer[SEND_SIZE] = {0};
	sprintf(buffer,"%ld", file_size);
	write( cd, buffer, SEND_SIZE );	// sending the file size to the client so that the client can keep tabs of how much of file is transferred

	int counter = 0;
	while ( 1 )
	{
		unsigned char buffer[SEND_SIZE] = {0};
		int bytes_read = fread ( buffer, 1, SEND_SIZE, fp) ;
		//printf("Bytes read %d \n", bytes_read);        

		if( bytes_read > 0 )
		{
			byte_written = write( cd, buffer, bytes_read );
			file_read += byte_written;
		    //printf("Sending %d bytes Packet: %d\n", byte_written, ++counter);
			printf("file read till now %ld B %.2lf %% of upload complete\r",file_read, (100.0*(double)file_read)/(double)file_size );
		}
		else
		{
			if (feof(fp))
			{
				//printf("Finally, %d bytes written Packet: %d\n",write(cd, buffer, 1), ++counter);
		        printf("\nEnd of file\n");
				break;
			}
		}
	}
	fclose(fp);
}








