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
		if(strcmp("CRT",operation)==0){
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
			memset(boards,0,sizeof(boards));


		// EDT
		} else if (strcmp("EDT", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
	
			printf("Please enter the name of the board to edit: ");
                        char board_name[MAX_LINE];
			
			fgets(board_name, sizeof(board_name), stdin);
                        strtok(board_name, "\n");
			char message_num[10];
			printf("Please enter the id of the message to edit: ");
			fgets(message_num,sizeof(message_num),stdin);
			strtok(message_num,"\n");
			printf("Please enter the new message: ");
			char message[MAX_LINE];
			fgets(message, sizeof(message), stdin);
			strtok(message,"\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                        if(sendto(s_udp,message_num,strlen(message_num), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                        if(sendto(s_udp,message,strlen(message), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char conf[1];
                        if(recvfrom(s_udp, conf, sizeof(conf), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
			int resp = atoi(conf);
			if(resp==1){
				printf("Board does not exist.\n");
				continue;
			}
			else if(resp == 2){
				printf("Invalid message id.\n");
				continue;
			}
			else if(resp == 3){
				printf("Invalid user.\n");
				continue;
			}
			printf("Successfully edited message %s on %s\n",message_num, board_name);


	
		// RDB
		} else if (strcmp("RDB", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char file_name[MAX_COMMAND];
			printf("Please enter the name of the board to read: ");
                        fgets(file_name, sizeof(file_name), stdin);
                        strtok(file_name, "\n");
                        if(sendto(s_udp,file_name,strlen(file_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char file_size[10];
                        if(recvfrom(s_udp, file_size, sizeof(file_size), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
			int size = atoi(file_size);
			if(size<0){
				printf("%s does not exist.\n", file_name);
				continue;
			}
			int received = 0;
			int t = 0;
			char content[1000];
			memset(content,0,sizeof(content));
                        while(received < size){//Receive file content

                                t = recv(s,content,1000,0);
                                if (t < 0){
                                       received = t;
                                       break;
                                }
                                received = received+t;
                                content[t]='\0';
                                printf("%s",content);
				memset(content,0,sizeof(content));
                        }
			if (received<0){
				printf("Failed to read board");
				continue;
			}

	
		// APN
		} else if(strcmp("APN",operation) ==0){
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char board_name[MAX_LINE];
			printf("Please enter the name of the board to append to: ");
			fgets(board_name, sizeof(board_name), stdin);
			strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

			char file_name[MAX_LINE];
			printf("Please enter the name of the file to be appended: ");
			fgets(file_name, sizeof(file_name), stdin);
			strtok(file_name, "\n");
                        if(sendto(s_udp,file_name,strlen(file_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char conf[1];
                        if(recvfrom(s_udp, conf, sizeof(conf), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
			if(atoi(conf)==1){
				printf("Requested board does not exist.\n");
				continue;
			}
			else if(atoi(conf)==2){
				printf("File already exists on %s.\n", board_name);
				continue;
			}
			else if(atoi(conf)==3){
				printf("File already exists on server.\n");
				continue;
			}
			FILE * fp;
			fp = fopen(file_name, "r");
                        if (fp == NULL){
                                ret = sendto(s_udp, "-1", 2,0,(struct sockaddr*) &sin, sizeof(struct sockaddr));
                                if(ret == -1){
                                        perror("server send error: Error sending file size");
                                }

                                continue;
                        }
                        //Getting the size of the file and sending it to the client
                        fseek(fp, 0L, SEEK_END);
                      	int size = ftell(fp);
                        rewind(fp);
			char file_size[10];
			sprintf(file_size,"%i",size);
			ret = sendto(s_udp, file_size, sizeof(file_size),0,(struct sockaddr*) &sin, sizeof(struct sockaddr));
                        if(ret == -1){
                              perror("server send error: Error sending file size");
                        }
                        char content[1000];
                        int t = 0;
                        int ret = 0;
                        while(ret < size){
			     int l = fread(content, sizeof(char), 1000, fp);
                             t = send(s,content,l,0);
                             if (t <= 0){
                                    ret = t;
                                     break;
                             }
			     ret+=t;
                             memset(content,0,sizeof(content));
                        }

                        if(ret<0){
                             perror("server recieve error: Error receiving file content!");
                             fclose(fp);
                             continue;
                      	}
			printf("Successfully added %s to %s\n", file_name, board_name);

	
		} else if (strcmp("DWN", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
		
                        char board_name[MAX_LINE];
                        printf("Please enter the name of the board to download from: ");
                        fgets(board_name, sizeof(board_name), stdin);
                        strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char file_name[MAX_LINE];
                        printf("Please enter the name of the file to download: ");
                        fgets(file_name, sizeof(file_name), stdin);
                        strtok(file_name, "\n");
                        if(sendto(s_udp,file_name,strlen(file_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
	
			char file_size[10];
                        ret = recvfrom(s_udp, file_size, sizeof(file_size), 0, (struct sockaddr *)&sin, &addr_len);
                        if(ret < 0){
                                perror("client receive error: Error receiving file size!");
                                exit(1);
                        }
			int size = atoi(file_size);
			if (size<0){
				printf("Error finding file %s\n",file_name);
				continue;
			}
			char f[MAX_LINE];
			sprintf(f,"%s-%s",board_name, file_name);
			FILE * fp = fopen(f,"w");
                        char content[1000];
                        int t = 0;
                        int ret = 0;
                        while(ret < size){

                         	t = recv(s,content,1000,0);
                                if (t <= 0){
                                       ret = t;
                                       break;
                                }
                                ret = ret + t;
                                content[t]='\0';
                                fwrite(content, t,1,fp);
                               	fflush(fp);
                        }
                        if(ret<0){
                                perror("client recieve error: Error receiving file content!");
                                fclose(fp);
                                continue;
                        }
			printf("Successfully downloaded %s\n",file_name);
			fclose(fp);



		} else if (strcmp("DST", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char board_name[MAX_LINE];
                        printf("Please enter the name of the board to destroy: ");
                        fgets(board_name, sizeof(board_name), stdin);
                        strtok(board_name, "\n");
                        if(sendto(s_udp,board_name,strlen(board_name), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

                        char conf[1];
                        if(recvfrom(s_udp, conf, sizeof(conf), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
                        if(atoi(conf)==1){
                                printf("Requested board does not exist.\n");
                                continue;
                        }
			else if(atoi(conf)==2){
				printf("User did not create board.\n");
				continue;
			}
			printf("Successfully destroyed %s\n",board_name);

		} else if (strcmp("XIT", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }

			close(s);
			printf("Session has been closed.\n");
			return 0;	
		} else if (strcmp("SHT", operation) == 0) {
                        if(sendto(s_udp,operation,strlen(operation), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
			char password[MAX_LINE];
			printf("Please enter the admin password: ");
			fgets(password, sizeof(password), stdin);
                        strtok(password, "\n");
                        if(sendto(s_udp,password,strlen(password), 0,(struct sockaddr*) &sin, sizeof(struct sockaddr))==-1){
                                perror("client send error!");
                                exit(1);
                        }
                        char conf[1];
                        if(recvfrom(s_udp, conf, sizeof(conf), 0, (struct sockaddr*) &sin, &addr_len)==-1){
                                printf("Error receiving boards");
                                exit(1);
                        }
			if(atoi(conf)==1){
				printf("Successfully shut down the server.\n");
				close(s);
				close(s_udp);
				return 0;				
			}
			else {
				printf("Password incorrect.\n");
			}
			

		}
	}

	close(s);
}

