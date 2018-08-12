


#include <Adafruit_NeoPixel.h>

#include "MainSettings.h"

#include "PongGame.h"
#include "particle.h"
#include "output.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT_PROGRAM_CHANGE
//#define TRACE_OUTPUT
#endif



/*device */

#define NEO_PIXEL_DATA_PIN    7    // Digital IO pin connected to the NeoPixels.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, NEO_PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

#define FPS 30
#define FRAME_DELAY 1000/FPS

/* game */

PongGame *displayGame;  // We store this globally as long as there is only one game to present at the same time

#define _get_ball_pixelPosition  (displayGame->getBallPosition()>>7)
#define _position_to_pixel(i) (i>>7)
#define BASE_A_POSITION 0
#define BASE_B_POSITION (PIXEL_COUNT-1)

/* Design */

struct colorRGB_t {
  byte red;
  byte green;
  byte blue;
} ;

// general elements
#define SELECT_SCENE_MAIN_COLOR_DM(i) dimmedColor(0,120,0,i)

// in game elements
colorRGB_t base_a_color={0, 128, 128};
colorRGB_t base_b_color={150, 0,0};
colorRGB_t bonus_barrier_color={230,230,240};

#define BALL_COLOR strip.Color(200, 200, 0) 
#define BALL_COLOR_DM(i)  dimmedColor(200, 200, 0,i) 
#define BALL_COLOR_AFTER_GLOW(i) dimmedColor(20, 10, 0,i)  
#define BACKGROUND_COLOR strip.Color(0, 0, 0)
#define BASE_HOT_COLOR strip.Color(230,140,0)
#define POINT_COLOR strip.Color(15,0,30)
#define BASE_STANDARD_INTENSITY 40


// general purpose objects for visual design
#define PARTICLE_COUNT 10
Particle effectParticle[PARTICLE_COUNT];

colorRGB_t color_register_a;

/* Output scene and sequence management */

// TODO add another setting for sequences, so sequences can overrule program and fall back to it afterwards
enum output_scene_t
{
  GAME_SCENE,
  GAME_OVER_SCENE,
  GAME_SELECT_SCENE,
  BALL_SERVICE_SCENE
};

output_scene_t output_current_scene=GAME_SCENE;

enum output_sequence_t
{
  NO_SEQUENCE,
  PLAYER_SCORE_SEQUENCE,
  GAME_OVER_SEQUENCE
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
     case GAME_OVER_SEQUENCE: output_process_GAME_OVER_SEQUENCE();break;
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

void draw_bases(byte intensity=BASE_STANDARD_INTENSITY)
{
  if(displayGame->getBase_A_State()==BASE_CLOSED || displayGame->getBase_A_State()==BASE_BOOST) strip.setPixelColor(BASE_A_POSITION,BASE_HOT_COLOR);
  else    strip.setPixelColor(BASE_A_POSITION,dimmedColorRGB(base_a_color,intensity));

  if(displayGame->getBase_B_State()==BASE_CLOSED || displayGame->getBase_B_State()==BASE_BOOST) strip.setPixelColor(BASE_B_POSITION,BASE_HOT_COLOR);
  else    strip.setPixelColor(BASE_B_POSITION,dimmedColorRGB(base_b_color,intensity));
}

void draw_bonus()
{
  int i;
  switch(displayGame->getBonusState()) {
  {
      case BONUS_BARRIER: 
            if(displayGame->getBonusIsPlaced())
            {
              strip.setPixelColor(_position_to_pixel(displayGame->getBonusPosition()),dimmedColorRGB(bonus_barrier_color,200));
              strip.setPixelColor(_position_to_pixel(displayGame->getBonusPosition())-1,dimmedColorRGB(bonus_barrier_color,200));
            } else {
              for(i=0;i<displayGame->getBaseBarrier(PLAYER_A);i++) {
               strip.setPixelColor(i,dimmedColorRGB(bonus_barrier_color,100));       
              }
              for(i=0;i<displayGame->getBaseBarrier(PLAYER_B);i++) {
               strip.setPixelColor(PIXEL_COUNT-1-i,dimmedColorRGB(bonus_barrier_color,100));       
              }
            }
            break;
    }
  }
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

  /* draw  */
  strip.clear();
  draw_score();
  
  /* Visual Effects */
  if(afterglow_position<PIXEL_COUNT)
  {
      strip.setPixelColor(afterglow_position,BALL_COLOR_AFTER_GLOW(255));
  }
  
  draw_bases();
  draw_bonus();

  /* Ball */ 
  strip.setPixelColor(_get_ball_pixelPosition, BALL_COLOR);     
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
  strip.clear();
  draw_score();
  draw_bases();  // TODO; Pulse score of winner
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
  
  if(output_frame_number<=4) draw_bases(255);
  else if(output_frame_number<=20) draw_bases(255-output_frame_number*10);
      else draw_bases(180);
  
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

/* ************ GAME_OVER_SCENE *************************** */


void output_begin_GAME_OVER_SEQUENCE()
{
  beginSequence();
  output_current_sequence=GAME_OVER_SEQUENCE;
  #ifdef TRACE_OUTPUT_PROGRAM_CHANGE
     Serial.println(F(">GAME_OVER_SEQUENCE"));
  #endif
  
  if(displayGame->player_getWinner()==PLAYER_A)
          color_register_a=base_a_color;
  if(displayGame->player_getWinner()==PLAYER_B)
          color_register_a=base_b_color;

}

void output_process_GAME_OVER_SEQUENCE()
{

  if(output_frame_number>4*FPS) fallbackToScene ();

  int pixel=output_frame_number/5;
  if(pixel<4) {   // Initial animation (5x4 = 20 Frames), Directly addressed for 12 pixel ring
       strip.setPixelColor(pixel+3,dimmedColorRGB(color_register_a,255));
        strip.setPixelColor(3-pixel,dimmedColorRGB(color_register_a,255));
        strip.setPixelColor(8-pixel,dimmedColorRGB(color_register_a,255));
        strip.setPixelColor(8+pixel,dimmedColorRGB(color_register_a,255));
 } else {    // Pulse 
      for(int i=0;i<PIXEL_COUNT;i++) {
    strip.setPixelColor(i,dimmedColorRGB(color_register_a,  ((FPS-(output_frame_number%FPS))*255/FPS) ) );
     }
  }

  strip.show();
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
  if(!output_sequence_running) 
  {
    strip.clear();
    output_frame_number=0;
  }
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




uint32_t dimmedColorRGB(struct colorRGB_t myColor,int i)
{
  if(i<0)i=0;
  if(i>255)i=255;
  return strip.Color(myColor.red*i/255, myColor.green*i/255, myColor.blue*i/255);
}

