#ifndef PongGame_h
#define PongGame_h

/* Game setting and memory */
#define GAME_TICK_DELAY 10
#define BASE_HOT_DURATION 500
#define BASE_HOT_RECOVERY 1000

enum GAME_STATES {
  START, 
  BALL_SERVICE,
  RUNNING,
  PLAYER_A_GETS_POINT,
  PLAYER_B_GETS_POINT,
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


  protected:

    void process_START(){ game_state=BALL_SERVICE;};
    void process_BALL_SERVICE();
    void process_PLAY();
 
    void process_PLAYER_A_GETS_POINT();
    void process_PLAYER_B_GETS_POINT();
    void process_GAME_OVER();


    /* General game state */
    GAME_STATES game_state=PREPARED;
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
    byte player_a_score=0;
    byte player_b_score=0;
    byte previous_scoring_player=0;

} // class PongGame

  


#endif
