#include <stdio.h>

enum spot { unk, water, ship, hit }
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
	
	spot& at(i,j){
		return sea.at(i*nc + j);
	}
	
	void set(spot sp){
		for(long i = 0; i < nl; i++){
			for(long j=0; j < nc; j++){
				this->at(i,j) = sp;
			}
		}
	}
	
	int set_ship(long i, long j, Ship sh){
		if(i+sh.height-1 >= nc || j+sh.width-1 >= nl){
			return fail;
		}
		for(long k=i; k <= i+sh.height-1; k++){
			for(long l=j; l <= i+sh.width-1; l++){
				this->at(k,l) = ship;
			}
		}
	}
}

void board_setup(Board& B){
	
	// read coordinates
	// no rotation
}


int main(int argc, char **argv)
{
	
	return 0;
}
