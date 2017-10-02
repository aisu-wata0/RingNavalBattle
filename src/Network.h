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

#include "Ship.h"

#define PORT 9635
#define PORT_S "9635"

#define content_miss 3
#define content_attack 4
#define content_ship_destroyed 5
#define content_turn 7
#define content_hit 8


#define status_ok 3

namespace std{

enum msg_type {msg_baton, msg_turn};
	
typedef struct {
	int8_t baton;
	int8_t status;
	int8_t dest;
	int8_t origin;
	int8_t content;	
} msg;

msg new_msg(msg_type type){
	msg Tegami;
	if(type == msg_baton){
		Tegami.baton = true;
		Tegami.content = 0;
	}
	if(type == msg_turn){
		Tegami.baton = true;
		Tegami.content = content_turn;
	}
	return Tegami;
}

void print(msg* Tegami){
	clog
	<< "baton: " << (int)Tegami->baton
	<< ";\tstatus: " << (int)Tegami->status
	<< ";\tdest: " << (int)Tegami->dest
	<< ";\torigin: " << (int)Tegami->origin
	<< ";\tcontent: " << (int)Tegami->content << endl;
}

typedef struct {
	msg info;
	Coord coord;
} coord_msg;

typedef struct {
	msg info;
	Ship ship;
} ship_msg;

msg turn_msg = new_msg(msg_turn);

msg baton =  new_msg(msg_baton);

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
	
	int rec(void* buf, size_t size, sockaddr_in* p_addr){
		socklen_t fromlen;
		fromlen = sizeof(*p_addr);
		int byte_count = recvfrom(sockfd, buf, size, 0, (sockaddr*)p_addr, &fromlen);
		
		inet_ntop(AF_INET, &(p_addr->sin_addr), ipstr, INET6_ADDRSTRLEN);
		clog <<"recvd "<< byte_count <<" bytes ";
		clog <<"from IP: "<< ipstr << endl;
		
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
	
	int send(void* buf, size_t size){
		sockaddr_in addr = *((sockaddr_in*)&addr_dest);
		inet_ntop(AF_INET, &(addr.sin_addr), ipstr, INET6_ADDRSTRLEN);
		
		clog <<"sent "<< size <<" bytes of data ";
		clog <<"to IP: "<< ipstr <<endl; 
		print((msg*)buf);
		
		return sendto(sock, buf, size, 0, (sockaddr*)&addr_dest, sizeof(addr_dest));
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
	
	void send_msg(void* Tegami, size_t size){
		char buf[BUFSIZ];
		msg* response;
		sockaddr_in addr;
		clog << "started trying to send to player " << next_player.hostname << endl;
		do {
			next_player.send(Tegami, size);
			prev_player.rec(buf, BUFSIZ, &addr);
			response = (msg*)buf;
			print(response);
		} while(response->status != status_ok || response->origin != my_id);
		clog << "player " << response->dest << "received msg" << endl;
	}
	
	void rec_msg(void* Tegami, size_t buf_size){
		size_t msg_size = 0;
		sockaddr_in addr;
		do { 
			msg_size = prev_player.rec(Tegami, buf_size, &addr);
			clog << "received tegami" << endl;
			// TODO: only if my message
			
			((msg*)Tegami)->status = status_ok;
			print((msg*)Tegami);
			clog << "tegami confirmed will send" << endl;
			next_player.send(Tegami, msg_size);
		} while (msg_size == 0);
	}
	
	void pass_turn(bool& my_turn){
		clog << "passing turn" << endl;
		my_turn = false;
		next_player.send(&turn_msg, sizeof(turn_msg));
	}
	
	void pass_baton(){
		clog << "passing baton" << endl;
		with_baton = false;
		next_player.send(&baton, sizeof(baton));
	}
	
	bool is_this_for_me(msg* tegami){
		return (tegami->dest == my_id) || tegami->baton;
	}
};



}// namespace std

#endif
