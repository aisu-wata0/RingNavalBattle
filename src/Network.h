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

#define content_attack 1
#define content_turn 2
#define content_ship_destroyed 3
#define content_hit 4
#define content_miss 5

#define status_ok 3

namespace std{

enum msg_type {msg_baton, msg_turn};
	
class msg {
public:
	int baton:1;
	int status:2;
	int dest;
	int origin;
	int content;
	
	msg(){}
	
	msg(msg_type type){
		if(type == msg_baton){
			baton = true;
			content = 0;
		}
		if(type == msg_turn){
			baton = true;
			content = content_turn;
		}
	}
	
	void print(){
		cout << "baton:1;" << baton << endl;
		cout << "status:2;" << status << endl;
		cout << "dest;" << dest << endl;
		cout << "origin;" << origin << endl;
		cout << "content;" << content << endl;
	}
};

class coord_msg {
public:
	msg info;
	Coord coord;
};

class ship_msg {
public:
	msg info;
	Ship ship;
};

msg turn_msg(msg_turn);

msg baton(msg_baton);

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
		clog <<"recvd "<< byte_count <<"bytes ";
		clog <<"from IP: "<< ipstr << endl;
		
		return byte_count;
	}
};

class SSocket {
public:
	sockaddr_in addr_listener;
	int sock;
	sockaddr_storage addr_dest;
	
	SSocket(string hostname) {
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
		clog <<"sent "<< size <<"bytes of data ";
		clog <<"to IP: "<< ipstr <<endl;
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
		with_baton = false;
	}
	
	void send_msg(void* Tegami, size_t size){
		char buf[BUFSIZ];
		msg* response;
		sockaddr_in addr;
		clog << "started trying to send to player " << response->dest << endl;
		do {
			next_player.send(&Tegami, size);
			prev_player.rec(buf, BUFSIZ, &addr);
			response = (msg*)buf;
			response->print();
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
			((msg*)Tegami)->print();
			clog << "tegami confirmed will send" << endl;
			next_player.send(Tegami, msg_size);
		} while (msg_size == 0);
	}
	
	void pass_turn(){
		clog << "passing turn" << endl;
		next_player.send(&turn_msg, sizeof(turn_msg));
	}
	
	void pass_baton(){
		clog << "passing baton" << endl;
		next_player.send(&baton, sizeof(baton));
	}
};



}// namespace std

#endif
