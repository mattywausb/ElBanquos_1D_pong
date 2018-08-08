#include "Arduino.h"
#include "PongGame.h"
#include "MainSettings.h"


#define TARGET_SCORE 5

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
    
    player_score[PLAYER_A]=0;
    player_score[PLAYER_B]=0;
    game_state=START;
    current_scoring_player=NONE;
    tick_number=0;
    
}

void PongGame::startGame(void)
{
    enter_BALL_SERVICE();
}


/* --------  information about game state ------------ */
    unsigned int PongGame::getBallPosition(){return ball_position;}
    bool PongGame::base_A_isTriggered() {return base_A_hot;}
    bool PongGame::base_B_isTriggered() {return base_B_hot;}
    int  PongGame::player_A_getScore() {return  player_score[PLAYER_A];}
    int  PongGame::player_B_getScore() {return  player_score[PLAYER_B];}
    byte PongGame::player_getWinner()
    {
      if(player_score[PLAYER_A]>player_score[PLAYER_B]) return PLAYER_A;
      else if(player_score[PLAYER_B]>player_score[PLAYER_A]) return PLAYER_B;
      return NONE;
    }
    

 /* ---------------   Main game logic --------------- */
unsigned long PongGame::process_tick()
{
  unsigned long current_tick_duration=millis()-game_tick_millis;

  if(current_tick_duration<GAME_TICK_DELAY) return GAME_TICK_DELAY-current_tick_duration;
  tick_number++;
  game_tick_millis=millis();  // globally fixed timestamp for this tick 

  input_capture_tick();  // Update all input sensor information

  switch(game_state) 
  {
    case BALL_SERVICE:  process_BALL_SERVICE(); break;
    case BALL_EXCHANGE: process_BALL_EXCHANGE();break;
    case PLAYER_SCORES: process_PLAYER_SCORES();break;
    case GAME_OVER:     process_GAME_OVER();
  }
  return 0; 
}

  
void PongGame::process_BALL_SERVICE(void)
{
  if(current_scoring_player==PLAYER_A) 
  {
     ball_position=PIXEL_COUNT-1;
     ball_velocity=-1;
  }
  else { 
    ball_position=0;
    ball_velocity=1;
  } 
  game_state=BALL_EXCHANGE; 
}



void PongGame::process_BALL_EXCHANGE(void)
{
  if(input_button_A_gotPressed() && game_tick_millis-base_A_trigger_millis>BASE_HOT_RECOVERY) {
    base_A_trigger_millis=game_tick_millis;
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
      if(base_A_hot)   
      {
        ball_position-=2*ball_velocity;  // Reflect the ball
        ball_velocity = -ball_velocity;
      }  else {
        enter_PLAYER_SCORES(PLAYER_B,1);
        return;
      }
    }

    if(ball_position>=PIXEL_COUNT) {         // Ball reached Base B
      if(base_B_hot)   
      {
        ball_position-=2*ball_velocity;  // Reflect the ball
        ball_velocity = -ball_velocity;
      }  else {
        enter_PLAYER_SCORES(PLAYER_A,1);
        return;
      }
    }
  }
}

void PongGame::enter_PLAYER_SCORES(int player,int amount)
{
  // TODO - Play some cool Animation
    player_score[player]+=amount;
    if(player_score[player]>=TARGET_SCORE)
    {
      enter_GAME_OVER();
      return;
    }
    game_state=PLAYER_SCORES;
}
     
void PongGame::process_PLAYER_SCORES(void)
{
  delay(1000);
  enter_BALL_SERVICE();
}

void PongGame::enter_GAME_OVER()
{
  //TODO : Play some cool victory Animation
  game_state=GAME_OVER;
}

void PongGame::process_GAME_OVER()
{
  delay(2000);  // TODO: Wait for some interaction, play game_over_scene
  enter_START();
}




