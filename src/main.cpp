#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Ship.h"
#include "Board.h"

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

int main(int argc, char **argv)
{
	//print_ascii("../content/amatsukaze-pc160.txt");
	
	Board my_board(Coord{.y = 5, .x = 5}, MINE);
	Ship target;

	board_setup(my_board);
	my_board.print();
	
	Coord att;
	att.x = 0;
	
	for(long att_coor=0; att_coor <= 2; att_coor++){
		for(long att_cory=0; att_cory <= 2; att_cory++){
			att.y = att_cory;
			att.x = att_coor;
			cout<<"\n attack "<< att.y <<","<< att.x <<" return: "<< my_board.attackField(att, target) <<"\n";
			my_board.print();
			cout<<"\n\n";
		}
	}
	
	return 0;
}
