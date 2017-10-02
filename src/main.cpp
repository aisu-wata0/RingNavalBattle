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

void print_game(Board my_board, vector<Board> enemies){
	my_board.print();
	
	for(int i = 0; i < enemies.size(); ++i){
		cout << "Player " << i << endl;
		enemies.at(i).print();
	}
}

void read_attack(Coord& pos, int8_t& dest){
	cout << "Which player will you attack?" << endl;
	cin >> dest;
	cout << "Where will you attack next taichou? (y x)" << endl;
	cin >> pos.y >> pos.x;
}

int main(int argc, char **argv)
{
	//print_ascii("../content/amatsukaze-pc160.txt");
	int enemy_n = 1;
	long board_max_y = 5;
	long board_max_x = 5;
	int numShips = 2;
	
	int c;
	int my_id = 0;
	string next_hostname = "";
	ifstream in_f;
	ofstream o_f;
	streambuf* coutbuf = cout.rdbuf(); //save old buf; 
	
	while (( c = getopt(argc, argv, "p:h:")) != -1){
		switch (c){
			case 'p':
				my_id = stoi(optarg);
				break;
			case 'h':
				next_hostname = optarg;
				break;
			case ':':
			// missing option argument
				fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
			default:
				fprintf(stderr, "Usage: %s -p player_order -n next_hostname\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	if(my_id == 0){
		cout << "-p is mandatory\n";
		exit(1);
	}
	
	if(next_hostname == ""){
		cout << "-h is mandatory\n";
		exit(1);
	}
	
	char buf[BUFSIZ];
	
	sockaddr_in p_addr;
	
	char msg_to_send[BUFSIZ];
	size_t msg_to_send_size;
	
	Board my_board(Coord{.y = board_max_y, .x = board_max_x});
	Ship ship_hit;
	
	vector<Board> enemies;
	
	for (int i = 0; i < enemy_n; ++i){
		enemies.push_back(Board(Coord{.y = board_max_y, .x = board_max_x}, numShips));
	}

	board_setup(my_board, numShips);
	
	print_game(my_board, enemies);
	
	Connection net(my_id, next_hostname); // TODO: real ids
	
	bool has_response = false;
	bool game_ended = false;
	bool my_turn;

	if(net.my_id == 1){
		my_turn = true;
		cout<< "it's your turn" << endl;
	}else{
		my_turn = false;
		cout<< "standby" << endl;
	}
	
	while(!game_ended){
		if(my_turn){
			if(my_board.ship_n > 0){
				coord_msg attack_msg;
				
				read_attack(attack_msg.coord, attack_msg.info.dest);
				
				attack_msg.info.baton = false;
				attack_msg.info.status = 0;
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
					if(enemies.at(0).ship_n == 0){
						cout << "Player at ID has been annihilated, who will be next?\n";
					}
				} else {
					cout <<  "you missed!" << endl;
				}
				cout << "enemy map\n";
				enemies.at(0).print();

				// wait baton
				while(!net.with_baton){
					size_t msg_size = net.prev_player.rec(buf, BUFSIZ, &p_addr);
					if(net.is_this_for_me((msg*)buf)){
						net.with_baton = ((msg*)buf)->baton;
					}
				}
			}
			
			net.pass_turn(my_turn);
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
				
				((msg*)buf)->print();
				
				coord_msg* attack_msg;
			
				// Message for me
				if( net.is_this_for_me((msg*)buf) ){
					cout << "We received a tegami taichou" << endl;
					((msg*)buf)->status = status_ok;
					net.with_baton = ((msg*)buf)->baton;
					
					if( ((msg*)buf)->content == content_attack ){
						cout << "We got attacked" << endl;
						has_response = true;
						attack_msg = (coord_msg*)buf;
						// calculate hit
						int ship_hp = my_board.attackField(attack_msg->coord, ship_hit);
						
						((coord_msg*)msg_to_send)->info.dest = attack_msg->info.origin;
						((coord_msg*)msg_to_send)->info.origin = net.my_id;
						if(ship_hp == 0){
							cout << "They destroyed our ship!" << endl;
							msg_to_send_size = sizeof(ship_msg);
//							for(int i=0; i < enemies.size(); i++){
//								ship_destroyed_msg.info.dest = enemies.at(i).player;
//								msg_queue.push_back(ship_destroyed_msg);
//							}
							((ship_msg*)msg_to_send)->info.content = content_ship_destroyed;
							((ship_msg*)msg_to_send)->ship = ship_hit;
						} else {
							msg_to_send_size = sizeof(msg);
							if(ship_hp != FAIL){
								cout << "it hit!" << endl;
								((msg*)msg_to_send)->content = content_hit;
							} else {
								cout << "it missed. nyahahah" << endl;
								((msg*)msg_to_send)->content = content_miss;
							}
							//msg_queue.push_back(hit_msg);
						}
						cout << "our map:\n";
						my_board.print();
					} else if (((msg*)buf)->content == content_turn){	//turn
						cout << "It's now our turn!" << endl;
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
