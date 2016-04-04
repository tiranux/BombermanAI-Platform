#include "Global_Vars.h"
#include "SDL_Handler.h"
#include "Player.h"

//Screen dimension constants 
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int SPRITE_SIZE   = 32;
const int NUM_COLS = SCREEN_WIDTH / SPRITE_SIZE;
const int NUM_ROWS = SCREEN_HEIGHT / SPRITE_SIZE;

// Global logic 
int gameover = 0;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
	
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Temporary events
SDL_Event event;

//Players' holder
Player* player1 = new Player("player.bmp" , 1, 1);
Player* player2 = new Player("player2.bmp" , 18, 13);;
Player* player3;
Player* player4;

//Number of active players
int num_Players = 2;

//Holds world map vector
char* world_Map = (char*) malloc(sizeof(char) * ((NUM_COLS) * (NUM_ROWS) ));

//Map object to control all events
Map* map;

//Group of threads to handle player's communication
std::thread *p_comm = new std::thread[num_Players - 1];

void call_from_thread(int tid) {
        std::cout << "Launched by thread " << tid << std::endl;
}

int main ( int argc, char *argv[] )
{
    std::vector<Player*> all_Players;
    all_Players.push_back(player1);
	all_Players.push_back(player2);
	all_Players.push_back(player3);
	all_Players.push_back(player4);
    
    //Launch a group of threads
    for (int i = 0; i < num_Players; ++i) {
        p_comm[i] = std::thread(call_from_thread, i);
    }   
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
		exit(-1);
	}
		//Load all images 
	if(!load_Media())
	{
		printf( "Failed to load textures!\n" );
		exit(-1);
	}
	else
	{			
        map = new Map(world_Map);	
		/* message pump */
		while (!gameover  && num_Players > 1 )
		{
			/* look for an event */
			if (SDL_PollEvent(&event) ){
				handle_Events(event, map, all_Players.front());
			}
            map->update_Game(all_Players);         
            draw_Map(map);
            draw_Player(all_Players);
			//Update the surface
			SDL_UpdateWindowSurface( gWindow );
		}
        const char * winner =  map->who_Won(all_Players);
        if ( winner == NULL)
            std::cout << "Draw!!" << std::endl;
        else
            std::cout << "Player " << winner << " won!! Congratualtions!" << std::endl;
	}
        //Join the threads with the main thread
    for (int i = 0; i < num_Players; ++i) {
        p_comm[i].join();
    }
	close();
	return 0;
}
