#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>

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

void print_game(Board my_board, vector<Board> enemies){
	my_board.print();
	
	for (int i = 0; i < enemies.size(); ++i)
		cout << "Player " << i << endl;
		enemies.at(i).print();
}

int main(int argc, char **argv)
{
	//print_ascii("../content/amatsukaze-pc160.txt");
	int enemy_n = 3;
	long board_max_y = 5;
	long board_max_x = 5;
	
	queue tegami_queue;
	
	Board my_board(Coord{.y = board_max_y, .x = board_max_x}, Mine);
	
	vector<Board> enemies;
	for (int i = 0; i < enemy_n; ++i)
		enemies.push_back(enemy(Coord{.y = board_max_y, .x = board_max_x}, Enemy));
	
	Ship target;

	board_setup(my_board);
	
	print_game(my_board, enemies);

	while(!game_ended){
		if(my_turn){
			read_attack();
			
			msg_queue.push_back(attack_msg);
			
			wait_attack_resolution();
			
			process_attack();
			
			pass_turn();
		} else {
			if (with_baton){
				while(msg_queue.size() > 0){
					send_msg(msg_queue.front());
					msg_queue.pop_front();
				}
			}else{
				receive_msgs(&Tegami);

				// Message for me
				if(Tegami.info.dest == my_id){
					with_baton = Tegami.info.baton;
					
					if(Tegami.info.content == content_attack){
						// calculate hit
						// ...
						if(ship_hp > 0){
							msg_queue.push_back(hit_msg);
						} else {
							for(int i=0; i < enemies.size(); i++){
								ship_destroyed_msg.info.dest = enemies.at(i).player;
								msg_queue.push_back(ship_destroyed_msg);
							}
						}
					}
				}
				// don't send forward baton msgs
				if(!with_baton){
					Tegami.info.status = true;
					send_msg(Tegami);
				}
			}
			
		}
	}
	
	return 0;
}