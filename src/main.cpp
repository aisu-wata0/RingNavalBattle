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
	cout << "Taishou, how many ships do you want to dispatch?\n";
	cin >> numShips;
	
	for(long i = 0; i < numShips; i++){
		cout << "Enter coordinates for top left (y, x) of the ship #" << i+1 << "\n";
		cin >> newShip.top_left.y >> newShip.top_left.x;
	
		cout << "Enter height(alltura) then width(largura) of ship #" << i+1 << "\n";
		cin >> newShip.height >> newShip.width;
		
		if(board.set_ship(newShip) == FAIL){
			i--;
			cout << "Ship depatching failed\n";
		}
		cout<<"\n\n";
	}
	
}

int main(int argc, char **argv)//dwqewq
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r); //stores the console's current dimensions
	MoveWindow(console, r.left, r.top, 1920, 1080, TRUE);
	
	ifstream in_f;
	//streambuf* cinbuf = cin.rdbuf(); //save old buf; 	ifstream in_f;
	in_f.open("../content/kancolle.txt");
	string buffer;
	while(getline(in_f, buffer)){
		cout << buffer <<"\n";
	}
	in_f.close();
	
	Board my_board(Coord{.y = 7, .x = 7}, MINE);
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
	// first ship destroyed board NOT dead, changed first ship coord[0 0] hxw[3 3] for test purposes
	/*att.x = 0;
	for(long att_coor=0; att_coor <= 2; att_coor++){
		att.y = att_coor;
		cout<<"\n attack "<< att.y <<","<< att.x <<" return: "<< my_board.attackField(att, target) <<"\n";
		my_board.print();
		cout<<"\n\n";
	}
	att.y = 2;
	for(long att_coor=2; att_coor <= 2+3-1; att_coor++){
		att.x = att_coor;
		cout<<"\n attack "<< att.y <<","<< att.x <<" return: "<< my_board.attackField(att, target) <<"\n";
		my_board.print();
		cout<<"\n\n";
	}*/
	//second ship destroyed board dead
	my_board.print();
	return 0;
}
