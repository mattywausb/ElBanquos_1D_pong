#include <Adafruit_NeoPixel.h>

#include "MainSettings.h"

#include "PongGame.h"
#include "particle.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT_PROGRAM_CHANGE
//#define TRACE_OUTPUT
#endif

/*device */

#define NEO_PIXEL_DATA_PIN    7    // Digital IO pin connected to the NeoPixels.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEO_PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

/* game */

PongGame *displayGame;  // We store this globally as long as there is only one game to present at the same time

#define _get_ball_pixelPosition  (displayGame->getBallPosition()>>7)
#define BASE_A_POSITION 0
#define BASE_B_POSITION (PIXEL_COUNT-1)

/* Design */

// #define COLOR(r,g,b,i) strip.Color(r*i/255, g*i/255, b*i/255)
#define FPS 30
#define FRAME_DELAY 1000/FPS

// general elements
#define SELECT_SCENE_MAIN_COLOR_DM(i) dimmedColor(0,120,0,i)

// in game elements
#define BALL_COLOR strip.Color(200, 200, 0) 
#define BALL_COLOR_DM(i)  dimmedColor(200, 200, 0,i) 
#define BALL_COLOR_AFTER_GLOW(i) dimmedColor(20, 10, 0,i)  
#define BACKGROUND_COLOR strip.Color(0, 0, 0)
#define BASE_A_COLOR(i) dimmedColor(0, 128, 128,i)
#define BASE_B_COLOR(i) dimmedColor(150, 00,0,i)
#define BASE_HOT_COLOR strip.Color(230,140,0)
#define POINT_COLOR strip.Color(15,0,30)





// general purpose objects for visual design
#define PARTICLE_COUNT 10
Particle effectParticle[PARTICLE_COUNT];

/* Output scene and sequence management */

// TODO add another setting for sequences, so sequences can overrule program and fall back to it afterwards
enum output_scene_t
{
  GAME_SCENE,
  GAME_OVER_SCENE,
  GAME_SELECT_SCENE,
  BALL_SERVICE_SCENE
} ;

output_scene_t output_current_scene=GAME_SCENE;

enum output_sequence_t
{
  NO_SEQUENCE,
  PLAYER_SCORE_SEQUENCE
} ;

output_sequence_t output_current_sequence=NO_SEQUENCE;

unsigned long output_frame_tick_millis=0;
bool output_sequence_running=false;
unsigned long output_scene_start_millis=0;
unsigned long output_sequene_start_millis=0;
unsigned long output_frame_number=0;



/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ------------- General Information --------------- */

bool output_isSequenceRunning() {return output_sequence_running;}
unsigned long output_sequenceDurationMillis() {return millis()-output_sequene_start_millis;}
unsigned long output_sceneDurationMillis() {return millis()-output_scene_start_millis;}

/*  ************************  tick  ************************************
 *  *********************************************************************
 */
unsigned long output_frame_tick()
{
  unsigned long current_frame_duration=millis()-output_frame_tick_millis;

  if(current_frame_duration<FRAME_DELAY) return FRAME_DELAY-current_frame_duration;
  output_frame_number++;   // Will be reset by every scene or sequence change
  output_frame_tick_millis=millis();  // globally fixed timestamp for this tick

  if(output_sequence_running) {
    switch(output_current_sequence)
    {
     case PLAYER_SCORE_SEQUENCE: output_process_PLAYER_SCORE_SEQUENCE(); break;
    }   
    return 0;  // Sequences overrule scenes
  }

  switch(output_current_scene)
  {
    case GAME_SCENE: output_process_GAME_SCENE(); break;
    case GAME_OVER_SCENE: output_process_GAME_OVER_SCENE(); break;
    case GAME_SELECT_SCENE: output_process_GAME_SELECT_SCENE(); break;
    case BALL_SERVICE_SCENE: output_process_BALL_SERVICE_SCENE();break;
  }
  return 0;
}

/*  ************************  parts  ***********************************
 *  *********************************************************************
 */

void draw_score()
{
  for(int i=1;i<PIXEL_COUNT-1;i++) {

       if(i<displayGame->player_A_getScore()+1) {
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }  
       if(i>=(PIXEL_COUNT-1-displayGame->player_B_getScore())){
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }
  }
};

void draw_bases()
{
  if(displayGame->getBase_A_State()==BASE_CLOSED || displayGame->getBase_A_State()==BASE_BOOST) strip.setPixelColor(BASE_A_POSITION,BASE_HOT_COLOR);
  else    strip.setPixelColor(BASE_A_POSITION,BASE_A_COLOR(255));

  if(displayGame->getBase_B_State()==BASE_CLOSED || displayGame->getBase_B_State()==BASE_BOOST) strip.setPixelColor(BASE_B_POSITION,BASE_HOT_COLOR);
  else    strip.setPixelColor(BASE_B_POSITION,BASE_B_COLOR(255));
}

#define __pulse_dimming(frequency, phaseshift, baseline) ((FPS-((output_frame_number+phaseshift*FPS*10/frequency)%(FPS*10/frequency)))*(255-baseline)/FPS+baseline)
/*  ************************  scenes  ***********************************
 *  *********************************************************************
 */
 
/* ************ GAME_SELECT_SCENE *************************** */

void output_begin_GAME_SELECT_SCENE() 
{
  simpleSceneChange();
  output_current_scene=GAME_SELECT_SCENE;
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">GAME_SELECT_SCENE"));
  #endif
}

void output_process_GAME_SELECT_SCENE()
{
  for(int i=0;i<PIXEL_COUNT;i++) strip.setPixelColor(i,  SELECT_SCENE_MAIN_COLOR_DM(64));
  strip.show();
}


/* ************ BALL_SERVICE_SCENE *************************** */

void output_begin_BALL_SERVICE_SCENE() 
{
  simpleSceneChange();
  output_current_scene=BALL_SERVICE_SCENE;
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">BALL_SERVICE_SCENE"));
  #endif
}

void output_process_BALL_SERVICE_SCENE()
{
  strip.clear();
  draw_bases();
  draw_score();
  
  strip.setPixelColor(_get_ball_pixelPosition,  BALL_COLOR_DM(__pulse_dimming(12, 10,0)));
  strip.show();
}
 

/* ************ GAME_SCENE *************************** */

void output_begin_GAME_SCENE ()
{
  simpleSceneChange();
  output_current_scene=GAME_SCENE;
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">GAME_SCENE"));
  #endif
}

void output_process_GAME_SCENE()
{ 
  static byte afterglow_position=255;
  static byte previous_ball_position=255;

  /* calculate visual effects */
  if(previous_ball_position!=_get_ball_pixelPosition) {
       afterglow_position=previous_ball_position;
       previous_ball_position=_get_ball_pixelPosition;      
  }
  
  for(int i=0;i<PIXEL_COUNT;i++) {

      /* Ball Layer */
      if(i==_get_ball_pixelPosition) {
        strip.setPixelColor(i, BALL_COLOR);
        continue;
      }

      /* Base layer */
      if(i==BASE_A_POSITION) 
        {
          if(displayGame->getBase_A_State()==BASE_CLOSED || displayGame->getBase_A_State()==BASE_BOOST) strip.setPixelColor(i,BASE_HOT_COLOR);
          else           strip.setPixelColor(i,BASE_A_COLOR(255));
          continue;
      }

      if(i==BASE_B_POSITION) 
      {
          if(displayGame->getBase_B_State()==BASE_CLOSED || displayGame->getBase_B_State()==BASE_BOOST) strip.setPixelColor(i,BASE_HOT_COLOR);
          else           strip.setPixelColor(i,BASE_B_COLOR(255));
          continue;
       }

       if(i<displayGame->player_A_getScore()+1) {
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }  
       if(i>=(PIXEL_COUNT-1-displayGame->player_B_getScore())){
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }

        
       /* Background and visual effects */
      
       if(i==afterglow_position)
       {
        strip.setPixelColor(i,BALL_COLOR_AFTER_GLOW(255));
        continue;
       }
       

            
       strip.setPixelColor(i,BACKGROUND_COLOR);           
    }
     
    strip.show();
}

/* ************ GAME_OVER_SCENE *************************** */


void output_begin_GAME_OVER_SCENE()
{
  simpleSceneChange();
  output_current_scene=GAME_OVER_SCENE;
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">GAME_OVER_SCENE"));
  #endif
}

void output_process_GAME_OVER_SCENE()
{
  uint32_t color;

  if(displayGame->player_getWinner()==PLAYER_A) {
      color=BASE_A_COLOR(((FPS-(output_frame_number%FPS))*255/FPS));
  }
  if(displayGame->player_getWinner()==PLAYER_B) {
      color=BASE_B_COLOR(((FPS-(output_frame_number%FPS))*255/FPS));
  }
  if(displayGame->player_getWinner()==NONE) {
      color=BALL_COLOR_AFTER_GLOW(255);
  }
  for(int i=0;i<PIXEL_COUNT;i++) {
    strip.setPixelColor(i,color);
  }
  strip.show();
}

/*  ************************  sequences  ***********************************
 *  *********************************************************************
 */

/* ************ PLAYER_SCORE_SEQUENCE *************************** */
 
void output_begin_PLAYER_SCORE_SEQUENCE(byte scoringPlayer)
{
  beginSequence();
  strip.clear();
  draw_bases();
  output_current_sequence=PLAYER_SCORE_SEQUENCE;
  for(int i=0;i<PARTICLE_COUNT;i+=3) {
    if(scoringPlayer==PLAYER_B) {
      effectParticle[i].ignite(0,255,255,0+(i)/3,230+50*i,85);
    } else {
      effectParticle[i].ignite(255,0,0,PIXEL_COUNT-1-(i/3),-230-50*i,85);
    }
  }
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">PLAYER_SCORE_SEQUENCE"));
  #endif
}

void output_process_PLAYER_SCORE_SEQUENCE()
{
  bool particleAlive=false;
  strip.clear();
  draw_bases();
  if(output_frame_number>=4) draw_score();
  
  for(int i=0;i<PARTICLE_COUNT;i++) 
  {
    if(effectParticle[i].isAlive()) 
    {
      particleAlive=true;
      effectParticle[i].draw(strip);
      effectParticle[i].frameTick(output_frame_number);     
    }
  }
  strip.show();
 
  if(!particleAlive || output_frame_number>=200) fallbackToScene();

}



/*  ************************  helper  ************************************
 *  *********************************************************************
 */

uint32_t dimmedColor(int r,int g,int b,int i)
{
  if(i<0)i=0;
  if(i>255)i=255;
  return strip.Color(r*i/255, g*i/255, b*i/255);
}

void simpleSceneChange()
{
  for(int i=0;i<PIXEL_COUNT;i++) strip.setPixelColor(i, 0);
  strip.show();
  output_frame_number=0;
  output_scene_start_millis=millis();
}

void beginSequence()
{
  output_sequence_running=true;
  output_sequene_start_millis=millis();
  output_frame_number=0;
}

void fallbackToScene ()
{
    output_sequence_running=false;
    output_frame_number=0;
    output_current_sequence=NO_SEQUENCE;
    #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F("<<fallbackToScene"));
    #endif
}

/*  ************************  setup  ************************************
 *  *********************************************************************
 */

void output_setup(PongGame *globalPongGame) 
{
  strip.begin();
  strip.setBrightness(64);
  strip.show(); // Initialize all pixels to 'off'

  displayGame=globalPongGame;
}


