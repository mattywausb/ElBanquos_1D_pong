#include <Adafruit_NeoPixel.h>

#include "MainSettings.h"

#include "PongGame.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT
#endif

/*device */

#define NEO_PIXEL_DATA_PIN    7    // Digital IO pin connected to the NeoPixels.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEO_PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

/* Design */

// #define COLOR(r,g,b,i) strip.Color(r*i/255, g*i/255, b*i/255)
#define FPS 50
#define FRAME_DELAY 1000/FPS
#define BALL_COLOR strip.Color(200, 200, 0) 
#define BALL_COLOR_AFTER_GLOW(i) dimmedColor(20, 10, 0,i)  
#define BACKGROUND_COLOR strip.Color(0, 0, 0)
#define BASE_A_COLOR(i) dimmedColor(0, 40, 40,i)
#define BASE_B_COLOR(i) dimmedColor(50, 00,0,i)
#define BASE_HOT_COLOR strip.Color(180,90,0)
#define POINT_COLOR strip.Color(15,0,30)

#define BASE_A_POSITION 0
#define BASE_B_POSITION (PIXEL_COUNT-1)

/* Output scene and sequence management */

// TODO add another setting for sequences, so sequences can overrule program and fall back to it afterwards
enum OUTPUT_PROGRAM
{
  GAME_SCENE,
  GAME_OVER_SCENE
} ;

OUTPUT_PROGRAM output_program=GAME_SCENE;

unsigned long output_frame_tick_millis=0;
bool output_sequence_complete=true;
unsigned long output_scene_start_millis=0;
unsigned long output_frame_number=0;

PongGame *displayGame;  // We store this globally as long as there is only one game to present at the same time

/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ------------- General Information --------------- */

bool output_isSequenceComplete() {return output_sequence_complete;}

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

  switch(output_program)
  {
    case GAME_SCENE: output_process_GAME_SCENE(); break;
    case GAME_OVER_SCENE: output_process_GAME_OVER_SCENE(); break;
  }
  return 0;
}

/*  ************************  scenes  ***********************************
 *  *********************************************************************
 */

/* ************ GAME_SCENE *************************** */

void output_begin_GAME_SCENE ()
{
  for(int i=0;i<PIXEL_COUNT;i++) strip.setPixelColor(i, 0);
  strip.show();
  output_frame_number=0;
  output_scene_start_millis=millis();
  output_program=GAME_SCENE;
}

void output_process_GAME_SCENE()
{ 
  static byte afterglow_position=255;
  static byte previous_ball_position=255;

  /* calculate visual effects */
  if(previous_ball_position!=displayGame->getBallPosition()) {
       afterglow_position=previous_ball_position;
       previous_ball_position=displayGame->getBallPosition();      
  }
  
  for(int i=0;i<PIXEL_COUNT;i++) {

      /* Ball Layer */
      if(i==displayGame->getBallPosition()) {
        strip.setPixelColor(i, BALL_COLOR);
        #ifdef TRACE_ON
          Serial.println(i);
        #endif
        continue;
      }

      /* Base layer */
      if(i==BASE_A_POSITION) 
        {
          if(displayGame->base_A_isTriggered()) strip.setPixelColor(i,BASE_HOT_COLOR);
          else           strip.setPixelColor(i,BASE_A_COLOR(255));
          continue;
      }

      if(i==BASE_B_POSITION) 
      {
          if(displayGame->base_B_isTriggered()) strip.setPixelColor(i,BASE_HOT_COLOR);
          else           strip.setPixelColor(i,BASE_B_COLOR(255));
          continue;
       }
        
       /* Background and visual effects */
      
       if(i==afterglow_position)
       {
        strip.setPixelColor(i,BALL_COLOR_AFTER_GLOW(255));
        continue;
       }
       
       if(i<displayGame->player_A_getScore()+1) {
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }  
       if(i>=(PIXEL_COUNT-2-displayGame->player_B_getScore())){
        strip.setPixelColor(i, POINT_COLOR);
        continue;
       }
            
       strip.setPixelColor(i,BACKGROUND_COLOR);           
    }
     
    strip.show();
}

/* ************ GAME_OVER_SCENE *************************** */


void output_begin_GAME_OVER_SCENE()
{
  for(int i=0;i<PIXEL_COUNT;i++) strip.setPixelColor(i, 0);
  strip.show();
  output_frame_number=0;
  output_scene_start_millis=millis();
  output_program=GAME_OVER_SCENE;
  
}

void output_process_GAME_OVER_SCENE()
{
  uint32_t color;

  if(displayGame->player_getWinner()==PLAYER_A) {
      color=BASE_A_COLOR((FPS-(output_frame_number%FPS)*255/FPS));
  }
  if(displayGame->player_getWinner()==PLAYER_B) {
      color=BASE_B_COLOR((FPS-(output_frame_number%FPS)*255/FPS));
  }
  if(displayGame->player_getWinner()==NONE) {
      color=POINT_COLOR;
  }
  for(int i=0;i<PIXEL_COUNT;i++) {
    strip.setPixelColor(i,BALL_COLOR_AFTER_GLOW(255));
  }
}


/*  ************************  helper  ************************************
 *  *********************************************************************
 */

uint32_t dimmedColor(int r,int g,int b,int i)
{
  return strip.Color(r*i/255, g*i/255, b*i/255);
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


