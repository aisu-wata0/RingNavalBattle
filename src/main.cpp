#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "Ship.h"
#include "Board.h"

using namespace std;

void board_setup(Board& board){
	Ship newShip;
	long numShips;
	
	board.print();
	cout << "Taishou, how many ships do you want to dispatch?\n";
	cin >> numShips;
	
	for(long i = 0; i < numShips; i++){
		cout << "What will be the coord. for top left (y, x) of the ship #" << i+1 << " Taishou?\n";
		cin >> newShip.top_left.y >> newShip.top_left.x;
	
		cout << "And how about the height and width of the ship #" << i+1 << " Taishou?\n";
		cin >> newShip.height >> newShip.width;
		
		if(board.set_ship(newShip) == FAIL){
			i--;
			cout << "You can't set up a ship like that!\n";
		}
		cout<<"\n";
	}
	
}

int main(int argc, char **argv)
{
	ifstream in_f;
	in_f.open("../content/amatsukaze.txt");
	string buffer;
	while(getline(in_f, buffer)){
		cout << buffer <<"\n";
	}
	in_f.close();
	
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
