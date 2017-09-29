#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>

#include "Ship.h"
#include "Board.h"
#include "Network.h"

using namespace std;

void board_setup(Board& board, long numShips){
	Ship newShip;
	
	cout << "Taichou, This is our advanced technology map of the highest precision, the top left coordinate is (0,0) (y,x)\n";
	board.print();
	cout << "You have " << numShips << " to dispatch.\n";
	string confirmed("n");
	while(confirmed != "y"){
		board.set_board(water);
		for(long i = 0; i < numShips; i++){
			cout << "What will be the coord for the top left (y x) of the ship #" << i << " taichou?\n";
			cin >> newShip.top_left.y >> newShip.top_left.x;
		
			cout << "And how about the height and width of the ship #" << i+1 << " taichou?\n";
			cin >> newShip.height >> newShip.width;
			
			if(board.set_ship(newShip) == FAIL){
				i--;
				cout << "You can't set up a ship like that! Try again but pay more attention this time taichou!\n";
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

void print_game(Board my_board, vector<Board> enemies){
	my_board.print();
	
	for(int i = 0; i < enemies.size(); ++i){
		cout << "Player " << i << endl;
		enemies.at(i).print();
	}
}

void read_attack(Coord& pos){
	cout << "Were will you attack next taichou? (y x)" << endl;
	cin >> pos.y >> pos.x;
}

int main(int argc, char **argv)
{
	//print_ascii("../content/amatsukaze-pc160.txt");
	int enemy_n = 1;
	long board_max_y = 5;
	long board_max_x = 5;
	int numShips = 2;
	
	int my_id = 1;
	char buf[BUFSIZ];
	
	sockaddr_in p_addr;
	
	char msg_to_send[BUFSIZ];
	size_t msg_to_send_size;
	
	Board my_board(Coord{.y = board_max_y, .x = board_max_x});
	Ship ship_hit;
	
	vector<Board> enemies;
	//map<int, Board> enemies; // TODO: necessary to manage ids?
	for (int i = 0; i < enemy_n; ++i){
		enemies.push_back(Board(Coord{.y = board_max_y, .x = board_max_x}, numShips));
	}

	board_setup(my_board, numShips);
	
	print_game(my_board, enemies);
	
	Connection net(1, "i8");
	bool has_response = false;
	bool game_ended = false;
	bool my_turn = false;
	cout << "Are you the first player? 1/0" << endl;
	cin >> my_turn;
	net.with_baton = my_turn;

	if(my_turn){ // Debug
		cout<< "it's your turn" << endl;
	}else{
		cout<< "standby" << endl;
	}
	
	while(!game_ended){
		if(my_turn){
			if(my_board.ship_n > 0){
				coord_msg attack_msg;
				
				read_attack(attack_msg.coord);
				
				attack_msg.info.baton = false;
				attack_msg.info.status = 0;
				attack_msg.info.dest = 1; // TODO: real ids
				attack_msg.info.origin = 1;
				attack_msg.info.content = content_attack;
				
				//msg_queue.push_back(attack_msg);
				net.send_msg(&attack_msg, sizeof(attack_msg));
				
				net.pass_baton();
				
				ship_msg* response_ship;
				coord_msg* response_hit;
				
				net.rec_msg(buf, BUFSIZ);
				
				// process hit
				if(((msg*)buf)->content == content_hit){
					// msg containing hit data
					response_hit = (coord_msg*)buf;
					
					enemies.at(0).at(response_hit->coord).hit = true;	
					if( available(enemies.at(0).at(response_hit->coord)) ){
						enemies.at(0).at(response_hit->coord).idn = unk_ship;
					}
					
				} else if (((msg*)buf)->content ==  content_ship_destroyed){
					cout <<  "destroyed enemy ship" << endl;
					enemies.at(0).set_destroyed_ship(response_ship->ship);
				} else {
					cout <<  "you missed!" << endl;
				}
				enemies.at(0).print();
				
				size_t msg_size = net.prev_player.rec(buf, BUFSIZ, &p_addr);
				if(((msg*)buf)->dest == my_id || ((msg*)buf)->baton){
					net.with_baton = ((msg*)buf)->baton;
				}
			}
			
			net.pass_turn();
		} else {	//not my turn
			if (net.with_baton){
				while(has_response){
					//send_msg(msg_queue.front());
					//msg_queue.pop_front();
					net.send_msg(msg_to_send, msg_to_send_size);
					has_response = false;
				}
				net.pass_baton();
			}else{
				size_t msg_size = net.prev_player.rec(buf, BUFSIZ, &p_addr);

				coord_msg* attack_msg;
			
				// Message for me
				if(((msg*)buf)->dest == my_id || ((msg*)buf)->baton){
					((msg*)buf)->status = status_ok;
					net.with_baton = ((msg*)buf)->baton;
					
					if( ((msg*)buf)->content == content_attack ){
						has_response = true;
						attack_msg = (coord_msg*)buf;
						// calculate hit
						int ship_hp = my_board.attackField(attack_msg->coord, ship_hit);
						
						((coord_msg*)msg_to_send)->info.dest = attack_msg->info.origin;
						((coord_msg*)msg_to_send)->info.origin = my_id;
						if(ship_hp == 0){
//							for(int i=0; i < enemies.size(); i++){
//								ship_destroyed_msg.info.dest = enemies.at(i).player;
//								msg_queue.push_back(ship_destroyed_msg);
//							}
							((ship_msg*)msg_to_send)->info.content = content_ship_destroyed;
							((ship_msg*)msg_to_send)->ship = ship_hit;
						} else {
							msg_to_send_size = sizeof(msg);
							if(ship_hp != FAIL){
								((msg*)msg_to_send)->content = content_hit;
							} else {
								((msg*)msg_to_send)->content = content_miss;
							}
							//msg_queue.push_back(hit_msg);
						}
					} else if (((msg*)buf)->content == content_turn){	//turn
						my_turn = true;
					}
				}
				// send forward non baton msgs
				if( ! (((msg*)buf)->baton) ){
					net.next_player.send(buf, msg_size);
				}
			}
		}
	}
	
	return 0;
}