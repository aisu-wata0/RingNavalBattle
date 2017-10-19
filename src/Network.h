#ifndef NETWORK_H
#define NETWORK_H

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
#include <stdio.h>
#include <errno.h>

#include "Ship.h"

#define PORT 9635
#define PORT_S "9635"

#define content_id 1
#define content_start 2
#define content_miss 3
#define content_attack 4
#define content_ship_destroyed 5
#define content_turn 7
#define content_hit 8


#define status_ok 3

namespace std{

enum msg_type {nil, msg_baton, msg_turn};
	
typedef struct {
	int8_t baton;
	int8_t status;
	int8_t dest;
	int8_t origin;
	int8_t content;
} msg;

void print(msg tegami){
	clog
	<< "baton: " << (int)tegami.baton
	<< ";\tstatus: " << (int)tegami.status
	<< ";\tdest: " << (int)tegami.dest
	<< ";\torigin: " << (int)tegami.origin
	<< ";\tcontent: " << (int)tegami.content << endl;
}

typedef struct {
	msg info;
	int my_id;
} id_msg;

typedef struct {
	msg info;
	Coord coord;
} coord_msg;

typedef struct {
	msg info;
	Ship ship;
} ship_msg;

union msg_buffer {
	msg info;
	id_msg id_info;
	coord_msg coord_info;
	ship_msg ship_info;
};

msg new_msg(msg_type type){
	msg tegami;
	
	tegami.baton = 0;
	tegami.status = 0;
	tegami.dest = 0;
	tegami.origin = 0;
	tegami.content = 0;
	
	if(type == msg_baton){
		tegami.baton = true;
	}
	if(type == msg_turn){
		tegami.baton = true;
		tegami.content = content_turn;
	}
	return tegami;
}

msg turn_msg = new_msg(msg_turn);

msg baton = new_msg(msg_baton);

msg nil_msg = new_msg(nil);

char ipstr[INET6_ADDRSTRLEN];

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

class RSocket {
public:
	struct addrinfo hints, *res;
	int sockfd;
	
	RSocket() {
		int result = 0;
		// get host info, make socket, bind it to port PORT
		hints = {};
		hints.ai_family = AF_INET;  // use IPv4
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;
		
		getaddrinfo(NULL, PORT_S, &hints, &res);
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		result = bind(sockfd, res->ai_addr, res->ai_addrlen);
		if (result == -1){
			int lasterror = errno;
			cerr << "error: " << lasterror;
		}
	}
	
	int rec(msg_buffer* buf_p, sockaddr_in* p_addr){
		socklen_t fromlen = sizeof(*p_addr);
		
		int byte_count = recvfrom(sockfd, buf_p, sizeof(msg_buffer), 0, (sockaddr*)p_addr, &fromlen);
		
		inet_ntop(AF_INET, &(p_addr->sin_addr), ipstr, INET6_ADDRSTRLEN);
		clog <<"recvd "<< byte_count <<" bytes ";
		clog <<"from IP: "<< ipstr << endl;
		print(buf_p->info);
		
		return byte_count;
	}
	
	int rec_packet(msg_buffer* buf_p, sockaddr_in* p_addr, int timeout_sec){
		socklen_t fromlen = sizeof(*p_addr);
		int byte_count = 0;
		struct timeval tv;
		if(timeout_sec > 0){
			tv.tv_sec =	timeout_sec;
			tv.tv_usec = 0;
			if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval)) < 0)
				fprintf(stderr, "setsockopt to set timeout failed, errno: %d\n", errno);
		}
		
		byte_count = recvfrom(sockfd, buf_p, sizeof(msg_buffer), 0, (sockaddr*)p_addr, &fromlen);
		// receive a network packet and copy in to buffer
		
		if(timeout_sec > 0){
			tv.tv_sec = 0; tv.tv_usec = 0;
			if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) < 0)
				fprintf(stderr, "setsockopt to unset timeout failed, errno: %d\n", errno);
		}
		
		if(byte_count > 0){
			inet_ntop(AF_INET, &(p_addr->sin_addr), ipstr, INET6_ADDRSTRLEN);
			clog <<"recvd "<< byte_count <<" bytes ";
			clog <<"from IP: "<< ipstr << endl;
			print(buf_p->info);			
		}

		return byte_count;
	}
};

class SSocket {
public:
	sockaddr_in addr_listener;
	int sock;
	sockaddr_storage addr_dest;
	string hostname;
	
	SSocket(string hostname) {
		this->hostname = hostname;
		int result = 0;
		sock = socket(AF_INET, SOCK_DGRAM, 0);
	
		addr_listener = {};
		addr_listener.sin_family = AF_INET;
		addr_listener.sin_port = PORT;
		
		result = bind(sock, (sockaddr*)&addr_listener, sizeof(addr_listener));
		if (result == -1){
			int lasterror = errno;
			cerr << "error: " << lasterror;
		}

		addr_dest = {};
		result = resolvehelper(hostname.c_str(), AF_INET, PORT_S, &addr_dest);
		if (result != 0){
			int lasterror = errno;
			cerr << "error: " << lasterror;
		}
	}
	
	int send(msg_buffer* buf_p, size_t size){
		sockaddr_in addr = *((sockaddr_in*)&addr_dest);
		inet_ntop(AF_INET, &(addr.sin_addr), ipstr, INET6_ADDRSTRLEN);
		
		clog <<"sent "<< size <<" bytes of data ";
		clog <<"to IP: "<< ipstr <<endl;
		print(buf_p->info);

		return sendto(sock, buf_p, size, 0, (sockaddr*)&addr_dest, sizeof(addr_dest));
	}
};

class Connection {
public:
	SSocket next_player;
	RSocket prev_player;
	int my_id;
	bool with_baton;
	
	Connection(int id, string next_hostname): 
		next_player(next_hostname),
		prev_player() {
		my_id = id;
		with_baton = (my_id == 1);
	}
	
	Connection(string next_hostname): 
		next_player(next_hostname),
		prev_player() {
	}
	
	void send_msg(msg_buffer* tegami_p, size_t size){
		msg_buffer buf;
		sockaddr_in addr;
		clog << "started trying to send to player " << next_player.hostname << endl;
		do {
			next_player.send(tegami_p, size);
			prev_player.rec(&buf, &addr);
		} while(buf.info.status != status_ok || buf.info.origin != my_id);
		clog << "p" << (int)buf.info.dest << " confirmed msg" << endl;
	}
	
	msg_buffer att_msg(Coord pos, int8_t targetID){
		msg_buffer tegami;
		tegami.info = nil_msg;
		tegami.coord_info.coord = pos;
		tegami.info.baton = false;
		tegami.info.status = 0;
		tegami.info.origin = my_id;
		tegami.info.dest = targetID;
		tegami.info.content = content_attack;
		return tegami;
	}
	
	void response_msg(msg_buffer* tegami, int dest){
		tegami->info = nil_msg;
		tegami->info.dest = dest;
		tegami->info.origin = my_id;
	}
	
	void rec_msg(msg_buffer* tegami_p){
		size_t msg_size = 0;
		sockaddr_in addr;
		do { 
			msg_size = prev_player.rec(tegami_p, &addr);
			clog << "received tegami_p" << endl;
			// TODO: only if my message
			
			tegami_p->info.status = status_ok;
			clog << "confirmed it! sending back" << endl;
			
			next_player.send(tegami_p, msg_size);
		} while (msg_size == 0);
	}
	
	void pass_turn(bool& my_turn){
		clog << "passing turn" << endl;
		my_turn = false;
		with_baton = false;
		next_player.send((msg_buffer*)&turn_msg, sizeof(msg));
	}
	
	void pass_baton(){
		clog << "passing baton" << endl;
		with_baton = false;
		next_player.send((msg_buffer*)&baton, sizeof(msg));
	}
	
	bool is_this_for_me(msg_buffer tegami){
		with_baton = tegami.info.baton;
		if(tegami.info.baton){
			cout<< "Received baton\n";
			return true;
		}
		return (tegami.info.dest == my_id) || tegami.info.baton;
	}
};



}// namespace std

#endif
