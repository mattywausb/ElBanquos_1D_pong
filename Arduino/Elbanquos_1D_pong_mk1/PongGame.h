#ifndef PongGame_h
#define PongGame_h


#include "input.h"

/* Game setting and memory */
#define GAME_TICK_DELAY 10
#define BASE_HOT_DURATION 500
#define BASE_HOT_RECOVERY 1000

#define PLAYER_A 0
#define PLAYER_B 1
#define NONE 255


enum GAME_STATES {
  START, 
  BALL_SERVICE,
  BALL_EXCHANGE,
  PLAYER_SCORES,
  GAME_OVER
};

class PongGame
{
  public:
    /* Instantiate the game */
    PongGame(void);

    /* ----------- Operations ------------------ */

    /* Main processing of the game logic 
     *  Returns: milliseconds until the next tick is pending
    */
    void setupGame();
    void startGame();
    
    unsigned long process_tick();

    /* ---- State information ----*/
    unsigned int getBallPosition();
    bool base_A_isTriggered();
    bool base_B_isTriggered();
    int  player_A_getScore();
    int  player_B_getScore();
    byte player_getWinner();


  protected:

    void enter_START(){ game_state=START;};
    void process_START(){ game_state=BALL_SERVICE;};
    void enter_BALL_SERVICE() {game_state=BALL_SERVICE;};
    void process_BALL_SERVICE();
    void process_BALL_EXCHANGE();
 
    void enter_PLAYER_SCORES(int player,int amount);
    void process_PLAYER_SCORES();
    void enter_GAME_OVER() ;
    void process_GAME_OVER();


    /* General game state */
    GAME_STATES game_state=START;
    unsigned long game_tick_millis=0;
    byte tick_number=0;
    

    /* Ball state */
    int ball_position=0;
    int ball_velocity=1;

    /* Base state */
    unsigned base_A_trigger_millis=0;
    bool base_A_hot=false;
    unsigned base_B_trigger_millis=0;
    bool base_B_hot=false;

    /* Player score */
    byte player_score[2]={0,0};
    byte current_scoring_player=NONE;

}; // class PongGame

  


#endif
