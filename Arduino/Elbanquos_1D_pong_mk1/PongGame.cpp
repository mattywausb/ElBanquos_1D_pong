#include "PongGame.h"


/* ------------- Creation  -------------- */
PongGame::PongGame(void)
{
  setupGame();
}

/* --------- Managing operations ------------*/
void PongGame::setupGame(void)
{
    game_tick_millis=0;

    base_A_trigger_millis=0;
    base_A_hot=false;
    base_B_trigger_millis=0;
    base_B_hot=false;
    
    base_b_score=0;
    base_a_score=0;
    game_state=PREPARED;
    
}

void PongGame::startGame(void)
{
     tick_number=0;
     game_state=PLAYING;
}

 /* ---------------   Main game logic --------------- */
unsigned long PongGame::process_tick()
{
  unsigned long current_tick_duration=millis()-game_tick_millis;
  base_a_score_event=false; 
  base_b_score_event=false; 

  if(current_tick_duration<GAME_TICK_DELAY) return GAME_TICK_DELAY-current_tick_duration;
  tick_number++;
  game_tick_millis=millis();

 input_foreward_tick();  // Update all input sensor information

  switch(game_state) 
  {
    case BALL_SERVICE:  process_BALL_SERVICE(); break;
    case PLAY:          process_PLAY();break;
    case PLAYER_A_GETS_POINT(): process_PLAYER_A_GETS_POINT();break;
    case PLAYER_B_GETS_POINT(): process_PLAYER_B_GETS_POINT();break;
    case GAME_OVER:   process_GAME_OVER();
  }
  return 0; 
}


  
      void process_BALL_SERVICE();
    void process_PLAY();
 
    void process_PLAYER_A_GETS_POINT();
    void process_PLAYER_B_GETS_POINT();
    void process_GAME_OVER();

  
  if(input_button_A_gotPressed() && game_tick_millis-base_A_trigger_millis>BASE_HOT_RECOVERY) {
    base_A_trigger_millis=game_tick_millis
    base_A_hot=true;
  }
  if(base_A_hot &&  game_tick_millis-base_A_trigger_millis>BASE_HOT_DURATION) base_A_hot=false;

  if(input_button_B_gotPressed()&& game_tick_millis-base_B_trigger_millis>BASE_HOT_RECOVERY) {
    base_B_trigger_millis=game_tick_millis;
    base_B_hot=true;
  }
  if(base_B_hot &&  game_tick_millis-base_B_trigger_millis>BASE_HOT_DURATION) base_B_hot=false;
  

  if((tick_number%10)==0) // primitve method to slow down the ball TODO: Variable speed
  {
    ball_position+=ball_velocity;  // Ball movement
    
    if(ball_position<0) {         // Ball reached Base A
      if(base_A_hot)     {
        ball_position-=2*ball_velocity;
        ball_direction = -ball_velocity;
      } else {
       player_b_score+=1;
       player_b_score_event=true;
       ball_position=PIXEL_COUNT-1;
      }
    }
    
    if(ball_position>=PIXEL_COUNT) 
    {
     if(base_B_hot)     {
        ball_position-=2*ball_velocity;
        ball_direction = -ball_velocity;
      } else {
       player_a_score+=1;
       player_a_score_event=true;
       ball_position=0;
      }    
     }
    }
  return 0;  // initiate immediate call to calculate exact delay without
}


    unsigned int PongGame::getBallPosition();
    bool PongGame::base_A_isTriggered();
    bool PongGame::base_B_isTriggered();
    int  PongGame::player_A_getScore();
    int  PongGame::player_B_getScore();
    bool PongGame::player_A_hasScoreEvent();
    bool PongGame::player_B_hasScoreEvent();

