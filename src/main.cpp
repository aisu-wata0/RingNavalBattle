#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>

#include "Ship.h"
#include "Board.h"
#include "Network.h"

using namespace std;

void board_setup(Board& board, long numShips, int ID){
	Ship newShip;
	board.player = ID;
	
	cout << "Taichou, you are player " << ID << " and this is our advanced technology map of the highest precision, the top left coordinate is (0,0) (y,x)\n";
	board.print();
	cout << "You have " << numShips << " to dispatch.\n";
	string confirmed("n");
	while(confirmed != "y"){
		board.set_board(water);
		for(long i = 0; i < numShips; i++){
			cout << "What will be the coord for the top left (y x) of the ship #" << i << " taichou?\n";
			cin >> newShip.top_left.y >> newShip.top_left.x;
		
			cout << "And how about the height and width of the ship #" << i << " taichou?\n";
			cin >> newShip.height >> newShip.width;
			
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

void read_attack(Coord& pos, int8_t& dest){
	int d = 0;
	pos.y = 0;
	pos.x = 0;
	cout << "Which player will you attack?" << endl;
	cin >> d;
	dest = d;
	cout << "Where will you attack next taichou? (y x)" << endl;
	cin >> pos.y >> pos.x;
}

int main(int argc, char **argv)
{
	//print_ascii("content/amatsukaze.txt");
	int enemy_n = 0;
	long board_max_y = 5;
	long board_max_x = 5;
	int numShips = 1;
	
	int c , wID;
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
	
	msg_buffer buf, buf2;
	msg_buffer start_msg; 
	
	sockaddr_in p_addr;

	bool ready = false;
	bool set = false;
	
	// Setup ids
	Connection net(next_hostname);
	
	if(my_id == 1){
		net.my_id = 1;
		start_msg.id_info.my_id = 1;
	}
	// TODO: what to do if a player joins then disconects, ids get messed up
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
	bool my_turn, received = false;

	if(net.my_id == 1){
		my_turn = true;
		cout<< "it's your turn" << endl;
	}else{
		my_turn = false;
		cout<< "standby" << endl;
	}
	
	while(!game_ended || has_response){
		if(my_turn){
			if(enemies.at(net.my_id-1).alive()){
				msg_buffer attack_msg;
				Coord pos;
				int8_t target;
				
				read_attack(pos, target);
				
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
						cout << "Player " << (int) buf.info.origin << " has been annihilated, who will be next?\n";
					}
				} else {
					cout << "You missed!" << endl;
				}
				if(enemy_n == 0){
					game_ended = true;
					wID = net.my_id;
				}
				cout << "Enemy map:\n";
				enemies.at(target-1).print();
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
			/*
			if(enemy_n == 0){
				game_ended = true;
				wID = net.my_id;
			} else if(enemy_n == 1 && not my_board.alive()) {
				game_ended = true;
				
				for(int p = 0; p < enemies.size(); p++){
					if(enemies.at(p).alive()){
						wID = p+1;
					}
				}
			}*/
			if (net.with_baton){
				if(has_response){ // msg_queue.size() > 0
					//send_msg(msg_queue.front());
					//msg_queue.pop_front();
					net.send_msg(&msg_to_send, sizeof(msg_to_send));
					has_response = false;
				}
				net.pass_baton();
			}else{
				size_t msg_size = net.prev_player.rec(&buf, &p_addr);
								
				// Message for me
				if( net.is_this_for_me(buf) ){
					cout << "We received a tegami taichou" << endl;
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
							// TODO: Send msg to everyone for ship destroyed
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
						} else {
							msg_to_send_size = sizeof(msg);
							if(ship_hp != FAIL){
								cout << "it hit!" << endl;
								msg_to_send.info.content = content_hit;
							} else {
								cout << "it missed. nyahahah" << endl;
								msg_to_send.info.content = content_miss;
							}
							//msg_queue.push_back(hit_msg);
						}
						cout << "our map:\n";
						enemies.at(net.my_id-1).print();
					// if not an attack
					} else if(buf.info.content == content_turn){	//turn
						cout << "It's now our turn!" << endl;
						my_turn = true;
					}
				}
				// send forward non baton msgs
				if (buf.info.content == content_ship_destroyed){
					enemies.at(buf.info.origin-1).set_destroyed_ship(buf.ship_info.ship);
					if(enemies.at(buf.info.origin-1).ship_n == 0){
						enemy_n--;
						cout << "Player " <<  (int) buf.info.origin << " has been annihilated by Player" << (int) buf.info.dest << ", who will be next? Nyaahahaha\n";
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
				if( ! buf.info.baton ){
					net.next_player.send(&buf, msg_size);
				}
			}
		}
	}
	cout << "And the winner is Player " << wID <<", GRATULEIXONS!!" << endl;
	return 0;
}
