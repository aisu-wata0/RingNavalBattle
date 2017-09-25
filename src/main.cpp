#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Ship.h"
#include "Board.h"
#include "Network.h"

using namespace std;

void board_setup(Board& board){
	Ship newShip;
	long numShips;
	
	cout << "Taishou, how many ships do you want to dispatch?\n";
	cin >> numShips;
	
	cout << "This is our advanced technology map of the highest precision, the top left coordinate is (0,0) (y,x)\n";
	board.print();
	string confirmed("n");
	while(confirmed != "y"){
		board.set_board(water);
		for(long i = 0; i < numShips; i++){
			cout << "What will be the coord for the top left (y, x) of the ship #" << i << " Taishou?\n";
			cin >> newShip.top_left.y >> newShip.top_left.x;
		
			cout << "And how about the height and width of the ship #" << i+1 << " Taishou?\n";
			cin >> newShip.height >> newShip.width;
			
			if(board.set_ship(newShip) == FAIL){
				i--;
				cout << "You can't set up a ship like that! Try again but pay more attention this time Taishou!\n";
			}
			board.print();
		}
		
		cout << "It's not like I like how you set up the board or anything, baka\n";
		cout << "But do you confirm it? (y/n)" << endl;
		cin >> confirmed;
		confirmed.at(0) = tolower(confirmed.at(0));
	}
}

void print_ascii(string filename){
	ifstream in_f;
	in_f.open(filename);
	string buffer;
	while(getline(in_f, buffer)){
		cout << buffer <<"\n";
	}
	in_f.close();
}

//int main(int argc, char **argv)
//{
//	//print_ascii("../content/amatsukaze-pc160.txt");
//	
//	Board my_board(Coord{.y = 5, .x = 5}, MINE);
//	Ship target;
//
//	board_setup(my_board);
//	my_board.print();
//	
//	Coord att;
//	att.x = 0;
//	
//	for(long att_coor=0; att_coor <= 2; att_coor++){
//		for(long att_cory=0; att_cory <= 2; att_cory++){
//			att.y = att_cory;
//			att.x = att_coor;
//			cout<<"\n attack "<< att.y <<","<< att.x <<" return: "<< my_board.attackField(att, target) <<"\n";
//			my_board.print();
//			cout<<"\n\n";
//		}
//	}
//	
//	return 0;
//}

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

void read_from_client(){
	int result = 0;
	
//	sockaddr_in addr_dest = {};
//	addr_dest.sin_family = AF_INET;
//	addr_dest.sin_port = PORT;	
	//inet_pton(AF_INET, "192.168.100.52", &(addr_dest.sin_addr));
	
	sockaddr_storage addr_dest = {};
	result = resolvehelper("192.168.100.52", AF_INET, PORT_S, &addr_dest);
	if (result == -1){
		int lasterror = errno;
		cout << "error: " << lasterror;
		exit(1);
	}

	sockaddr_in addr_listener = {};
	addr_listener.sin_family = AF_INET;
	addr_listener.sin_port = PORT;

	// get socket
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	// bind listener
	result = bind(sock, (sockaddr*)&addr_listener, sizeof(addr_listener));
	if (result == -1){
		int lasterror = errno;
		cout << "error bind: " << lasterror;
		exit(1);
	}

	// connect listener to dest
//	result = connect(sock, (sockaddr*)&addr_dest, sizeof(addr_dest));
//	if (result == -1){
//		int lasterror = errno;
//		cout << "error connect: " << lasterror;
//		exit(1);
//	}
	
	char buf[1024];
	
	while(1){
		//int bytes = read(sock, buf, sizeof(buf));
		unsigned slen = sizeof(sockaddr);
		int bytes = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&addr_dest, &slen);
		cout << "received " << bytes << "bytes:\n";
		cout << buf << endl;
	}
	
	close(sock);
}

void server_recvfrom(){
	// datagram sockets and recvfrom()

	struct addrinfo hints, *res;
	int sockfd;
	int byte_count;
	socklen_t fromlen;
	char buf[512];
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
	cout << buf <<endl;
	
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
	result = resolvehelper("192.168.100.8", AF_INET, PORT_S, &addr_dest);
	if (result != 0){
		int lasterror = errno;
		cout << "error: " << lasterror;
		exit(1);
	}

	const char* msg = "Tegami";
	size_t msg_length = strlen(msg);

	result = sendto(sock, msg, msg_length, 0, (sockaddr*)&addr_dest, sizeof(addr_dest));

	cout << result << " bytes sent" << endl;
}


int main(int argc, char **argv)
{
	server_recvfrom();
}