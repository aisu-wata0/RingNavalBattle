#include <stdio.h>
#include <iostream>

#include "Ship.h"
#include "Board.h"

using namespace std;

void board_setup(Board& board){
	
	// read coordinates
	// no rotation
}

int main(int argc, char **argv)
{
	Board my_board(Coord{.y = 7, .x = 7}, MINE);
	
	cout<<"\n\n";
	my_board.print();
	
	Ship ship;
    Ship target;
    
	ship.top_left.y = 0;
	ship.top_left.x = 0;
	ship.height = 3;
	ship.width = 1;
	
	my_board.set_ship(ship);
    
	cout<<"\n\n";
    
	my_board.print();
	
	ship.top_left.y = 2;
	ship.top_left.x = 2;
	ship.height = 1;
	ship.width = 3;
    
	my_board.set_ship(ship);

	cout<<"\n\n";

    my_board.print();
    
    Coord att;
    att.x = 0;
    att.y = 0;

    my_board.attackField(att, target);
    att.y++;
    my_board.print();
    cout<<"\n\n";

    
    my_board.attackField(att, target);
    att.y++;
    my_board.print();
    cout<<"\n\n";


    my_board.attackField(att, target);
    my_board.print();
    cout<<"\n\n";

    //first ship destroyed
    /*
    att.x = 2;
    att.y = 2;
    
    my_board.attackField(att, target);
    att.x++;
    //my_board.print();
    
    my_board.attackField(att, target);
    att.x++;
    //my_board.print();

    my_board.attackField(att, target);
    //my_board.print();
    //second ship destroyed board dead
	cout<<"\n\n";*/
    
	board_setup(my_board);
	return 0;
}
