#include <stdio.h>

enum spot { unk, water, ship_l, ship_u, captain, hit }

#define MINE 0
#define ENEMIES 1
#define fail -1
#define board_dead -2

class Coord {
	long y,x;
}

class Ship {
	Coord top_left;
	long height, width;
	
	Coord bot_right(){
		Coord br;
		br.y = top_left.y + height -1;
		br.x = top_left.x + width -1;
		return br;
	}
	
	bool not_exist(Coord sea_max){
		return (sh.bot_right().y > sea_max.y-1 || sh.bot_right().x > sea_max.x-1
		|| sh.top_left().x < 0 || sh.top_left().y < 0);
	}
}

class Board {
	Coord sea_max;
	int posession;
	int ship_n, ship_max;
	vector<spot> sea;
	
	Board(Coord sea_max, int posession): sea_max(sea_max.y*sea_max.x), posession(posession){
		if(posession == MINE){
			this->set_board(water);
		} else {
			this->set_board(unk);
		}
	}
	
	spot& at(long y, long x){
		return sea.at(y*sea_max.y + x);
	}
	spot& at(Coord pos){
		if( (pos.y*sea_max.x + pos.x) < sea.size() ){
			return sea.at(pos.y*sea_max.x + pos.x);
		}else{
			return unk_def;
		}
	}
	
	void set_board(spot sp){
		for(long i = 0; i < sea_max.y; i++){
			for(long j=0; j < sea_max.y; j++){
				this->at(i,j) = sp;
			}
		}
	}
	
	bool available(spot sp){
		return sp >= avail_min; // unk or water
	}

	int set_ship(Ship sh){
		int ship_id = ship_max;
		/*if(ship_id == ship_max){
			return fail;	//more ships than max
		}*/
		ship_max++;
		ship_n++;	//ship_n wa nan desu ka
		
		if(sh.not_exists(sea_max)){
			return fail;
		}
		
		for(long k = sh.top_left.y; k <= sh.bot_right().y; k++){
			for(long l = sh.top_left.x; l <= sh.bot_right().x; l++){
				if(!available(this->at(k,l)){
					return fail;
				}
			}
		}
		
		for(long k = sh.top_left.y; k <= sh.bot_right().y; k++){
			for(long l = sh.top_left.x; l <= sh.bot_right().x; l++){
				this->at(k,l) = ship_id;
			}
		}
		return true;
	}
	
	int get_ship(Coord pos, Ship& sh){
		int ship_hp = 0;
		Coord it;
		it = pos;
		
		// go left
		while(this->at(it).id == this->at(pos).id && it.x >= 0){
			if(this->at(it).hit == false){
				ship_hp++;
			}
			it.x--;
		}
		
		sh.top_left.x = it.x + 1;
		
		// go up and left
		it.y -= 1;
		
		while(this->at(it).id == this->at(pos).id && it.y >= 0 && it.x >= 0){
			for(it.x = pos.x; it.x >= sh.top_left.x; it.x--){
				if(this->at(it).hit == false){
					ship_hp++;
				}
			}
			it.y--;
		}
		sh.top_left.y = it.y + 1;
		
		// go right TODO
		it.y = pos.y;
		it.x = pos.x + 1;
		while(this->at(it).id == this->at(pos).id && it.x < sea_max.x){ //less or equal or just less
			if(this->at(it).hit == false){
				ship_hp++;
			}
			it.x++;
		}
		
		it.x--;
		
		// go bot right

		/*while(this->at(it).id == this->at(pos).id && it.y < sea_max.y && it.x < sea_max.x){
			for(it.x = pos.x; it.x < sh.br.x; it.x++){
				if(this->at(it).hit == false){
					ship_hp++;
				}
			}
			it.y++;
		}
		
		it.y --;
		*/
		
		
		return ship_hp;
	}
	
	int attackField(Coord pos, Ship& sh){
		int ship_hp;
		sh.top_left = fail;
		
		if(this->Board.at(pos).id <= max_ship_id){  //max ship id?
			this->Board.at(pos).hit = true;
			
			if((ship_hp = get_ship(pos, Ship sh)) == 0){
				ship_n--;
				if(ship_n == 0){
					return board_dead;
				}
			}
			return ship_hp;
		} else {
			return(fail);
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
