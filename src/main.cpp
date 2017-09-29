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
	
	for (int i = 0; i < enemies.size(); ++i)
		cout << "Player " << i << endl;
		enemies.at(i).print();
}

void pass_turn(){
	next_player.send(msg {.baton = true, .content = content_turn});
}

void read_attack(Coord& pos){
	cout << "Were will you attack next taichou? (y x)";
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
	
	char msg_to_send[BUFSIZ];
	size_t msg_to_send_size;
	
	queue tegami_queue;
	
	Board my_board(Coord{.y = board_max_y, .x = board_max_x});
	Ship ship_hit;
	
	vector<Board> enemies;
	//map<int, Board> enemies;
	for (int i = 0; i < enemy_n; ++i){
		enemies.push_back(Coord{.y = board_max_y, .x = board_max_x}, numShips);
	}

	board_setup(my_board, numShips);
	
	print_game(my_board, enemies);
	
	SSocket next_player("h3");
	RSocket prev_player();
	
	while(!game_ended){
		if(my_turn){
			if(ship_n > 0){
				sockaddr_in p_addr;
				coord_msg attack_msg;
				
				read_attack(attack_msg.coord);
				
				attack_msg.info.baton = false;
				attack_msg.info.status = 0;
				attack_msg.info.dest = 1; // TODO: real ids
				attack_msg.info.origin = 1;
				attack_msg.info.content = content_attack;
				
				//msg_queue.push_back(attack_msg);
				send_msg(&attack_msg, sizeof(attack_msg));
				
				pass_baton();
				
				//wait_attack_response();
				ship_msg* response_ship;
				coord_msg* response_hit;
				
				rec_msg(buf, BUFSIZ);
				
				//process_attack();
				if(((msg*)buf)->info.content == content_hit){
					response_hit = buf;
					
					enemies.at(0).at(response_hit->coord).hit = true;
					if( available(enemies.at(0).at(response_hit->coord).idn) ){
						enemies.at(0).at(response_hit->coord).idn = unk_ship;
					}
					
				} else if (buf.info.content ==  content_ship_destroyed){
					cout <<  "destroyed enemy ship" << endl;
					enemies.at(0).set_destroyed_ship(response_ship->ship);
				}
				enemies.at(0).print();
			}
			
			pass_turn();
		} else {
			if (with_baton){
				while(has_response){
					//send_msg(msg_queue.front());
					//msg_queue.pop_front();
					send_msg(msg_to_send, msg_to_send_size);
					has_response = false;
				}
				pass_baton();
			}else{
				msg_size = prev_player.rec(buf, BUFSIZ, &p_addr);

				coord_msg* attack_msg;
				coord_msg* attack_msg;
				
				// Message for me
				if(((msg*)buf)->info.dest == my_id || (msg*)buf)->info.baton){
					(msg*)buf)->info.status = status_ok;
					with_baton = (msg*)buf)->info.baton;
					
					if((msg*)buf)->info.content == content_attack){
						attack_msg = buf;
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
								((msg*)msg_to_send)->info.content = content_hit;
							} else {
								((msg*)msg_to_send)->info.content = content_miss;
							}
							//msg_queue.push_back(hit_msg);
						}
					} else if (((msg*)buf)->info.content == content_turn){
						my_turn = true;
					}
				}
				// send forward non baton msgs
				if(!(msg*)buf)->info.baton){
					next_player.send(buf, msg_size);
					send_msg(Tegami);
				}
			}
			
		}
	}
	
	return 0;
}