#include "Arduino.h"
#include "PongGame.h"
#include "MainSettings.h"

#include "input.h"
#include "output.h"

#define TARGET_SCORE 5

#ifdef TRACE_ON
#define TRACE_PONG_STATE
//#define TRACE_TICK
#endif

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
    game_state=OFF;
    current_scoring_player=NONE;
    game_tick_number=0;
  #ifdef TRACE_PONG_STATE
    Serial.println(F("setupGame"));
  #endif    
}

void PongGame::startGame(void)
{
    enter_START();
}


/* --------  information about game state ------------ */
bool PongGame::isActive() {return !(game_state==OFF || game_state==CLOSING);} 

bool PongGame::isClosing() 
{
  if(game_state!=CLOSING) return false;
  game_state=OFF;
  return true;
} 

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
  game_tick_number++;
  game_tick_millis=millis();  // globally fixed timestamp for this tick 

  input_capture_tick();  // Update all input sensor information
  #ifdef TRACE_TICK
    Serial.print(F("Game Tick:"));Serial.println(game_tick_number); 
  #endif
  
  switch(game_state) 
  {
    case START:  process_START(); break;
    case BALL_SERVICE:  process_BALL_SERVICE(); break;
    case BALL_EXCHANGE: process_BALL_EXCHANGE();break;
    case PLAYER_SCORES: process_PLAYER_SCORES();break;
    case GAME_OVER:     process_GAME_OVER();break;
    default: game_state=OFF;
  }
  return 0; 
}

void PongGame::enter_START()
{ 
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">START"));
  #endif
  game_state=START;
  if(random(19)>9)current_scoring_player=PLAYER_A;
  else current_scoring_player=PLAYER_B;
};

void PongGame::process_START()
{
  enter_BALL_SERVICE();
};


void PongGame::enter_BALL_SERVICE() 
{
  game_state=BALL_SERVICE;
  output_begin_GAME_SCENE ();
  base_A_trigger_millis=0; 
  base_B_trigger_millis=0;
  if(current_scoring_player==PLAYER_A) 
  {
    ball_position=PIXEL_COUNT-1;
    ball_velocity=-1;
  }
  else { 
    ball_position=0;
    ball_velocity=1;
  } 
  output_begin_BALL_SERVICE_SCENE();
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">BALL_SERVICE"));
  #endif
};
  
void PongGame::process_BALL_SERVICE(void)
{
  if(((current_scoring_player==PLAYER_B)&& input_button_A_gotPressed())
   || ((current_scoring_player==PLAYER_A)&& input_button_B_gotPressed())
   || output_sceneDurationMillis()>5000)
  {
      output_begin_GAME_SCENE ();
      game_state=BALL_EXCHANGE; 
    }  

  manageBaseTriggering();
}



void PongGame::process_BALL_EXCHANGE(void)
{
 
  manageBaseTriggering();
  
  if((game_tick_number%10)==0) // primitve method to slow down the ball TODO: Variable speed
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
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">PLAYER_SCORES"));
  #endif
  
  
    player_score[player]+=amount;
    
    current_scoring_player=player;
    output_begin_PLAYER_SCORE_SEQUENCE(player);
    if(player_score[player]>=TARGET_SCORE)
    {
      enter_GAME_OVER();
      return;
    }
    game_state=PLAYER_SCORES;
}
     
void PongGame::process_PLAYER_SCORES(void)
{
  while(output_isSequenceRunning()) return;
  enter_BALL_SERVICE();
}

void PongGame::enter_GAME_OVER()
{
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">GAME_OVER"));
  #endif
  //TODO : Play some cool victory Animation
  game_state=GAME_OVER;
  output_begin_GAME_OVER_SCENE();
}

void PongGame::process_GAME_OVER()
{
  if(output_sceneDurationMillis()>5000) {
    game_state=CLOSING;
  }
}

void PongGame::manageBaseTriggering()
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
}


