#include "Functions.h"
#include <functional>

//Tile node struct pos(x,y) & father
struct node{
    //position in map matrix
    int x;
    int y;
    //is a dangerous tile?
    bool danger;
    //has father
    bool has_father;
    //father 
    node *father;
};

//Plan K to follow
int k = 2;
int kcount = 0;

//Last bomb's position
int bomb_x = -1;
int bomb_y = -1;

//Last action
int last_action = 0;

std::deque<Actions> *plan = new std::deque<Actions>();
// Next objectives
int mission_x;
int mission_y;
bool in_danger;
int largest_range; 
int turnsWaiting = 111;

//Helper functions
int next_action();
double distance_Calculate(int x1, int y1, int x2, int y2);
std::vector<Actions> get_Possible_Moves(int x, int y);
std::deque<node> get_Possible_Positions(node *father);
Actions get_What_Move_To_Make (int x, int y, int x2, int y2);
bool check_for_players();
void search_for_players();
void flee_bombs();
void search_for_walls();
bool get_plan(node* fat, int tmpx, int tmpy, std::deque<node> & explore);
void bfs( std::function<bool( node&,std::deque<node> & explore )> condition );
bool wall_conditions(node& at,std::deque<node> & explore);
bool bombs_conditions(node& at, std::deque<node> & explore);
bool players_conditions(node& at, std::deque<node> & explore); 
bool check_for_bombs(int x_pos, int y_pos);

int main()
{
    bool gameover = false;
    clock_t t1,t2;
    float diff,seconds; 
    int action;
    in_danger = false;
    
    if( connect() ) 
    {
        //Allocate space to world map
        wordl_map = (char**) std::malloc(sizeof(char*) * NUM_ROWS);
        for(int i = 0; i < NUM_ROWS; i++)
        {
            wordl_map[i] = (char*) std::malloc(sizeof(char) * NUM_COLS);
        }
        //Allocate space for players positions
        x = (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        y = (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        r = (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        speed = (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        alive = (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        teams =  (int*) std::malloc(sizeof(int) * NUM_PLAYERS);
        //Game loop
        while( !gameover )
        {
            gameover = update_Map();
            if ( !gameover )
            {  //Updates players' information
               gameover = update_Players();
            }
            t1=clock();
            // AI agents get next action 
            action = next_action();
            t2=clock();
            diff  = ((float)t2-(float)t1);
            seconds = diff / CLOCKS_PER_SEC;
                std::cout << action;
           /* if ( seconds <= 1 ){
                // 5 plays per second 
                usleep((CLOCKS_PER_SEC/speed[PLAYER_ID])-seconds*(CLOCKS_PER_SEC/5));
                // Print action to send to server 
                std::cout << action;
            }
            else{
                // Agent only have 1sec to give the next action
                std::cout << "TIMEOUT"<<std::endl;
            }*/
        }
    }
    return 0;
}

/* Calculates euclidean distance */
double distance_Calculate(int x1, int y1, int x2, int y2)
{
    int x = x1 - x2;
    int y = y1 - y2;
    double dist;
    dist = pow(x,2)+pow(y,2); 
    dist = sqrt(dist);
    return dist;
}

/* Check tiles arround position (x,y) */
std::vector<Actions> get_Possible_Moves(int x, int y)
{
    std::vector<Actions> possible_moves;
    if( wordl_map[x + 1][y] != STONE &&  wordl_map[x + 1][y] != WALL &&  wordl_map[x + 1][y] != BOMB &&  wordl_map[x + 1][y] != EXPLOSION)
        possible_moves.push_back(DOWN); //DOWN
        
    if( wordl_map[x - 1][y] != STONE &&  wordl_map[x - 1][y] != WALL &&  wordl_map[x - 1][y] != BOMB &&  wordl_map[x - 1][y] != EXPLOSION)
        possible_moves.push_back(UP); //UP

    if( wordl_map[x][y + 1] != STONE &&  wordl_map[x][y + 1] != WALL &&  wordl_map[x][y + 1] != BOMB &&  wordl_map[x][y + 1] != EXPLOSION)
        possible_moves.push_back(RIGHT); //RIGHT

    if( wordl_map[x][y -1] != STONE &&  wordl_map[x][y - 1] != WALL &&  wordl_map[x][y - 1] != BOMB &&  wordl_map[x][y - 1] != EXPLOSION)
        possible_moves.push_back(LEFT); //LEFT

    return possible_moves;
}
/* */
Actions get_What_Move_To_Make (int x, int y, int x2, int y2)
{
    if( x < x2 )
        return DOWN; //DOWN
    else if( x > x2 )
        return UP;
    else if( y < y2)
        return RIGHT;
    else if( y > y2)
        return LEFT;
        
    return STOP;
}

std::deque<node> get_Possible_Positions(node *father)
{
    int x = father->x; 
    int y = father->y;
    std::deque<node> possible_moves;
    /* Flees from bombs */
    if(  wordl_map[x + 1][y] != STONE && wordl_map[x + 1][y] != WALL  && wordl_map[x + 1][y] != BOMB && wordl_map[x + 1][y] != EXPLOSION)
    {   
        node pos = {x+1,y,false,true,father};
        possible_moves.push_back(pos); //DOWN
    }
    if( wordl_map[x - 1][y] != STONE &&  wordl_map[x - 1][y] != WALL &&   wordl_map[x - 1][y] != BOMB && wordl_map[x - 1][y] != EXPLOSION)
    {
        node pos = {x-1, y,false,true,father};
        possible_moves.push_back(pos); //DOWN
    }
    if( wordl_map[x][y +1] != STONE && wordl_map[x][y + 1] != WALL && wordl_map[x][y + 1] != BOMB &&  wordl_map[x][y + 1] != EXPLOSION)
    {
        node pos = { x, y+1,false,true,father};
        possible_moves.push_back(pos); //DOWN
    }
    if(  wordl_map[x][y -1] != STONE && wordl_map[x][y - 1] != WALL && wordl_map[x][y - 1] != BOMB &&  wordl_map[x][y - 1] != EXPLOSION)
    {
        node pos = { x, y-1,false,true,father};
        possible_moves.push_back(pos); //DOWN
    }
    return possible_moves;
} 

bool check_for_players()
{
    /* Update largest range */
    double lr = -1;
    int lr_id = -1;
    for ( int i = 0; i < NUM_PLAYERS; i++)
    {
        if( alive[i])
        {
            if ( r[i] > lr )
            {
                lr = r[i];
                lr_id = i;
            }
        }
    }
    largest_range = r[lr_id];
    /* now check if anyplayer in range*/
    double min_dist = 9999.00;
    int min_dist_pid = -1;
    for ( int i = 0; i < NUM_PLAYERS; i++)
    {
        if( i != PLAYER_ID && alive[i])
        {
            double tmp_dist = distance_Calculate(x[PLAYER_ID], y[PLAYER_ID], x[i], y[i]);
            if ( tmp_dist - r[i] <= 0 && (tmp_dist - r[i]) < min_dist )
            {
                min_dist = tmp_dist;
                min_dist_pid = i;
            }
        }
    }
    if ( min_dist_pid != -1){
        mission_x = x[min_dist_pid];
        mission_y = y[min_dist_pid];
        return true;
    }
    return false;
}

void search_for_players()
{
    bfs(players_conditions);
}

bool players_conditions(node& at, std::deque<node> & explore)
{
     /* Check if it has a pathway to anyplayer */
    for ( int p = 0; p < NUM_PLAYERS; p++)
    {
        if( p != PLAYER_ID && alive[p])
        {
            if ( (at.x == x[p] &&  at.y == y[p]))
            {
                if ( distance_Calculate(at.x, at.y, x[PLAYER_ID], y[PLAYER_ID]) < r[PLAYER_ID]  )
                {
                    plan->push_front(FIRE);
                    return true;
                }
                if ( get_plan(at.father,at.x, at.y, explore) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool get_plan(node* fat, int tmpx, int tmpy, std::deque<node> & explore)
{
    plan->push_front(get_What_Move_To_Make(fat->x, fat->y, tmpx, tmpy));
    while ( fat->has_father )
    {
        plan->push_front(get_What_Move_To_Make(fat->father->x, fat->father->y, fat->x, fat->y));
        fat = fat->father;
    }
    if ( !plan->empty() )
    {
        explore.clear();
        return true;
    }
    return false;
}


void flee_bombs(){
    bool seen;
    bool stop_exploration = false;
    
    bomb_x = -1;
    bomb_y = -1;    
    /* Check for bombs */

    if ( check_for_bombs (x[PLAYER_ID], y[PLAYER_ID]) ) 
    {
        bfs(bombs_conditions);
    }
}

bool check_for_bombs(int x_pos, int y_pos){
    for ( int i = -largest_range; i <= largest_range; i++)
    {
        if( x_pos +i >= 0 && x_pos+i < NUM_ROWS && wordl_map[x_pos + i][y_pos] == BOMB )
        {   
            bomb_x = x_pos+i;
            bomb_y = y_pos;
            return true;
        }
        if(  y_pos+i >= 0 && y_pos+i < NUM_COLS &&  wordl_map[x_pos ][y_pos + i] == BOMB )
        {   
            bomb_x = x_pos;
            bomb_y = y_pos+i;
            return true;
        }
    } 
    return false;
}

bool bombs_conditions(node& at, std::deque<node> & explore)
{
    if (  !check_for_bombs (at.x, at.y) )
    {
        in_danger = true;
        if ( get_plan(at.father, at.x, at.y, explore) )
        {
            return true;
        }
    }
    return false;
}

void search_for_walls()
{
    bool seen;
    bool stop_exploration = false;
    
    if ( wordl_map[x[PLAYER_ID]+1][y[PLAYER_ID]] == STONE || wordl_map[x[PLAYER_ID]-1][y[PLAYER_ID]] == STONE ||
        wordl_map[x[PLAYER_ID]][y[PLAYER_ID] -1] == STONE || wordl_map[x[PLAYER_ID]][y[PLAYER_ID] +1] == STONE)
    {
        plan->push_front(FIRE);
        in_danger = true;
    }
    else
    {
        bfs(wall_conditions);
    }
}
bool wall_conditions(node& at, std::deque<node> & explore)
{    
    if ( wordl_map[at.x+1][at.y] == STONE || wordl_map[at.x-1][at.y] == STONE ||
        wordl_map[at.x][at.y -1] == STONE || wordl_map[at.x][at.y +1] == STONE)
    {
        if ( get_plan(at.father, at.x, at.y, explore) )
        {
            return true;
        }
    }
    return false;
}


void bfs( std::function<bool(node &, std::deque<node> & explore)> condition )
{
    bool seen;
    bool stop_exploration = false;
    node root = {x[PLAYER_ID], y[PLAYER_ID],false ,NULL};
    std::deque<node> visited;
    visited.push_front(root);
    std::deque<node> explore = get_Possible_Positions(&root);
    for( int i = 0; i < explore.size(); i++)
    {
            if ( ( !check_for_bombs(explore[i].x, explore[i].y) || in_danger) && condition(explore[i], explore) )
                break;
    }
    while( !explore.empty() ) 
    {
        visited.push_front(explore.front());
        explore.pop_front();
        
        std::deque<node> tmp = get_Possible_Positions(&visited.front());
        for( int i = 0; i < tmp.size(); i++)
        {
            seen = false;
            //Checks if it has been already visited
            for ( int j = 0; j < visited.size(); j++)
            {
                if ( tmp[i].x == visited[j].x &&  tmp[i].y == visited[j].y ){
                    seen = true;
                    break;
                }
            }
            //Checks if it has been pinned to explore
            for ( int j = 0; j < explore.size(); j++)
            {
                if ( tmp[i].x == explore[j].x &&  tmp[i].y == explore[j].y ){
                    seen = true;
                    break;
                }
            }
            if ( !seen && ( !check_for_bombs(tmp[i].x, tmp[i].y) || in_danger) ) 
            {
                explore.push_back(tmp[i]);
                
                if ( condition(tmp[i], explore) )
                    break;
            }
        }
    }
}

/* AI agents next action */
int  next_action()
{
    int action = -1;
 
    //Updates mission objective position
    bool cfp = check_for_players();
   
    if ( plan->empty() ){
        kcount = 0;

        /*Survival above all */
        flee_bombs();
        if ( plan->empty() && in_danger){
            in_danger = false;
            turnsWaiting = 0;
            plan->push_front(STOP);
        }
        /*If it's not in danger go after players*/
        if ( plan->empty())
            search_for_players();        
        /*Open pathways to players */
        if ( plan->empty())
            search_for_walls();
            
        if ( !plan->empty() )
        {
            kcount++;
            action = plan->front();
            plan->pop_front();
        }

    }
    else
    {
        kcount++;
        action = plan->front();
        plan->pop_front();
        if ( kcount >= k ){
            plan->clear();
        }
    }

    return action;
}