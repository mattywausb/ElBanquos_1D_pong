#include "Arduino.h"
#include "PongGame.h"
#include "MainSettings.h"

#include "input.h"
#include "output.h"
#include "sound.h"

#define TARGET_SCORE 5

#ifdef TRACE_ON
#define TRACE_PONG_STATE
//#define TRACE_TICK
#endif

/*  ************************  Construction  ************************************
 *  *********************************************************************
 */
PongGame::PongGame(int gridsize)
{
  game_gridsize=gridsize;


  
  setupGame();
}

/*  ************************  operational methods  ************************************
 *  *********************************************************************
 */
void PongGame::setupGame(void)
{
    game_tick_millis=0;

    base_A_trigger_millis=0;
    base_A_state=BASE_OPEN;
    base_B_trigger_millis=0;
    base_B_state=BASE_OPEN;
    
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

  int standard_traversal_ticks= (STANDARD_TRAVERSAL_TIME/(MILLIS_PER_TICK)/2);
  standard_velocity=game_gridsize / standard_traversal_ticks;
  
   #ifdef TRACE_ON
    Serial.print(F("Gridsize:"));Serial.println(game_gridsize);
    Serial.print(F("Standard velocity:"));Serial.println(standard_velocity);
    Serial.print(F("standard_traversal_ticks:"));Serial.println(standard_traversal_ticks);
    Serial.print(F("MILLIS_PER_TICK:"));Serial.println(MILLIS_PER_TICK);
    Serial.print(F("(gridsize / Velocity)* (Time to traverse):"));Serial.println(game_gridsize*2*MILLIS_PER_TICK/standard_velocity);
      
   #endif
    
    enter_START();
}

/*  ************************  Information methods  ************************************
 *  *********************************************************************
 */
bool PongGame::isActive() {return !(game_state==OFF || game_state==CLOSING);} 

bool PongGame::isClosing() 
{
  if(game_state!=CLOSING) return false;
  game_state=OFF;
  return true;
} 

unsigned int PongGame::getBallPosition(){return ball_position;}

base_state_enum PongGame::getBase_A_State() {return base_A_state;}

base_state_enum PongGame::getBase_B_State() {return base_B_state;}

int  PongGame::player_A_getScore() {return  player_score[PLAYER_A];}

int  PongGame::player_B_getScore() {return  player_score[PLAYER_B];}

byte PongGame::player_getWinner()
{
  if(player_score[PLAYER_A]>player_score[PLAYER_B]) return PLAYER_A;
  else if(player_score[PLAYER_B]>player_score[PLAYER_A]) return PLAYER_B;
  return NONE;
}
    
/*  ************************  tick  ************************************
 *  *********************************************************************
 */

unsigned long PongGame::process_tick()
{
  unsigned long current_tick_duration=millis()-game_tick_millis;

  if(current_tick_duration<MILLIS_PER_TICK) return (MILLIS_PER_TICK)-current_tick_duration;
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

/*  ************************  state processing functions  ************************************
 *  *********************************************************************
 */

/* ************ START *************************** */


void PongGame::enter_START()
{ 
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">START"));
  #endif
  game_state=START;
  if(random(19)>9)current_scoring_player=PLAYER_A;
  else current_scoring_player=PLAYER_B;

  level_velocity= standard_velocity;
  // TODO Trigger Start Animation
  sound_start_GameStartMelody();

};

void PongGame::process_START()
{
  if(sound_isPlaying()) return;
  enter_BALL_SERVICE();
};


/* ************ BALL_SERVICE *************************** */

void PongGame::enter_BALL_SERVICE() 
{
  game_state=BALL_SERVICE;
  output_begin_GAME_SCENE ();
  base_A_trigger_millis=0; 
  base_B_trigger_millis=0;
  level_velocity = standard_velocity;   // This might be changed later to some fancy logic
  ball_velocity = level_velocity;
  hitcount=0;
  if(current_scoring_player==PLAYER_A) 
  {
    ball_direction = -1;
    ball_position =game_gridsize-1;  // position ball in base B
  }
  else { 
    ball_direction = 1;
    ball_position = 0;  // position ball in base A
  } 
  output_begin_BALL_SERVICE_SCENE();
  input_pauseUntilRelease();
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">BALL_SERVICE"));
  #endif
};
  
void PongGame::process_BALL_SERVICE(void)
{
  if(((current_scoring_player==PLAYER_B)&& input_button_A_gotReleased())
   || ((current_scoring_player==PLAYER_A)&& input_button_B_gotReleased())
   || ((output_sceneDurationMillis()>5000))  )
  {
      base_A_state=BASE_OPEN;
      base_B_state=BASE_OPEN;
      game_state=BALL_EXCHANGE; 
      output_begin_GAME_SCENE ();
      sound_start_Ping();
    }  

  manageBaseTriggering();
}

/* ************ BALL_EXCHANGE *************************** */

void PongGame::process_BALL_EXCHANGE(void)
{
 
  manageBaseTriggering();

  /* Ball movement */ 
  if((game_tick_number%TICKS_PER_MOVEMENT)==0) 
  {
    ball_position+=ball_velocity*ball_direction;  // Ball movement
    
    if(ball_position<0) {         // Ball reached Base A
      hitcount++;
      if(base_A_state==BASE_CLOSED || base_A_state==BASE_BOOST)   
      {
        ball_direction = 1;
        if(base_A_state==BASE_BOOST) {
          sound_start_Ping();
          ball_velocity=level_velocity<<1;  //Double speed
          level_velocity+=2;                
        } else {
          sound_start_Pong();
          ball_velocity=level_velocity;
        }
        ball_position = ball_velocity;  // Reflect the ball
        //TODO Trigger Sound

        return;

      }  else {
        enter_PLAYER_SCORES(PLAYER_B,1);
        return;
      }
    }

    if(ball_position>=game_gridsize) {         // Ball reached Base B
      hitcount++;
      if(base_B_state==BASE_CLOSED || base_B_state==BASE_BOOST)   
      {
        ball_direction = -1;
        if(base_B_state==BASE_BOOST) {
          sound_start_Ping();
          ball_velocity=level_velocity<<1;  //Double speed
          level_velocity+=2;                
        } else {
          sound_start_Pong();
          ball_velocity=level_velocity;
        }
        ball_position=game_gridsize-1-ball_velocity;  // Reflect the ball
        //TODO Trigger Sound
        return;

      }  else {
        enter_PLAYER_SCORES(PLAYER_A,1);
        return;
      }
    }
  }

  if(game_tick_number%TICKS_UNTIL_AUTOMATIC_ACCELERATION==0) 
  {
    level_velocity+=2;
  }
}

/* ************ PLAYER_SCORES *************************** */

void PongGame::enter_PLAYER_SCORES(int player,int amount)
{
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">PLAYER_SCORES"));
  #endif
  player_score[player]+=amount;
    
  current_scoring_player=player;
  output_begin_PLAYER_SCORE_SEQUENCE(player);
  sound_start_Crash();

  game_state=PLAYER_SCORES;
}
     
void PongGame::process_PLAYER_SCORES(void)
{
 while(output_isSequenceRunning()) return;
 if(player_score[current_scoring_player]>=TARGET_SCORE)
 {    
      enter_GAME_OVER();
      return;
  }
  enter_BALL_SERVICE();
}

/* ************ GAME_OVER *************************** */

void PongGame::enter_GAME_OVER()
{
  #ifdef TRACE_PONG_STATE
    Serial.println(F(">GAME_OVER"));
  #endif
  //TODO : Play some cool victory Animation
  sound_start_GameOverMelody();
  game_state=GAME_OVER;
  output_begin_GAME_OVER_SCENE();
}



void PongGame::process_GAME_OVER()
{
  if(output_sceneDurationMillis()>5000) {
    game_state=CLOSING;
  }
}


/*  ************************ helping modules  ************************************
 *  *********************************************************************
 */

/** ---------  Check and switch hot states of bases ---- */  
void PongGame::manageBaseTriggering()
{
  bool boost_enabled= (hitcount>3)|| (player_score[PLAYER_A]>0&&player_score[PLAYER_B]>0);
  switch(base_A_state) {
    case BASE_OPEN: 
                    if(input_button_A_gotPressed())  // Only in open State we accept a button press as valid
                    {
                      base_A_trigger_millis=game_tick_millis;
                      if(boost_enabled) base_A_state=BASE_BOOST;
                      else  base_A_state=BASE_CLOSED;
                    }
                    break;
    case BASE_BOOST:
                    if(game_tick_millis-base_A_trigger_millis>BASE_BOOST_DURATION) base_A_state=BASE_CLOSED;
                    break;
    case BASE_CLOSED:
                    if(game_tick_millis-base_A_trigger_millis>BASE_CLOSED_DURATION) base_A_state=BASE_RECOVERY;
                    break;
    case BASE_RECOVERY:
                    if(game_tick_millis-base_A_trigger_millis> BASE_RECOVERY_DURATION) base_A_state=BASE_OPEN;
                    break;      
    default:
             base_A_state=BASE_OPEN;    
                
  }

  switch(base_B_state) {
    case BASE_OPEN: 
                    if(input_button_B_gotPressed())  // Only in open State we accept a button press as valid
                    {
                      base_B_trigger_millis=game_tick_millis;
                      if(boost_enabled) base_B_state=BASE_BOOST;
                      else  base_B_state=BASE_CLOSED;
                    }
                    break;
    case BASE_BOOST:
                    if(game_tick_millis-base_B_trigger_millis>BASE_BOOST_DURATION) base_B_state=BASE_CLOSED;
                    break;
    case BASE_CLOSED:
                    if(game_tick_millis-base_B_trigger_millis>BASE_CLOSED_DURATION) base_B_state=BASE_RECOVERY;
                    break;
    case BASE_RECOVERY:
                    if(game_tick_millis-base_B_trigger_millis> BASE_RECOVERY_DURATION) base_B_state=BASE_OPEN;
                    break;      
    default:
             base_B_state=BASE_OPEN;    
                
  }
  

}


