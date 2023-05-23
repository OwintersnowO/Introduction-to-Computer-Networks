/* code based on server.c -- a stream socket server demo */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 12002    // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold
#define MSG_LEN 10000	// the maximum length of the recieved message 
#define FILEPATH_LEN 1000	//the maximum length of the file name requested

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
	
	char filename[FILEPATH_LEN];
    char* message = malloc(MSG_LEN);
    while(1) {  // main accept() loop
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
		if(recv(new_fd, message, MSG_LEN, 0) != -1){
			printf("message:\n%s", message);
			printf("------------------------------------------------\n");
			if (strcmp(strtok(message, " "),"GET") != 0){
				if (send(new_fd, "Bad request.\n", strlen("Bad request.\n"), 0) == -1)
					perror("send");
			}
			FILE *_file;
			strcpy(filename, strtok(NULL, " "));
			if(strcmp(filename, "/favicon.ico") == 0){
				memset(message, '\0', MSG_LEN);
				continue;
			}
			if((strcmp(filename, "http:") == 0) || (strcmp(filename, "https:") == 0)){
				// handling absoluteURI
				strtok(NULL, "/");
				strcpy(filename, strtok(NULL, " "));
				_file = fopen(filename, "rb");
				printf("filename: %s\n", filename);
				printf("------------------------------------------------\n");
			}
			else{
				_file = fopen(filename+sizeof(char), "rb");
				printf("filename: %s\n", filename+sizeof(char));
				printf("------------------------------------------------\n");
			}
			
			if(!(_file)){
				if (send(new_fd, "File not existed.\n", strlen("File not existed.\n"), 0) == -1)
					perror("send");
			}
			else{
				fseek(_file, 0, SEEK_END);
				sprintf(message, "File found\nFile size: %ld\n", ftell(_file));
				if (send(new_fd, message, strlen(message), 0) == -1)
					perror("send");
			}
            memset(message, '\0', MSG_LEN);
        }
        close(new_fd);
    }
	free(message);
    return 0;
}
