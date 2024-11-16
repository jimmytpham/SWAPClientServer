/*
*	Jimmy Pham
*	T00629354
*	COMP 3271
*	swap_client.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define	MAXLINE 128	// maximum characters to receive and send at once
#define	MAXFRAME 256


extern unsigned short checksum(unsigned char buf[], int length);

extern int swap_connect(unsigned int addr, unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);

int session_id = 0;
int S = 0;	// frame number sent

int swap_open(unsigned int addr, unsigned short port)
{
	int	sockfd;		// sockect descriptor
	struct	sockaddr_in	servaddr;	// server address
	char	buf[MAXLINE];
	int	len, n;

	/*
	*	if the session is already open, then return error
	*/

	if (session_id != 0)
		return -1;

	/*
	*	connect to a server
	*/

	session_id = swap_connect(addr, port);	// in sdp.o

	/*
	*	return the seesion id
	*/

	return session_id;
}

int swap_write(int sd, char *buf, int length)
{
	int n;
	char frame[MAXFRAME];

	if (session_id == 0 || sd != session_id)
		return -1;

	printf("Client: Sending data...\n");
 	
	// Calculate the length of the frame
    int frame_length = snprintf(NULL, 0, "%d:%s:", S, buf);
    snprintf(frame, MAXFRAME, "%d:%s:", S, buf);

	// Calculate checksum for the data part only
    unsigned short chksum = checksum((unsigned char*)frame, frame_length);

	// Append checksum to frame
    snprintf(frame + frame_length, MAXFRAME - frame_length - 1, "%hu", chksum);

	/*
    *	send a DATA frame
    */
    n = sdp_send(sd, frame, frame_length + sizeof(chksum)+1);
    if (n < 0) {
        printf("Client: Error sending data.\n");
        return -1;
    }
    printf("Client: Data sent successfully.\n");

    // Increment sequence number
    S++;

    /*
    *	return the length sent
    */

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
