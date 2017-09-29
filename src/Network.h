#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>

#define PORT 9635
#define PORT_S "9635"

#define content_attack 1
#define content_hit 2
#define content_ship_destroyed 3

typedef struct {
	int baton:1;
	int status:2;
	int dest;
	int origin;
	int content;
} msg; // TODO

typedef struct {
	msg info;
	Coord coord;
} coord_msg;

typedef struct {
	msg info;
	Ship ship;
} ship_msg;

int resolvehelper(const char* hostname, int family, const char* service, sockaddr_storage* pAddr)
{
	int result;
	addrinfo* result_list = NULL;
	addrinfo hints = {};
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	result = getaddrinfo(hostname, service, &hints, &result_list);
	if (result == 0)
	{
		memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
		freeaddrinfo(result_list);
	}

	return result;
}

void server_recvfrom(){
	// datagram sockets and recvfrom()

	struct addrinfo hints, *res;
	int sockfd;
	int byte_count;
	socklen_t fromlen;
	Coord buf[512];
	char ipstr[INET6_ADDRSTRLEN];

	// get host info, make socket, bind it to port PORT
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  // use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, PORT_S, &hints, &res);
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	bind(sockfd, res->ai_addr, res->ai_addrlen);
	
	// no need to accept(), just recvfrom():
	sockaddr_in addr;
	fromlen = sizeof addr;
	while (1){
		byte_count = recvfrom(sockfd, buf, sizeof buf, 0, (sockaddr*)&addr, &fromlen);

		inet_ntop(AF_INET, &(addr.sin_addr), ipstr, INET6_ADDRSTRLEN);
		printf("recv()'d %d bytes of data in buf\n", byte_count);
		printf("from IP address %s\n", ipstr);
		cout << buf[0].y << buf[0].x <<endl;
	}
}

void sendto_server(){
	int result = 0;
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in addr_listener = {};
	addr_listener.sin_family = AF_INET;
	addr_listener.sin_port = PORT;
	
	result = bind(sock, (sockaddr*)&addr_listener, sizeof(addr_listener));
	if (result == -1){
		int lasterror = errno;
		cout << "error: " << lasterror;
		exit(1);
	}

	sockaddr_storage addr_dest = {};
	result = resolvehelper("10.254.225.10", AF_INET, PORT_S, &addr_dest);
	if (result != 0){
		int lasterror = errno;
		cout << "error: " << lasterror;
		exit(1);
	}
	
	Coord c;
	c.y = 2;
	c.x = 3;
	
	while(1){
		result = sendto(sock, msg, sizeof(Coord), 0, (sockaddr*)&addr_dest, sizeof(addr_dest));
		cout << ".";
		cout.flush();
		sleep(1);
	}
	cout << result << " bytes sent" << endl;
}