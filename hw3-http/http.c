#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "http.h"

//---------------------------------------------------------------------------------
// TODO:  Documentation
//
// Note that this module includes a number of helper functions to support this
// assignment.  YOU DO NOT NEED TO MODIFY ANY OF THIS CODE.  What you need to do
// is to appropriately document the socket_connect(), get_http_header_len(), and
// get_http_content_len() functions. 
//
// NOTE:  I am not looking for a line-by-line set of comments.  I am looking for 
//        a comment block at the top of each function that clearly highlights you
//        understanding about how the function works and that you researched the
//        function calls that I used.  You may (and likely should) add additional
//        comments within the function body itself highlighting key aspects of 
//        what is going on.
//
// There is also an optional extra credit activity at the end of this function. If
// you partake, you need to rewrite the body of this function with a more optimal 
// implementation. See the directions for this if you want to take on the extra
// credit. 
//--------------------------------------------------------------------------------

char *strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

char *strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

/**
 * Takes a host and a port as parameters and returns a valid socket
 * descriptor if a connection to that host and port is successful.
 *
 * Returns a socket descriptor if successful, -2 if it cannot get the
 * host by name, and -1 if the socket cannot be created or the
 * connection fails
 */
int socket_connect(const char *host, uint16_t port){
    struct hostent *hp; // stores host info 
    struct sockaddr_in addr; // stores server info
    int sock; // socket descriptor 

    // Resolve the host name to an IP address
    if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		return -2;
	}
    
        // copies first resolved IP address to sockaddr_in structure
	bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port); // converts port number to network byte order
	addr.sin_family = AF_INET;   // sets address family to IPv4
	sock = socket(PF_INET, SOCK_STREAM, 0); // creates a socket 

        // checks if socket creation fails
	if(sock == -1){
		perror("socket");
		return -1;
	}

    // checks if connecting socket to the server fails
    // on failure, close the socket
    if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		close(sock);
        return -1;
	}
    
    // return valid socket descriptor
    return sock;
}

/**
 * Takes a pointer to an HTTP response buffer and its total length and looks for
 * HTTP header end ("\r\n\r\n") if it exists to find HTTP header length
 *
 * Returns the HTTP header length if found or -1 if there is no
 * HTTP header end found.
 */
int get_http_header_len(char *http_buff, int http_buff_len){
    char *end_ptr; // pointer to locate the end of the HTTP header 
    int header_len = 0; // stores the HTTP header length
    end_ptr = strnstr(http_buff,HTTP_HEADER_END,http_buff_len); // search for end of the HTTP header

    // check if HTTP header end is found, and return -1 if not
    if (end_ptr == NULL) { 
        fprintf(stderr, "Could not find the end of the HTTP header\n");
        return -1;
    }

    // calculate HTTP header length based on end pointer and start pointer
    // adds length of HTTP header end as well
    header_len = (end_ptr - http_buff) + strlen(HTTP_HEADER_END);

    return header_len;
}

/** 
 * Takes a pointer to an HTTP response buffer and the HTTP header length
 * and looks for HTTP content length from the Content-Length header if it 
 * exists in the header
 *
 *
 * Returns HTTP content length if found, else return 0
 */
int get_http_content_len(char *http_buff, int http_header_len){
    char header_line[MAX_HEADER_LINE]; // buffer to hold header lines 

    char *next_header_line = http_buff; // pointer used to traverse header lines
    char *end_header_buff = http_buff + http_header_len; // pointer for the HTTP header end

    // loop through all header lines in buffer
    while (next_header_line < end_header_buff){
        bzero(header_line,sizeof(header_line)); // clear header_line buffer before reading a new line
        sscanf(next_header_line,"%[^\r\n]s", header_line); // read next header line until newline is found

	// if Content-Length and : are found in header line, move past it to extract the length value, convert it to an int and return it
        char *isCLHeader2 = strcasecmp(header_line,CL_HEADER); // case insensitive comparison of header line with "Content-Length" (CL_HEADER)
        char *isCLHeader = strcasestr(header_line,CL_HEADER); // case insensitive search for CL_HEADER
        if(isCLHeader != NULL){
            char *header_value_start = strchr(header_line, HTTP_HEADER_DELIM); // find ":" (HTTP_HEADER_DELIM)
            if (header_value_start != NULL){
                char *header_value = header_value_start + 1; // move past delimiter to value
                int content_len = atoi(header_value); // convert value to an int
                return content_len;
            }
        }
        next_header_line += strlen(header_line) + strlen(HTTP_HEADER_EOL); // move to next header line if 'Content-Length' is not found
    }
    fprintf(stderr,"Did not find content length\n"); // if no Content-Length header is found
    return 0;
}

//This function just prints the header, it might be helpful for your debugging
//You dont need to document this or do anything with it, its self explanitory. :-)
void print_header(char *http_buff, int http_header_len){
    fprintf(stdout, "%.*s\n",http_header_len,http_buff);
}

//--------------------------------------------------------------------------------------
//EXTRA CREDIT - 10 pts - READ BELOW
//
// Implement a function that processes the header in one pass to figure out BOTH the
// header length and the content length.  I provided an implementation below just to 
// highlight what I DONT WANT, in that we are making 2 passes over the buffer to determine
// the header and content length.
//
// To get extra credit, you must process the buffer ONCE getting both the header and content
// length.  Note that you are also free to change the function signature, or use the one I have
// that is passing both of the values back via pointers.  If you change the interface dont forget
// to change the signature in the http.h header file :-).  You also need to update client-ka.c to 
// use this function to get full extra credit. 
//--------------------------------------------------------------------------------------
int process_http_header(char *http_buff, int http_buff_len, int *header_len, int *content_len){
    int h_len, c_len = 0;
    h_len = get_http_header_len(http_buff, http_buff_len);
    if (h_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }
    c_len = get_http_content_len(http_buff, http_buff_len);
    if (c_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }

    *header_len = h_len;
    *content_len = c_len;
    return 0; //success
}
