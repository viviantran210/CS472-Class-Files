#include "http.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define  BUFF_SZ 1024

char recv_buff[BUFF_SZ];

char *generate_cc_request(const char *host, int port, const char *path){
	static char req[512] = {0};
	int offset = 0;
	
    //note that all paths should start with "/" when passed in
	offset += sprintf((char *)(req + offset),"GET %s HTTP/1.1\r\n", path);
	offset += sprintf((char *)(req + offset),"Host: %s\r\n", host);
	offset += sprintf((char *)(req + offset),"Connection: Close\r\n");
	offset += sprintf((char *)(req + offset),"\r\n");

	printf("DEBUG: %s", req);
	return req;
}


void print_usage(char *exe_name){
    fprintf(stderr, "Usage: %s <hostname> <port> <path...>\n", exe_name);
    fprintf(stderr, "Using default host %s, port %d  and path [\\]\n", DEFAULT_HOST, DEFAULT_PORT); 
}

int process_request(const char *host, uint16_t port, char *resource){
    int sock;
    int total_bytes;

    sock = socket_connect(host, port);
    if(sock < 0) return sock;

    //generate request and send
    char *request = generate_cc_request(host, port, resource);
    int ret = send(sock, request, strlen(request), 0);
    if (ret != strlen(request)) {
	    perror("Error sending request");
	    close(sock);
	    return -1;
    }

    // Loop and receive response data from server
    total_bytes = 0;
    int bytes_recvd = 0;
    while ((bytes_recvd = recv(sock, recv_buff, BUFF_SZ, 0)) > 0) {
	    printf("%.*s", bytes_recvd, recv_buff);
	    total_bytes += bytes_recvd;
    }

    // recv() will return a negative number to indicate error or closing socket
    if (bytes_recvd < 0) {
        perror("Error receiving response or server closed the socket");
    }

    close(sock);
    return total_bytes;
}

int main(int argc, char *argv[]){
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Get start time

    int sock;

    const char *host = DEFAULT_HOST;
    uint16_t   port = DEFAULT_PORT;
    char       *resource = DEFAULT_PATH;
    int        remaining_args = 0;

    // Command line argument processing should be all setup, you should not need
    // to modify this code
    if(argc < 4){
        print_usage(argv[0]);
        //process the default request
        process_request(host, port, resource);
	} else {
        host = argv[1];
        port = atoi(argv[2]);
        resource = argv[3];
        if (port == 0) {
            fprintf(stderr, "NOTE: <port> must be an integer, using default port %d\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
        fprintf(stdout, "Running with host = %s, port = %d\n", host, port);
        remaining_args = argc-3;
        for(int i = 0; i < remaining_args; i++){
            resource = argv[3+i];
            fprintf(stdout, "\n\nProcessing request for %s\n\n", resource);
            process_request(host, port, resource);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // Get end time
    double runtime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("/nExecution time: %.6f seconds\n", runtime);
}
