#ifndef PongGame_h
#define PongGame_h




/* Game setting and memory */
#define TICKS_PER_SECOND 60
#define MILLIS_PER_TICK 1000/TICKS_PER_SECOND
#define TICKS_PER_MOVEMENT 2
#define STANDARD_TRAVERSAL_TIME 1400

#define BASE_BOOST_DURATION 150
#define BASE_CLOSED_DURATION 500
#define BASE_RECOVERY_DURATION 1000
#define TICKS_UNTIL_AUTOMATIC_ACCELERATION 600


#define PLAYER_A 0
#define PLAYER_B 1
#define NONE 255


enum GAME_STATES {
  OFF,
  START, 
  BALL_SERVICE,
  BALL_EXCHANGE,
  PLAYER_SCORES,
  GAME_OVER,
  CLOSING
};

enum base_state_enum {
  BASE_BOOST,
  BASE_CLOSED,
  BASE_OPEN,
  BASE_RECOVERY
};

class PongGame
{
  public:
    /* Instantiate the game */
    PongGame(int gridsize);

    /* ----------- Operations ------------------ */

    /* Main processing of the game logic 
     *  Returns: milliseconds until the next tick is pending
    */
    void setupGame();
    void startGame();
   
    
    unsigned long process_tick();

    /* ---- State information ----*/
    bool isActive();
    bool isClosing();
    
    unsigned int getBallPosition();
    base_state_enum getBase_A_State();
    base_state_enum getBase_B_State();
    int  player_A_getScore();
    int  player_B_getScore();
    byte player_getWinner();
    


  protected:

    void enter_START();
    void process_START();
    void enter_BALL_SERVICE();
    void process_BALL_SERVICE();
    void process_BALL_EXCHANGE();
 
    void enter_PLAYER_SCORES(int player,int amount);
    void process_PLAYER_SCORES();
    void enter_GAME_OVER() ;
    void process_GAME_OVER();
    void manageBaseTriggering();


    /* General game state */
    GAME_STATES game_state=OFF;
    unsigned long game_tick_millis=0;
    unsigned long game_tick_number=0;
    int game_gridsize=0;   //* Meaning 0= baseA and gridsize-1=Base B)
    int standard_velocity=0;
    int level_velocity=0;
    int hitcount=0;    

    /* Ball state */
    int ball_position=0;
    int ball_velocity=0;
    int ball_direction=0;


    /* Base state */
    unsigned long base_A_trigger_millis=0;
    base_state_enum base_A_state=BASE_OPEN;
    unsigned long base_B_trigger_millis=0;
    base_state_enum base_B_state=BASE_OPEN;

    /* Player score */
    byte player_score[2]={0,0};
    byte current_scoring_player=NONE;

}; // class PongGame

  


#endif
