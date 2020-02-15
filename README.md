Files Submitted:
C file: peer.c, server.c
Out file: peer, server
Directory: p2p-files for storing files to be shared by client



File created at runtime:
	pee-list.txt: To store the list of peers connected.
	File-list.txt: To store the list of files along with the ip and port number of peer



Command to execute file:
For server: 
Compile: cc server.c -o server
Execute: ./server
The Ip and port address pf server will be displayed

For peer:
Compile: cc peer.c -o peer
Execute: ./peer <server ip> <server port>

