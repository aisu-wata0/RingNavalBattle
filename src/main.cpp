#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <cstdint>
#include <ios>
#include <algorithm>

#include "Ship.h"
#include "Board.h"
#include "Network.h"

#define DEBUG false
using namespace std;

bool id_in_range(int target, int enemy_n, int my_id){
	if(target == my_id || target > enemy_n + 1 || target < 1) return false;
	return true;
}

bool pos_in_range(Coord pos, long max_y, long max_x){
	if(pos.x < 0 || pos.x > max_x) return false;
	if(pos.y < 0 || pos.y > max_y) return false;
	return true;
}

int readInteger(){
	int int_num;
	
	while(not(cin >> int_num)){
		cout << "Not a valid number" << endl;
		cin.clear();
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return int_num;
}

void board_setup(Board& board, long numShips, int ID){
	Ship newShip;
	
	cout << "Taichou, you are player " << ID << " and this is our advanced technology map of the highest precision, the top left coordinate is (0,0) (y,x)\n";
	board.print();
	cout << "You have " << numShips << " to dispatch.\n";
	string confirmed("n");
	while(confirmed != "y"){
		board.set_board(water);
		for(long i = 0; i < numShips; i++){
			cout << "What will be the coord for the top left (y x) of the ship #" << i << " taichou?\n";
			
			newShip.top_left.y = readInteger();
			newShip.top_left.x = readInteger();
		
			cout << "And how about the height and width of the ship #" << i << " taichou?\n";
			newShip.height = readInteger();
			newShip.width = readInteger();
			
			if(board.set_ship(newShip) == FAIL){
				i--;
				cout << "You can't set up a ship like that! Try again but pay more attention this time taichou!\n";
			}
			
			board.print();
			
		}
		
		confirmed = "";
		cout << "It's not like I like how you set up the board or anything, baka\n";
		cout << "But do you confirm it? (y/n)" << endl;
		cin >> confirmed;
		confirmed.at(0) = tolower(confirmed.at(0));

		while(confirmed != "y" && confirmed != "n"){	//wrong input handler
			confirmed = "";
			cout << "Answer correctly baka! Do you confirm it? (y/n)" << endl;
			cin >> confirmed;
			confirmed.at(0) = tolower(confirmed.at(0));
		}
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

void print_game(vector<Board> enemies){
	for(size_t i = 0; i < enemies.size(); ++i){
		cout << "Player " << i + 1 << endl;
		enemies.at(i).print();
	}
	cout << endl;
}

void readAttack(Coord& pos, int8_t& dest, int enemy_n, int my_id, Coord max){
	int d = 0;
	pos.y = 0;
	pos.x = 0;
	bool valid = false;
	
	cout << "Which player will you attack?" << endl;
	
	while(!valid){
		d = readInteger();
		if(id_in_range(d, enemy_n, my_id)) valid = true;
		else cout << "Baka, enter a valid player." << endl;
	}
	
	dest = d;
	valid = false;
	
	cout << "Where will you attack next taichou? (y x)" << endl;
	
	while(!valid){
		pos.y = readInteger();
		pos.x = readInteger();
		if (pos_in_range(pos, max.y, max.x)){
			valid = true;
		} else {
			cout << "Baka! Please enter a valid position to attack." << endl;
		}
	}
}

int main(int argc, char **argv)
{
	//print_ascii("content/amatsukaze.txt");
	
	if(!DEBUG){
		clog.rdbuf(NULL);
	}
	
	int enemy_n = 0;
	int maxID = 0;
	long board_max_y = 5;
	long board_max_x = 5;
	int numShips = 2;
	
	int c , wID = 0;
	int my_id = 0;
	string next_hostname = "";
	
	while (( c = getopt(argc, argv, "fh:")) != -1){
		switch (c){
			case 'f':
				my_id = 1;
				break;
			case 'h':
				next_hostname = optarg;
				break;
			case ':':
			// missing option argument
				fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
			default:
				fprintf(stderr, "Usage: %s [-f first_player] -h next_hostname\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	if(next_hostname == ""){
		cout << "-h is mandatory\n";
		exit(1);
	}
	
	msg_buffer buf, start_msg;
	
	sockaddr_in p_addr;

	bool ready = false;
	bool set = false;
	
	// Setup ids
	Connection net(next_hostname);
	
	if(my_id == 1){
		net.my_id = 1;
		start_msg.id_info.my_id = 1;
	}
	
	while(!ready){
		if(net.my_id == 1){
			if(!set){ // Ring not complete, still sending id setup msg
				start_msg.info.content = content_id_setup;
				net.next_player.send(&start_msg, sizeof(start_msg));
				
				// wait for message to come back, with timeout
				int msg_size = net.prev_player.rec_packet(&buf, &p_addr, TIMEOUT);
				if(msg_size < 1){ // timed out
					set = false;
				} else { // received message back, ring is complete.
					// ids set, need to start game
					set = true;
					enemy_n = buf.id_info.my_id - 1;
					maxID = enemy_n + 1;
				}
			} else { // Ring complete, every player has their id
				// send the number of players in the network in the start msg
				start_msg.info.content = content_start;
				start_msg.info.status = enemy_n;
				
				net.next_player.send(&start_msg, sizeof(start_msg));
				// with timeout
				size_t msg_size = net.prev_player.rec_packet(&buf, &p_addr, TIMEOUT);
				if(msg_size < 1){ // timed out
					ready = false;
				} else {
					ready = true;
				}
			}
		} else {
			size_t msg_size = net.prev_player.rec(&buf, &p_addr);
			
			if(buf.info.content == content_id_setup){
				buf.id_info.my_id++;
				net.my_id = buf.id_info.my_id;
			} else { // if(buf.info.content == content_start)
				ready = true;
				enemy_n = buf.info.status;
				maxID = enemy_n + 1;
			}
			net.next_player.send(&buf, msg_size);
		}
	}
	
	
	msg_buffer msg_to_send;
	size_t msg_to_send_size;

	msg_to_send_size = sizeof(msg_buffer);
	
	Ship ship_hit;
	
	vector<Board> enemies;
	
	//treat ids for multiplayer, player2[0] == player1 player2[1] == player[3]
	for (int i = 0; i < enemy_n + 1; ++i){
		if(net.my_id != i+1){
			enemies.push_back(Board({board_max_y,board_max_x}, numShips));
		} else {
			enemies.push_back(Board({board_max_y,board_max_x}));
		}
	}
			
	board_setup(enemies.at(net.my_id-1), numShips, net.my_id);
	
	print_game(enemies);
	
	bool has_response = false;
	bool game_ended = false;
	bool my_turn;

	if(net.my_id == 1){
		my_turn = true;
		cout<< "it's your turn" << endl;
	}else{
		my_turn = false;
		//cout << "standby" << endl;
	}
	
	while(!game_ended || has_response){
		if(my_turn){
			if(enemies.at(net.my_id-1).alive()){
				cout << "It's now our turn!" << endl;
				
				msg_buffer attack_msg;
				Coord pos;
				int8_t target;
				
				readAttack(pos, target, maxID, net.my_id, Coord{board_max_y, board_max_x});
				
				// set up msg info
				attack_msg = net.att_msg(pos, target);

				net.send_msg(&attack_msg, sizeof(coord_msg));	//send msg forward
				
				net.pass_baton();
				
				
				net.rec_msg(&buf);	//wait for msg to return
				// process hit
				if(buf.info.content == content_hit){
					enemies.at(target-1).at(pos).hit = true;
					if( available(enemies.at(target-1).at(pos)) ){
						enemies.at(target-1).at(pos).idn = unk_ship;
					}
					
				} else if (buf.info.content ==  content_ship_destroyed){
					cout <<  "Destroyed enemy ship!" << endl;
					enemies.at(target-1).set_destroyed_ship(buf.ship_info.ship);
					if(enemies.at(target-1).ship_n == 0){
						enemy_n--;
						cout << "Player " << (int)buf.info.origin << " has been annihilated, who will be next?\n";
					}
				} else {
					cout << "You missed!" << endl;
				}
				if(enemy_n == 0){
					game_ended = true;
					wID = net.my_id;
				}
				print_game(enemies);
				
				if(enemy_n == 0){
					game_ended = true;
					wID = net.my_id;
					break;
				}
				// wait baton for passing turn
				while(!net.with_baton){
					net.prev_player.rec(&buf, &p_addr);
					net.is_this_for_me(buf);
				}
			}
			
			net.pass_turn(my_turn);
		//not my turn
		} else {
			if (net.with_baton){
				if(has_response){ 
					net.send_msg(&msg_to_send, msg_to_send_size);
					has_response = false;
				}
				net.pass_baton();
			}else{
				size_t msg_size = net.prev_player.rec(&buf, &p_addr);
				
				// Message for me
				if( net.is_this_for_me(buf) ){
					//cout << "We received a tegami taichou" << endl;
					buf.info.status = status_ok;
					
					if(buf.info.content == content_attack){
						cout << "We got attacked" << endl;
						
						has_response = true;
						// set up response msg info 
						net.response_msg(&msg_to_send, buf.info.origin);
						
						// calculate hit
						int ship_hp = enemies.at(net.my_id-1).attackField(buf.coord_info.coord, ship_hit);
						
						if(ship_hp == 0){
							cout << "They destroyed our ship!" << endl;
							if(enemy_n == 1 && enemies.at(net.my_id-1).ship_n == 0){
								game_ended = true;
								for(int k = 0; k < enemies.size(); k++){
									if(enemies.at(k).alive()){
										wID = k+1;
									}
								}
							}
							msg_to_send.info.content = content_ship_destroyed;
							msg_to_send.ship_info.ship = ship_hit;
							msg_to_send.info.dest = buf.info.origin;
							msg_to_send.info.origin = net.my_id;
							msg_to_send_size = sizeof(ship_msg);
						} else {
							msg_to_send_size = sizeof(msg);
							if(ship_hp != FAIL){
								cout << "it hit!" << endl;
								msg_to_send.info.content = content_hit;
							} else {
								cout << "it missed. nyahahah" << endl;
								msg_to_send.info.content = content_miss;
							}
						}
						cout << "our map:\n";
						enemies.at(net.my_id-1).print();
					// if not an attack
					} else if(buf.info.content == content_turn){	//turn
						my_turn = true;
					}
				}
				if (buf.info.content == content_ship_destroyed){
					enemies.at(buf.info.origin-1).set_destroyed_ship(buf.ship_info.ship);
					cout <<"Player "<< (int)buf.info.origin <<"\'s ship was destroyed! Map:\n";
					enemies.at(buf.info.origin-1).print();
					if(enemies.at(buf.info.origin-1).ship_n == 0){
						enemy_n--;
						cout << "Player " << (int)buf.info.origin << " has been annihilated by Player " << (int)buf.info.dest << ", who will be next? Nyaahahaha\n";
					}
					if(enemy_n == 1 && enemies.at(net.my_id-1).ship_n == 0){
						game_ended = true;
						for(int k = 0; k < enemies.size(); k++){
							if(enemies.at(k).alive()){
								wID = k+1;
							}
						}
					}
				}
				// send forward non baton msgs
				if( ! buf.info.baton ){
					net.next_player.send(&buf, msg_size);
				}
			}
		}
	}
	if(wID == net.my_id){
		cout << "You are the winner! Congratulations!" << endl;
	} else {
		cout << "You LOST! Player " << wID << " destroyed everyone! Try harder next time!" << endl;
	}
	return 0;
}
