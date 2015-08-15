This assignment belongs to 12775, Utsav Sinha


Transfer the server.c to 2 locations (may be on different systems with different IP's) and compile them with
gcc server.c -o server

Keep the client.c file where the client is present and complie it using
gcc client.c -o client

Run both the servers with
./server
The servers will each ask for a port number, specify a free port number.

Run the client with
./client
The client asks for 2 ports. Specify the same 2 ports which you entered for the servers.
The client then asks for 2 IP addresses corresponding to the 2 servers.
Enter the IP addresses of the 2 servers such that port1 typed above corresponds to server1's IP typed now and port2 typed above corresponds to server2's IP address

Now, the client asks the file name.
Enter a file name that does not contain spaces. (since we are scanning using %s with scanf(), so file names cannot contain spaces)

The server1 first try to locate the given file name in the directory in which it is running and recursively in all the subdirectories inside the current directory ( in which ./server is running )
If it finds the file, it sends the size of the file to the client as the first packet.
Then get_file_contents( ) function is called in server1 which fetches the cintents of the file and sends it to the client in chunks of RECEIVE_SIZE bytes.
The progress of the file upload is shown on the server as well as the progress of download is shown on the client's terminal.
After the entire file is transferred, the client terminal closes and a new file named uutsav_OriginalFileNameToFind is created in the directory from which ./client was run

If the file is not found on server1, then server1 sends a packet containing -1 as the size of the file which is interpreted as file not found by the client.
Now the client connects to server2 and sends the file name to server2.
Server2 also tries to find the file and send it to client in a similar fashion as server1.

If the file is not found on server2 also, then server2 also sends -1 as the file size which is interpreted as file not found by the client.
The client now prints "RECEIVED FROM SERVERS: File not found"

If the file is found in the above steps, then a new file is created as explained earlier.

Appropriate errors are displayed and server/client is closed if any of the following happens:
	Port number is negative
	Socket is not created
	Bind error occurs at server
	client connect is refused by the server
	IP address of any of the servers is invalid
	server1 refuses connection to client
	if file is not found on server1 and server2 then refuses connection to client 

An example:

SERVER 1
	$./server
	Enter port number for the server
	3333
	current working directory is /users/cse/bt/uutsav
	Ready to accept requests
	Ready to accept requests
	accept value 4
	Message from client: mm.mkv
	File requested by client not found
	Ready to accept requests

SERVER 2
	uutsav@ubuntu:~/cs425$ ./server
	Enter port number for the server
	2222
	current working directory is /home/uutsav/cs425/assignment1/12775_assignment1
	Ready to accept requests
	Ready to accept requests
	accept value 4
	Message from client: mm.mkv
	File Found: /home/uutsav/cs425/assignment1/12775_assignment1/testing/mm.mkv
	Sending to client
	file read till now 989163577 B 100.00 % of download complete
	End of file
	Ready to accept requests

CLIENT
	uutsav@ubuntu:~/cs425$ ./client 
	Enter port number for 1st and 2nd servers
	3333
	2222
	Enter the ip addresses of the 2 servers
	172.31.4.210
	127.0.0.1
	Enter file Name: mm.mkv
	Size of file transferred 989163577

	file transferednow 989163577 B 100.00 % of download complete
	uutsav@ubuntu:~/cs425$













