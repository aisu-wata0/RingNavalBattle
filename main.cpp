#include <stdio.h>

enum spot { unk, water, ship_l, ship_u, captain, hit }

#define MINE 0
#define ENEMIES 1
#define fail -1

class Ship {
	long width;
	long height;
	
	Ship(long width, long height): width(width), height(height){
	}
}

class Board {
	long nl,nc;
	int posession;
	vector<spot> sea;
	
	Board(long nl, long nc, int posession): sea(nl*nc), posession(posession){
		if(posession == MINE){
			this->set(water);
		} else {
			this->set(unk);
		}
	}
	
	spot& at(long i, long j){
		return sea.at(i*nc + j);
	}
	
	void set(spot sp){
		for(long i = 0; i < nl; i++){
			for(long j=0; j < nc; j++){
				this->at(i,j) = sp;
			}
		}
	}
	
	int set_ship(long y, long x, Ship sh){
		if(i+sh.height-1 >= nc || j+sh.width-1 >= nl){
			return fail;
		}
		
		this->at(y,x) = captain;
		for(long k=y; k <= y+sh.height-1; k++){
			this->at(k,l) = ship_u;
		}
		for(long l=x; l <= x+sh.width-1; l++){
			this->at(k,l) = ship_l;
		}
		for(long k=y+1; k <= y+sh.height-1; k++){
			for(long l=x+1; l <= x+sh.width-1; l++){
				this->at(k,l) = ship_l;
			}
		}
	}
	
	void attackField(long i, long j){
		if(this->Board.at(i, j) == ship){
			check_if_ship_destroyed
		}
	}
}

void board_setup(Board& B){
	
	// read coordinates
	// no rotation
}




int main(int argc, char **argv)
{
	enemyBoard.attackField(3,4);
	return 0;
}
