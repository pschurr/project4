//Rob Freedy, Nicholas Haydel, Patrick Schurr
//Computer Networks
//Assignment 3
//NetIDs: nhaydel, pschurr, rfreedy
//myftpd.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
//#include <mhash.h>
#include <dirent.h>
#include <unistd.h>
#define MAX_PENDING 5
#define MAX_COMMAND 256
#define MAX_FILE_MESSAGE 512

int checkUsername(char user[]){
	FILE* fp;
	char line[MAX_COMMAND];
	fp = fopen("Users.txt","r");
	int i = 0;
	if(fp == NULL){
		fp = fopen("Users.txt","w");
		if (fp==NULL){
			return -1;
		}
		fprintf(fp,"%s\n",user);
		fclose(fp);
		return 1;
	}
	else {
		while (fgets(line, sizeof(line), fp)){
			i++;
			strtok(line,"\n");
			if(strcmp(line,user)==0){
				fclose(fp);
				return i;
			}
		}
		fclose(fp);
	}
	i++;
	fp = fopen("Users.txt","a");
	fprintf(fp,"%s\n",user);
	fclose(fp);
	return i;
}

int checkPassword(char pword[], int i){
	FILE* fp;
	char line[MAX_COMMAND];
	fp = fopen("Passwords.txt","r");
	int j = 0;
	if(fp == NULL){
		fp = fopen("Passwords.txt","w");
		fprintf(fp,"%s\n",pword);
		fclose(fp);
		return 1;
	}
	else {
		while (fgets(line, sizeof(line), fp)){
			j++;
			strtok(line,"\n");
			if(strcmp(line,pword)==0 && j==i){
				fclose(fp);
				return 1;
			}
		}
		fclose(fp);
	}
	if(i>j){
		fp = fopen("Passwords.txt","a");
		fprintf(fp,"%s\n",pword);
		fclose(fp);
		return 1;
	}
	return 0;
}
int main(int argc, char * argv[]){
	struct sockaddr_in sin, client_addr;
	char buf[MAX_COMMAND];
	int len;
	struct timeval tv, tv2;
	double elapsed_time;
	int s,s_udp,new_s1, new_s2, new_s3;
	int opt;
	socklen_t addr_len = sizeof(client_addr);
	int server_port;
	
	//Input Arugment Error Checking
	if(argc == 2){
		server_port = atoi(argv[1]);		
	} else {
		printf("Please run the server using the following command after compiling: ./myftpd <port_number>\n");
		exit(1);
	}

	/*build address data structure*/
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(server_port);
	
	/*setup passive open*/
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("simplex-talk: socket");
		exit(1);
	}

        if((s_udp = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
                perror("simplex-talk: socket");
                exit(1);
        }

	
	/*Set socket option*/
	if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))<0){
		perror("simplex-talk:socket");
		exit(1);
	}
        /*if((setsockopt(s_udp, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))<0){
                perror("simplex-talk:socket");
                exit(1);
        }*/

	if((bind(s,(struct sockaddr *)&sin, sizeof(sin)))<0){
		perror("simplex-talk: bind");
		exit(1);
	}
        if((bind(s_udp,(struct sockaddr *)&sin, sizeof(sin)))<0){
                perror("simplex-talk: bind");
                exit(1);
        }

	if((listen(s,MAX_PENDING))<0){
		perror("simplex-talk:listen");
		exit(1);
	}
	printf("Welcome to the Message Server\n");
	printf("Waiting for client connection. \n");
	//Waiting for the connection and then acting
	if((new_s1 = accept(s,(struct sockaddr *)&sin, &len))<0){ //Moved outside to wait for initial connection
                perror("Server Connection Error!");
                exit(1);
        }
	
	int ret = -4;
	ret = send(new_s1, "1", 10, 0);
	if (ret < 0){
		printf("Unable to connect send to client\n");
		exit(1);
	}
	char username[MAX_COMMAND];
	char password[MAX_COMMAND];
	int verified = 0;
	while(verified == 0){
		memset(username, 0,strlen(username));
		memset(password, 0,strlen(password));
		printf("Prompting for login information\n");
		
		ret = recvfrom(s_udp, username, sizeof(username), 0, (struct sockaddr *)&client_addr, &addr_len);
		if (ret == -1){
			printf("Username receive error\n");
			exit(1);
		}
//		printf("%s\n",username);
//Add check username function
		int n = checkUsername(username);
		if (n>=0){
			ret = sendto(s_udp, "1", 10, 0,(struct sockaddr *)&client_addr, addr_len);
			if (ret < 0){
				printf("Unable to connect send to client\n");
				exit(1);
			}
		}
		else {
                        ret = sendto(s_udp, "-1", 10, 0,(struct sockaddr *)&client_addr, addr_len);
                        if (ret < 0){
                                printf("Unable to connect send to client\n");
                                exit(1);
                        }

		}
        	ret = recvfrom(s_udp, password, sizeof(password), 0, (struct sockaddr *)&client_addr, &addr_len);
        	if (ret == -1){
                	printf("Password receive error\n");
                	exit(1);
        	}
//Add check password function
		int p = checkPassword(password,n);
		if(p==0){
			ret = sendto(s_udp, "0", 10, 0,(struct sockaddr *)&client_addr, addr_len);
			if (ret < 0){
				printf("Unable to send to client\n");
				exit(1);
			}
		}
		else {
			ret = sendto(s_udp, "1", 10, 0,(struct sockaddr *)&client_addr, addr_len);
			verified=1;
			if (ret < 0){
				printf("Unable to connect send to client\n");
				exit(1);
			}
		}
	
	}
	
	while(1){
		//Resetting the buf string to accept new request from the client
		memset(buf, 0,strlen(buf));	
		if (new_s1!=-1){
			printf("Prompting for client command.\n");
			ret = recvfrom(s_udp, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		}
		if (ret == 0 || new_s1==-1){	//Ret == 0 if client has closed connection
			printf("Waiting for client connection.\n");
			if((new_s1 = accept(s,(struct sockaddr *)&sin, &len))<0){
				perror("Server Connection Error!");
				exit(1);
			}
		}
		else if(ret < 0){
			perror("Server receive error: Error receiving command!");
			exit(1);
		}
		if (strcmp("DWN", buf) == 0){
			char name_len[10];
			FILE * fp;
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
                        if(ret == 0) continue; // Client has closed connection continue
                        else if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }
			//Checking to see if the file exists. If it does, continue with request. If not, send the client a -1
			fp = fopen(file_name, "r");
			if (fp == NULL){
				ret = send(new_s1, "-1", 2,0);
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
			snprintf(file_size, 10, "%d", size);
			ret = send(new_s1, file_size, 10, 0);
                        if(ret == -1){
                                perror("server send error: Error sending file size");
                                //exit(1);
                                continue;
                        }
			//Server computes MD5 hash of the file and sends it to client as a 16-byte string. 
			unsigned char *hash;
			char content[1000];
			int read = 0;
                        //mhash(td,&content , 1);
		//	mhash_deinit(td,hash);
		//	printf("%s\n",hash);
			fp = fopen(file_name,"r");
			memset(content,0,strlen(content));
			while (read<size){
				int bytes=fread(content,sizeof(char),1000,fp);
				read+=bytes;
                        	ret = send(new_s1, content,bytes , 0);
				if(ret == -1){
					break;
				}
				memset(content,0,strlen(content));
			}
			fclose(fp);
                        if(ret == -1){
                                perror("server send error: Error sending file content");
                                continue;
                        }

		}else if(strcmp("APN", buf) == 0){
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0){
				 bzero((char *)&sin, sizeof(sin));
				 continue; // Client has closed connection continue
			}
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
                        if(ret == 0) continue; // Client has closed connection continue
                        else if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }

			//Sending an acknowledgement to the client that the file name/size was received
			char *ack_string = "ACK";
			int ack_string_len = strlen(ack_string) + 1;
                        ret = send(new_s1, ack_string, ack_string_len, 0);

			//Server receiving the File Size from the client
			char size[10];
			if((ret = recv(new_s1,size, 10, 0))<0){
				perror("server receive error: Error receiving file length!");
				//exit(1);
				continue;							
			}
			int file_size = atoi(size);
			if ( file_size >= 0){ //Server returns an error message if file size is negative
                                unsigned char hash[16];
                                if(recv(new_s1,hash, 16, 0)<0){//Get that hash
                                        perror("server receive error: Error receiving file hash!");
                                        continue;
                                }

				//Receiving File
				char content[1000];
                                memset(content,0,strlen(content));
                                ret = 0;
                                int t = 0;
				FILE * fp;
				fp = fopen(file_name, "w");
				while(ret < file_size){

                                        t = recv(new_s1,content,1000,0);
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
					perror("server recieve error: Error receiving file content!");
					continue;
				}

				//Writing file and checking the hash
				fclose(fp);

			}
		}else if(strcmp("LIS", buf) == 0){
			//Opening listing.txt
			FILE * fp;
			//Sending the contents of listing.txt to the client
			fp = fopen("listing.txt", "r");
                        fseek(fp, 0L, SEEK_END);
                        int size = ftell(fp);
                        rewind(fp);
			char content[size];
			fread(content, sizeof(char), size, fp);
                        fclose(fp);
                        ret = sendto(s_udp, content, size, 0,(struct sockaddr *)&client_addr, addr_len);
                        if(ret == -1){
                                perror("server send error: Error sending file content");
                                continue;
                        }
                               
			
	
		} else if(strcmp("CRT", buf) == 0){
			
			//Server receiving the length of the file in a short int as well as the file name
			char board_name[MAX_COMMAND];
                        ret = recvfrom(s_udp, board_name, sizeof(board_name), 0, (struct sockaddr *)&client_addr, &addr_len);
                        if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }
			board_name[ret]='\0';
			if( access(board_name, F_OK ) != -1 ) {//Board exists
		                ret = sendto(s_udp, "-2", 2, 0,(struct sockaddr *)&client_addr, addr_len);
                		if (ret < 0){
                        		printf("Unable to connect send to client\n");
                        		exit(1);
                		}

				
			} 
			else {   		
				FILE * fp = fopen(board_name,"w");
				if(fp==NULL){
					ret = sendto(s_udp, "-1", 2, 0,(struct sockaddr *)&client_addr, addr_len);
		                	if (ret < 0){
                        			printf("Unable to connect send to client\n");
						fclose(fp);
                   				exit(1);
                			}
				}
				else {
		                        FILE * list;
		                       	list = fopen("listing.txt", "a");
					fprintf(list,"%s\n",board_name);
					fclose(list);
					fprintf(fp,"%s\n",username);
                                        ret = sendto(s_udp, "1", 2, 0,(struct sockaddr *)&client_addr, addr_len);
                                        if (ret < 0){
                                                printf("Unable to connect send to client\n");
						fclose(fp);
                                                exit(1);
                                        }

				}
				fclose(fp);

			}
		}else if(strcmp("MSG", buf) == 0){
			char board_name[MAX_COMMAND];
                        ret = recvfrom(s_udp, board_name, sizeof(board_name), 0, (struct sockaddr *)&client_addr, &addr_len);
                        if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }
			board_name[ret]='\0';
			char message[MAX_COMMAND];
                        ret = recvfrom(s_udp, message, sizeof(message), 0, (struct sockaddr *)&client_addr, &addr_len);
                        if(ret < 0){
                                perror("server receive error: Error receiving message!");
                                exit(1);
                        }
			message[ret]='\0';
			FILE * fp;
			if (access(board_name,F_OK)!=-1){
				char c;
				unsigned line_count = 0;
				/*fp = fopen(board_name,"r");
				while ( (c=fgetc(fp)) != EOF ) {
     			        	if ( c == '\n' )
            					line_count++;
    				}
				int message_num = (line_count)/2;
				fclose(fp);*/
				
				fp = fopen(board_name,"a");
				printf("%s \n", message);
				fprintf(fp,"%s\n", message);
				fprintf(fp,"%s\n",username);
				fclose(fp);
                                ret = sendto(s_udp, "1", 2, 0,(struct sockaddr *)&client_addr, addr_len);
                                if (ret < 0){
                                       printf("Unable to connect send to client\n");
                                       fclose(fp);
                                       exit(1);
                                 }
		
				
			}
			else {
                                ret = sendto(s_udp, "-1", 2, 0,(struct sockaddr *)&client_addr, addr_len);
                                if (ret < 0){
                                       printf("Unable to connect send to client\n");
                                       fclose(fp);
                                       exit(1);
                                 }

			}
		}else if(strcmp("CHD", buf) == 0){
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char dir_name[l];
                        ret = recv(new_s1, dir_name, l,0);
                        if(ret == 0) continue; // Client has closed connection continue
                        else if(ret < 0){
                                perror("server receive error: Error receiving file name!");
                                exit(1);
                        }

		}else if(strcmp("DEL", buf) == 0){
			FILE * fp;	
			char name_len[10];
			//Server receiving the length of the file in a short int as well as the file name
			ret = recv(new_s1, name_len, 10,0);
			if(ret == 0) continue; // Client has closed connection continue
			else if(ret < 0){
				perror("server receive error: Error receiving file name length!");
				exit(1);
			}
			int l = atoi(name_len);
			char file_name[l];
                        ret = recv(new_s1, file_name, l,0);
			
			//Sending the client either a 1 or -1 based on if the file exists or not
			fp = fopen(file_name, "r");
			if (fp == NULL){
				ret = send(new_s1, "-1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file size");
				}
				continue;
			}
			ret = send(new_s1, "1",2,0);

			//Receiving the confirmation for deleting the file. If the user confirms the delete, receiving 1, if not, receiving -1
			char confirmation_string[2];
			ret = recv(new_s1, confirmation_string, 2, 0);
			int confirm_delete = atoi(confirmation_string);
			if(confirm_delete == -1) continue;
			//Actually deleting the file
			int remove_check = remove(file_name);
			if(remove_check == 0) {
				ret = send(new_s1, "1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file deletion status");
				}
				
   			} else {
				ret = send(new_s1, "-1",2,0);
				if(ret == -1){
					perror("server send error: Error sending file deletion status");
				}
   			}
 
		}else if(strcmp("XIT", buf) == 0){
			//closing the client socket and going back to the waiting for client state
			bzero((char *)&sin, sizeof(sin));
			close(new_s1);
			new_s1 = -1;
		} else {
			continue;
		}
		
	}
	return 0;	
}

