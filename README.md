project4
Rob Freedy, Nicholas Haydel, Patrick Schurr
Net ID's: rfreedy, nhaydel, pschurr
November 16th, 2016

There are two directories along with this README in the program4 directory:
Client
Server

The Client directory contains the following files: 
Makefile
myfrm.c
testfile.txt

The Server directory contains the following files:
Makefile
myfrmd.c
listing.txt
Passwords.txt
Users.txt

In order to compile and execute the field in the Server directory, run the following the commands:
make clean
make
./myfrmd <port_number> <admin_password>

<port_number> = the port number
<admin_password> = the password for the admin user 


In order to compile and execute the field in the Client directory, run the following the commands:
make clean
make
./myfrm <host> <port_number>

<host> = student machine that the server is hosted on
<port_number> = the port number

In order to run this program correctly, please start the server side first and then start the client.After both the server and the client ade started, please following the command prompts on both the server and client sides. The client will accept the following commands: 
CRT: Create Board
LIS: List Boards
MSG: Leave Message
DLT: Delete Message
RDB: Read Board
EDT: Edit Message
APN: Append File
DWN: Download File
DST: Destroy Board
XIT: Exit 
SHT: Shutdown Server

In order to complete the task, follow the instructions on the client side to complete the task.



