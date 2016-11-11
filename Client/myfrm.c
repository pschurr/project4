//Rob Freedy, Nicholas Haydel, Patrick Schurr
//Computer Networks
//Assignment 4
//NetIDs: nhaydel, pschurr, rfreedy
//myfrm.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#define MAX_LINE 256
#define MAX_COMMAND 256

int main(int argc, char * argv[]){
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	char ack[MAX_LINE];
	struct timeval tv,tv2;
        double elapsed_time;
	int s, s_udp, ret, len, new_s1;
	int server_port;
	char command[MAX_LINE];
	char operation[10];
	char decision[3];
	char file_name[MAX_LINE];
	socklen_t addr_len = sizeof(sin);
	if (argc==3) {
		host = argv[1];
		server_port = atoi(argv[2]);
	}else {
		fprintf(stderr, "usage: simplex-talk host\n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);

	if (!hp) {
		fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
		exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(server_port);

	/* active open */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket"); 
		exit(1);
	}
	if ((s_udp = socket(PF_INET, SOCK_DGRAM, 0)) < 0) { 
		perror("simplex-talk: socket");
		exit(1);
	}
	
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("simplex-talk: connect");
		close(s); 
		exit(1);
	}
		
	/*if (connect(s_udp, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("simplex-talk: connect");
		close(s); 
		exit(1);
	}*/

	int usercheck = 0;
	char username[MAX_COMMAND];
	char password[MAX_COMMAND];
	int ret1 = -4;
	while(usercheck == 0){
		//Sending the Username
		//bzero((char *)&sin, sizeof(sin));
		printf("Please enter a username : ");
		memset(username,'\0',strlen(username));
		fgets(username, sizeof(username), stdin);
		strtok(username,"\n");
		if(sendto(s_udp,username,strlen(username),0,(struct sockaddr*) &sin, sizeof(struct sockaddr)) == -1){
			perror("Client send error");
			exit(1);
		}
		char response[2];
                if(recvfrom(s_udp, response, 2, 0, (struct sockaddr*) &sin, &addr_len)==-1){
                        printf("Server password verification receive error");
                        exit(1);
                }
		int resp = atoi(response);
		if (resp<0){
			printf("Unable to find or create user.\n");
			continue;
		}
		memset(response,'\0',strlen(response));

		//Sending the Password
		printf("Please enter a password : ");
		memset(password,'\0',strlen(password));
		fgets(password, sizeof(password), stdin);
		strtok(password, "\n");
		if(sendto(s_udp,password,strlen(password),0, (struct sockaddr*) &sin, sizeof(struct sockaddr))== -1){
			perror("Client send error");
			exit(1);
		}
		memset(password,'\0',strlen(password));
		//Receving verification from the server the the user is good to create messages
		char ack1[1];
		memset(ack1,'\0',sizeof(ack1));

		if(recvfrom(s_udp, ack1, 1, 0, (struct sockaddr*) &sin, &addr_len)==-1){
			printf("Server password verification receive error");
			exit(1);
		}
		
		usercheck = atoi(ack1);
		if(usercheck == 0){
			printf("Incorrect Username/Password. Please enter a valid username and password combination.\n");
		}		
	}
	printf("Welcome to the Message Board!\n");

	/* main loop: get and send lines of text */
	while (1){//fgets(buf, sizeof(buf), stdin)) {
		//buf[MAX_LINE-1] = '\0';
		memset(operation,'\0',strlen(operation));
		printf("Please enter an operation for the message board : CRT, LIS, MSG, DLT, RDB, EDT, APN, DWN, DST, XIT, SHT: ");   // PSchurr prompt user input
		while(strlen(operation)<3){
			memset(operation,'\0',strlen(operation));
			fgets(operation, sizeof(operation), stdin);
			strtok(operation,"\n");	
		}
		
		len=strlen(operation) +1;
		
		// CRT
		if(strcmp("DWN", operation) == 0) {
			if(sendto(s,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
				perror("client send error!"); 
				exit(1);	
			}
			printf("Please enter the name of the board to create: ");
			fgets(file_name, sizeof(file_name), stdin);
			strtok(file_name, "\n");
			int name_len = strlen(file_name)+1;
			char len_str[10];
			snprintf(len_str, 10, "%d", name_len);//send file name and length as strings.
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){
				perror("client send error: Error sending file name length!");
				//exit(1);
				continue;
			}
			file_name[name_len] ='\0';
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                //exit(1);
                                continue;
                        }
			char size[10];
			if((ret = recv(s,size, 10, 0))<0){
				perror("client receive error: Error receiving file length!");
				//exit(1);
				continue;
			}
			int file_size = atoi(size);

			if ( file_size >= 0){ // Server returns a negative file length if file doesn't exist on server
				unsigned char hash[16];
				int ret = 0;
				int t =0;
				ret = recv(s,hash, 16, 0);
				hash[16]='\0';
				if(ret<0){//Check if the proper hash was received.
					perror("client receive error: Error receiving file hash!");
					//exit(1);
					continue;
				}
				fp = fopen(file_name, "w");
                                if(fp==NULL){
                                        printf("Error opening file\n");
                                        exit(1);
                                }

				fflush(fp);
				char content[1000];
				ret = 0;
				t = 0;
				gettimeofday(&tv, NULL);
				while(ret < file_size){//Receive file content
		
					t = recv(s,content,1000,0);
					if (t < 0){
						ret = t;
						break;
					} 
					ret = ret + t;
					content[t]='\0';
					fwrite(content, t,1,fp);
					fflush(fp);
				}
                                gettimeofday(&tv2, NULL);
                                elapsed_time = ((tv2.tv_sec*1000000+tv2.tv_usec) - (tv.tv_sec*1000000+tv.tv_usec));
				elapsed_time = elapsed_time/1000000.0;
				fclose(fp);
					
				if(ret<0){
					perror("client recieve error: Error receiving file content!");
					//exit(1);
					continue;
				}
				//Cleanup here
				
			}
			else {
				printf("%s does not exist.\n", file_name);
			}
			
			

		//MSG
		} else if( strcmp("APN", operation) == 0) {
			if(send(s,operation,len,0)==-1){  // client sends operation to upload a file to server
				perror("client send error!"); 
				exit(1);	
			}
			// client gets file name and checks if it exists
			printf("Please enter the name of the file to be uploaded: ");
			fgets(file_name, sizeof(file_name), stdin);
			strtok(file_name, "\n");
			int name_len = strlen(file_name)+1;
			char len_str[10];
			snprintf(len_str, 10, "%d", name_len);
			fp = fopen(file_name, "r");			
			
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){ // send file name length
				perror("client send error: Error sending file name length!");
				continue;
			}
			file_name[name_len] ='\0';
			// client send file name (char string)
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                continue;
                        }
			char ack[3];
			// receive ACK
			if(recv(s, ack, 4, 0)==-1){
				perror("Client receive error: Error receiving acknowledgement!");
				continue;
			}
			if( strcmp("ACK", ack) !=0){ // make sure server sent proper acknowledgement
				perror("Acknowledgement not properly received!");
				continue;
			}
			if(fp == NULL){
				ret = send(new_s1, "-1", 2,0);
				if(ret == -1){
                                        perror("server send error: Error sending file size");
                                }

                                continue;
			}
			

			// client send 32 bit value of file size
			fseek(fp, 0L, SEEK_END);
                        int size = ftell(fp);
                        rewind(fp);
                        char file_size[10];
                        snprintf(file_size, 10, "%d", size);
                        if(send(s, file_size, 10, 0)==-1){
                                perror("Client send error: Error sending file size");
				continue;
			}
			unsigned char *hash;
                        char content[1000];
			fclose(fp);
                        fp = fopen(file_name,"r");
                        if(fp==NULL){
                               printf("Error opening file\n");
                               exit(1);
                        }

                        memset(content,0,strlen(content));
			int read = 0;
                        while (read<size){
                                int bytes=fread(content,sizeof(char),1000,fp);
                                read+=bytes;
                                ret = send(s, content,bytes , 0);
                                if(ret == -1){
                                        break;
                                }
                                memset(content,0,strlen(content));
                        }
                        fclose(fp);
			if(ret==-1){
                                perror("Client send error: Error sending file");
				continue;

			}
                        char time[10];
                        ret = recv(s,time,10,0);//Receive the elapsed time from server.
                        if (ret==-1){
				perror("Client receive error: Error receiving throughput info");
				continue;
			}
			double elapsed = atof(time);
			if (elapsed == -1.0){
				printf("Error: upload unsuccessful.\n");
			}
			else{
                                        printf("%i bytes transferred in %lf seconds: %lf Megabytes/sec\n", size, elapsed,(size/(elapsed*1000000)));
                                        printf("File MD5sum: ");
                                        int i;
                                        for (i =0;i<16;i++) printf("%0x", hash[i]);
                                        printf("\n");
	
			}




		// CRT	
		} else if(strcmp("CRT",operation)==0){
			char board_name[MAX_COMMAND];
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                        printf("Please enter the name of the board to create: ");
                        fgets(board_name, sizeof(board_name), stdin);
                        strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                	char response[2];
                	if(recvfrom(s_udp, response, 2, 0, (struct sockaddr*) &sin, &addr_len)==-1){
                        	printf("Server password verification receive error");
                        	exit(1);
                	}
                	int resp = atoi(response);
                	if (resp==-1){
                        	printf("Unable to create board.\n");
                        	continue;
                	}
			else if(resp==-2){
				printf("Board %s already exists.\n", board_name);
			}
			else {
				printf("Board %s successfully created!\n", board_name);
			}
                	memset(response,'\0',strlen(response));


	
		} else if (strcmp("MSG", operation)==0){
                        char board_name[MAX_COMMAND];
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                        printf("Please enter the name of the board to post to: ");
                        fgets(board_name, sizeof(board_name), stdin);
                        strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char message[MAX_COMMAND];
			printf("Please enter the message: ");
			fgets(message,sizeof(message),stdin);
			strtok(message,"\n");
                        if(sendto(s_udp,message,strlen(message), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char response[2];
                        if(recvfrom(s_udp, response, 2, 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Server password verification receive error");
                                exit(1);
                        }
                        int resp = atoi(response);
                        if (resp==-1){
                                printf("Board does not exist.\n");
                                continue;
                        }
                        else {
                                printf("Successfully added message to %s!\n", board_name);
                        }

		}else if (strcmp("DLT", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char board_name[MAX_COMMAND];
			printf("Please enter the board to delete from: ");
                        fgets(board_name, sizeof(board_name), stdin);//Send file info to server
                        strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char message_id[10];
			printf("Please enter the id of the message to delete: ");
                        fgets(message_id, sizeof(message_id), stdin);//Send file info to server
                        strtok(message_id, "\n");
                        if(sendto(s_udp,message_id,strlen(message_id), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char response[2];
                        if(recvfrom(s_udp, response, 2, 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Server password verification receive error");
                                exit(1);
                        }
                        int resp = atoi(response);
                        if (resp==-1){
                                printf("Board does not exist.\n");
                                continue;
                        }
			else if(resp==-2){

				printf("Invalid message id\n");
				continue;
			}
			else if(resp==-4){
				printf("Invalid user.\n");
				continue;
			}
			else if(resp==-5){
				printf("Message does not exist.\n");
				continue;
			}
			
                        else {
                                printf("Successfully deleted message from %s!\n", board_name);
                        }
	

			//memset(operation,0,strlen(operation));	
		
		// LIS
		} else if (strcmp("LIS", operation) == 0) {
			if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
	
			char boards[MAX_COMMAND];
                        if(recvfrom(s_udp, boards, sizeof(boards), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
			printf("%s",boards);


		// EDT
		} else if (strcmp("EDT", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
			printf("Please enter the directory name to create: ");
                        char dir_name[MAX_LINE];
			
			fgets(dir_name, sizeof(dir_name), stdin);
                        strtok(dir_name, "\n");
                        int name_len = strlen(dir_name)+1;
                        char len_str[10];
                        snprintf(len_str, 10, "%d", name_len);
				
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){//Send the directory name and length of name
				perror("client send error: Error sending directory name length!");
                                continue;
                        }
                        dir_name[name_len] ='\0';
                        if(send(s, dir_name, name_len, 0)==-1){
                                perror("client send error: Error sending directory name!");
                                continue;
                        }
			char success[10];
			if(recv(s, success, 10,0)==-1){//Check success val;
				perror("Client receive error: Error receiving server confirmation!");
				continue;
			}
			int succ = atoi(success);
			if(succ == -2){
				printf("The directory already exists on the server!\n");
				continue;
			} else if(succ ==-1){
				printf("Error making directory!\n");
				continue;
			}else if (succ >0){
				printf("The directory was successfully made!\n");
			}
		
	
		// RDB
		} else if (strcmp("RDB", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);	
			}
			printf("Please enter the directory to delete: ");
                        fgets(file_name, sizeof(file_name), stdin);
                        strtok(file_name, "\n");
                        int name_len = strlen(file_name)+1;
                        char len_str[10];
                        snprintf(len_str, 10, "%d", name_len);

                         if(send(s, len_str, strlen(len_str)+1, 0)==-1){
                                perror("client send error: Error sending directory name length!");
                                continue;
                        }
                        file_name[name_len] ='\0';
                        if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending directory name!");
                                continue;
                        }
			char conf[2];
			ret = recv(s,conf, 2,0);
			if (ret < 0){
				perror("client receive error: Error receiving directory existence confirmation");
				continue;
			}
			int c = atoi(conf);
			char decision[3];
			if (c > 0){
				int i = 0;
				while(1){
					printf("Are you sure you want to delete this file? Enter Yes or No: ");
					fgets(decision, sizeof(decision)+1, stdin);
					strtok(decision,"\n");
					decision[3]='\0';//Ensure only the confirmation is sent.
					if(strcmp("Yes",decision)==0){

					break;
					}else if(strcmp("No",decision) ==0){
						break;	
					}else{
						printf("Enter a valid decision\n");
					}
				}
				if (strcmp(decision,"No") == 0){
					printf("Delete abandoned by the user!\n");
					if(send(s, decision, strlen(decision),0) < 0){
						perror("client send error: Error sending deletion confirmation!");
						continue;
					}
				}
				else {
					if(send(s, decision, strlen(decision),0) < 0){
						perror("client send error: Error sending deletion confirmation!");
						continue;
					}
					char success[2];
					ret = recv(s, success, 2, 0);
					if (ret < 0){
						perror("client receive error: Error receiving directory deletion confirmation");
						continue;
					}
					int succ = atoi(success);
					if (succ > 0){
						printf("Directory deleted\n");
					
					}
					else if(succ < 0){
						printf("Failed to delete directory\n");
				
					}
				}
			} 
			else {
				printf("%s does not exist on server\n", file_name);
				continue;
			}
		// APN
		} else if(strcmp("APN",operation) ==0){
			if(send(s,operation,len,0) ==-1){
				perror("Client send error!");
				exit(1);
			}
                        char dir_name[MAX_LINE];
			printf("Please enter the directory name: ");
			fgets(dir_name, sizeof(dir_name), stdin);
			strtok(dir_name, "\n");
			int name_len = strlen(dir_name)+1;
			char len_str[10];
			snprintf(len_str, 10, "%d", name_len);
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){
				perror("Client send error: Error sending directory name length!");
				continue;
			}
			dir_name[name_len] ='\0';
                        if(send(s, dir_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                //exit(1);
                                continue;
                        }
			// recv confirm
			char conf[2];
			if(recv(s,conf, 2,0)==-1){
				perror("client receive error: Error receiving directory deletion confirmation");
				continue;
			}
			int confirm = atoi(conf);
			

			if(confirm>0){
				printf("Changed current directory\n");
				continue;
			}else if(confirm =-1){
				printf("Error in changing current directory\n");
				continue;
			} else if(confirm =-2){
				printf("The directory does not exist on server\n");
				continue;
			}

	
		} else if (strcmp("DWN", operation) == 0) {
		} else if (strcmp("DST", operation) == 0) {
		} else if (strcmp("XIT", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}
			close(s);
			printf("Session has been closed.\n");
			return 0;	
		} else if (strcmp("SHT", operation) == 0) {
		}
	}

	close(s);
}

