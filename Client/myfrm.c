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
#include <sys/time.h>
#include <netdb.h>
#include <mhash.h>
#define MAX_LINE 256

int main(int argc, char * argv[]){
	MHASH td;
	FILE *fp;
	struct hostent *hp;
	struct sockaddr_in sin;
	char *host;
	char buf[MAX_LINE];
	char ack[MAX_LINE];
	struct timeval tv,tv2;
        double elapsed_time;
	int s, ret, len, new_s1;
	int server_port;
	char command[MAX_LINE];
	char operation[10];
	char decision[3];
	char file_name[MAX_LINE];
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
	
	printf("Welcome to your first TCP client! To quit, type \'Exit\'\n");

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("simplex-talk: connect");
		close(s); 
		exit(1);
	}

	/* main loop: get and send lines of text */
	while (1){//fgets(buf, sizeof(buf), stdin)) {
		//buf[MAX_LINE-1] = '\0';
		memset(operation,'\0',strlen(operation));

		printf("Please enter an operation (REQ, UPL, DEL, LIS, MKD, RMD, CHD, XIT): ");   // PSchurr prompt user input
		while(strlen(operation)<3){
			memset(operation,'\0',strlen(operation));
			fgets(operation, sizeof(operation), stdin);
			strtok(operation,"\n");	
		}
		
		len=strlen(operation) +1;
		
		// REQ
		if(strcmp("REQ", operation) == 0) {
			if(send(s,operation,strlen(operation)+1,0)==-1){
				perror("client send error!"); 
				exit(1);	
			}
			printf("Please enter the requested file name: ");
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
				fp = fopen(file_name, "r");
				if(fp==NULL){
					printf("Error opening file\n");
					exit(1);
				}
				unsigned char temp[1000];
				td = mhash_init(MHASH_MD5);
				if (td == MHASH_FAILED) return 1; 
				int bytes = 0;
				while ((bytes=fread(&temp, sizeof(char),1000, fp) != 0))//Calculate the hash of the whole file.
    				{
					
      					mhash(td, &temp, 1000);
    				}
				fclose(fp);
				unsigned char *serverHash = mhash_end(td);
				hash[16] = '\0';
				if(strcmp(hash,serverHash) == 0){
					printf("%i bytes transferred in %lf seconds: %lf Megabytes/sec\n", file_size, elapsed_time,(file_size/(elapsed_time*1000000)));
					printf("File MD5sum: ");
					int i;
					for (i =0;i<16;i++) printf("%0x", hash[i]);
					printf("\n");
				}

				else{
					printf("Failed to receive %s.\n", file_name);
				}	
				//Cleanup here
				
			}
			else {
				printf("%s does not exist.\n", file_name);
			}
			
			

		//UPL
		} else if( strcmp("UPL", operation) == 0) {
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
                        td = mhash_init(MHASH_MD5);
                        if (td == MHASH_FAILED) return 1;
                        int bytes = 0;
                        while ((bytes=fread(&content, sizeof(char),1000, fp) != 0)){
                                mhash(td,&content,1000);
                        }

                        hash = mhash_end(td);
                        hash[16]='\0';
                        len = strlen(hash);
			ret = send(s,hash,len,0);
                        if(ret==-1){
                                perror("Client send error: Error sending hash");
				continue;
			}
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




		// DEL	
		} else if (strcmp("DEL", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
			
			printf("Please enter the file to delete: ");
                        fgets(file_name, sizeof(file_name), stdin);//Send file info to server
                        strtok(file_name, "\n");
                        int name_len = strlen(file_name)+1;
                        char len_str[10];
                        snprintf(len_str, 10, "%d", name_len);
		
			if(send(s, len_str, strlen(len_str)+1, 0)==-1){
				perror("client send error: Error sending file name length!");
                                continue;
                        }
                        file_name[name_len] ='\0';
                        
			if(send(s, file_name, name_len, 0)==-1){
                                perror("client send error: Error sending file name!");
                                continue;
                        }
			char size[10];
  			
			if(recv(s,size, 10, 0)==-1){
				perror("client receive error: Error receiving file length!");
				continue;
			}
			int file_size = atoi(size);
			
			if(file_size<0){ // make sure file exists on server side 
				continue;
			}
			int c =0;
			char decision[3];
			while(1){
				printf("Are you sure you want to delete this file? Enter YES or NO: ");
				fgets(decision, sizeof(decision)+1, stdin);
				strtok(decision,"\n");
				//printf("%s\n",decision);
				if(strcmp("YES",decision)==0){
					if(send(s,"1",len,0)==-1){
						perror("client send error!"); 
						exit(1);
					}
					break;
				}else if(strcmp("NO",decision) ==0){
					if(send(s,"-1",len,0)==-1){
						perror("client send error!"); 
						exit(1);
					}
					break;	
				}else{
					printf("Enter a valid decision\n");
				}
			}
  			
			if(recv(s,size, 10, 0)==-1){
				perror("client receive error: Error receiving file length!");
				continue;
			}
			int did_del= atoi(size);
			
			if(did_del<0){ // make sure file was deleted.
				printf("Delete was not successful\n");
			} else{
				printf("Delete successful\n");
			} 

			//memset(operation,0,strlen(operation));	
		
		// LIS
		} else if (strcmp("LIS", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}	
			char size[10];
  			if((ret = recv(s,size, 10, 0))<0){
				perror("client receive error: Error receiving file length!");
				//exit(1);
				continue;
			}
			int file_size = atoi(size);
			ret = 0;
			int t = 0;
			char content[file_size+1];
			char temp[file_size];
			while(ret < file_size){
                                t = recv(s,temp,file_size,0);
                                if (t <= 0){
                                       ret = t;
                                       break;
                                }
                                ret = ret + t;
                                strcat(content, temp);
                                //if(ret < file_size) content[t]=0;
                                memset(temp,0,strlen(temp));
	
			}
			content[file_size]='\0';
			if(ret<0){
				perror("client receive error: Error receiving file listing!");
				continue;
			}
			printf("%s\n", content);
		

		// MKD
		} else if (strcmp("MKD", operation) == 0) {
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
		
	
		// RMD
		} else if (strcmp("RMD", operation) == 0) {
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
		// CHD
		} else if(strcmp("CHD",operation) ==0){
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

	
		} else if (strcmp("XIT", operation) == 0) {
			if(send(s,operation,len,0)==-1){
				perror("client send error!"); 
				exit(1);
			}
			close(s);
			printf("Session has been closed.\n");
			return 0;	
		}
	}

	close(s);
}

