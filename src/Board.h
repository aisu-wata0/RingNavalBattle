#ifndef BOARD_H
#define BOARD_H

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include <vector>
#include <string>

#include "Ship.h"

namespace std {

class Board {
	public:
	Coord sea_max;
	int posession;
	int ship_n, ship_max;
	vector<Spot> sea;
	
	Board(Coord sea_max, int posession): posession(posession), sea(sea_max.y*sea_max.x){
		this->sea_max.y = sea_max.y;
		this->sea_max.x = sea_max.x;
		ship_max = 0;
		ship_n = 0;
		if(posession == MINE){
			this->set_board(water);
		} else {
			this->set_board(unk);
		}
	}
	
	Spot& at(long y, long x){
		if( (y >= 0) && (x >= 0) && (y < sea_max.x) && (x < sea_max.x) ){
			return sea.at(y*sea_max.x + x);
		}else{
			return unk_def;
		}
	}
	Spot& at(Coord pos){
		return this->at(pos.y,pos.x);
	}
	
	void set_board(uint32_t sp){
		for(long i = 0; i < sea_max.y; i++){
			for(long j=0; j < sea_max.y; j++){
				this->at(i,j).idn = sp;
				this->at(i,j).hit = false;
			}
		}
	}
	
	bool available(Spot sp){
		return sp.idn >= avail_min; // unk or water
	}

	int set_ship(Ship ship){
		if(ship_max >= max_ships){
			return FAIL;
		}
		int ship_id = ship_max;
		
		ship_max++;
		ship_n++;
		
		if(ship.not_exist(sea_max)){
			return FAIL;
		}
		
		for(long k = ship.top_left.y; k <= ship.bot_right().y; k++){
			for(long l = ship.top_left.x; l <= ship.bot_right().x; l++){
				if( !available(this->at(k,l)) ){
					return FAIL;
				}
			}
		}
		
		for(long k = ship.top_left.y; k <= ship.bot_right().y; k++){
			for(long l = ship.top_left.x; l <= ship.bot_right().x; l++){
				this->at(k,l).idn = ship_id;
			}
		}
		return true;
	}
	
	int get_ship(Coord pos, Ship& ship){
		int ship_hp = 0;
		Coord it;

		// go left
		it = pos;
		while(this->at(it).idn == this->at(pos).idn){
			if(this->at(it).hit == false){
				ship_hp++;
			}
			it.x--;
		}
		
		ship.top_left.x = it.x + 1;
		
		// go up and left
		it.y = pos.y - 1;
		it.x = pos.x;
		while(this->at(it).idn == this->at(pos).idn){
			for(; it.x >= ship.top_left.x; it.x--){
				if(this->at(it).hit == false){
					ship_hp++;
				}
			}
			it.y--;
			it.x = pos.x;
		}
		ship.top_left.y = it.y + 1;
		
		// go right
		it.y = pos.y;
		it.x = pos.x + 1;
		while(this->at(it).idn == this->at(pos).idn){
			if(this->at(it).hit == false){
				ship_hp++;
			}
			it.x++;
		}
		
		it.x--;
		ship.width = it.x - ship.top_left.x + 1;
		
		// go bot right
		it.y = pos.y + 1;
		it.x = pos.x;
		while(this->at(it).idn == this->at(pos).idn){
			for(; it.x <= ship.bot_right().x; it.x++){
				if(this->at(it).hit == false){
					ship_hp++;
				}
			}
			it.y++;
			it.x = pos.x;
		}
		
		it.y --;
		ship.height = it.y - ship.top_left.y + 1;

		// go top right
		for(it.x = pos.x+1; it.x <= ship.top_left.x + ship.width; it.x++){
			for(it.y = pos.y-1; it.y >= ship.top_left.y; it.y--){
				if(this->at(it).hit == false && this->at(it).idn == this->at(pos).idn){
					ship_hp++;
				}
			}
		}
		
		// go bot left
		for(it.x = pos.x - 1; it.x >= ship.top_left.x; it.x--){
			for(it.y = pos.y + 1; it.y <= ship.top_left.y + ship.height; it.y++){
				if(this->at(it).hit == false && this->at(it).idn == this->at(pos).idn){
					ship_hp++;
				}
			}
		}

		return ship_hp;
	}
	
	bool is_ship(Spot sp){
		return sp.idn <= max_ships;
	}
	
	int attackField(Coord pos, Ship& ship){
		int ship_hp;
		ship.top_left.y = FAIL;
		
		if( is_ship(this->at(pos)) ){
			this->at(pos).hit = true;
			
			if((ship_hp = get_ship(pos, ship)) == 0){
				ship_n--;
				if(ship_n == 0){
					return board_dead;
				}
			}
			return ship_hp;
		}
		return FAIL;
	}
	
	void print(){
		Coord it;
		for(it.y = 0; it.y < sea_max.y; it.y++){
			for(it.x = 0; it.x < sea_max.x; it.x++){
				this->print(this->at(it));
			}
			cout<< endl;
		}
	}
	
	void print(Spot spot){
		switch(spot.idn){
			case unk:
				cout<<" ";
				break;
			case water:
				cout<<"~";
				break;
			default:
				if(spot.hit){
					cout<<"w";
				} else {
					cout<<"s";
				}
		}
	}
};

}// namespace std

#endif
