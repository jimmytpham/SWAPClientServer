/*
*	Jimmy Pham
*	T00629354
*	COMP 3271
*	swap_server.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define	MAXLINE	128	// maximum characters to receive and send at once
#define MAXFRAME 256


extern unsigned short checksum(unsigned char buf[], int length);
extern int swap_accept(unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);

int session_id = 0;
int R = 0;	// frame number to receive

int swap_wait(unsigned short port)
{
	/*
	*	if the session is already open, then return error
	*/

	if (session_id != 0)
		return -1;

	/*
	*	accept a connection
	*/

	session_id = swap_accept(port);	// in sdp.o

	/*
	*	return a ssession id
	*/

	return session_id;
}

int swap_read(int sd, char *buf) {
    int n;
    char frame[MAXFRAME];

    if (session_id == 0 || sd != session_id)
        return -1;

    printf("Server: Waiting for data...\n");

    /*
	*	receive a frame without a timer
	*/
    n = sdp_receive(sd, frame);
    if (n < 0) {
        printf("Server: Error receiving data.\n");
        return -1;
    }
    printf("Server: Data received.\n");

    char *token = strtok(frame, ":");
    if (token == NULL) {
        printf("Server: Error parsing data.\n");
        return -1;
    }
    int received_frame_number = atoi(token);

    token = strtok(NULL, ":");
    if (token == NULL) {
        printf("Server: Error parsing data.\n");
        return -1;
    }
    char *received_data = token;
    token = strtok(NULL, ":");
    if (token == NULL) {
        printf("Server: Error parsing data.\n");
        return -1;
    }
    
   unsigned short received_chksum = (unsigned short)atoi(token);

    // Calculate checksum for the received frame
    unsigned short expected_chksum = checksum((unsigned char *)frame, strlen(frame));

    if (received_chksum != expected_chksum) {
        printf("Server: Error - Checksum mismatch in received data.\n");
        return -1;
    }

    printf("Server: Data checksum verified.\n");

    sprintf(frame, "ACK");

    n = sdp_send(sd, frame, strlen(frame));
    if (n < 0) {
        printf("Server: Error sending ACK.\n");
        return -1;
    }

    printf("Server: ACK sent.\n");

    /*
	*	copy the data field in the frame into buf, and return the length
	*/
    strcpy(buf, received_data);

    return strlen(buf);
}


void swap_close(int sd)
{
	if (session_id == 0 || sd != session_id)
		return;

	else
		session_id = 0;

	swap_disconnect(sd);	// in sdp.o
}
